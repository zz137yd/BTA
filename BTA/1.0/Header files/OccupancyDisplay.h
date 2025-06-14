#ifndef OCCUPANCYDISPLAY_H
#define OCCUPANCYDISPLAY_H

#pragma once

#include <QObject>
#include <QRunnable>
#include <QString>
#include <QDir>
#include <QDirIterator>
#include <QThread>
#include <QApplication>
#include <QStyledItemDelegate>
#include <QProgressBar>
#include <QStorageInfo>
#include <QFileSystemModel>
#include <QHash>
#include <QThreadPool>
#include <QSet>
#include <QPainter>

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

// 后台任务:
// 使用 QDirIterator 遍历目录，同时降低线程优先级和适时让出 CPU
class DirectorySizeWorker : public QObject, public QRunnable
{
   Q_OBJECT

public:
   DirectorySizeWorker(const QString& path) : m_path(path)
   {
       setAutoDelete(true);
   }
   void run() override
   {
       // 降低线程优先级以避免高占用率
       QThread::currentThread()->setPriority(QThread::LowPriority);
       qint64 size = computeSize(m_path);
       emit sizeComputed(m_path, size);
   }

signals:
   void sizeComputed(const QString& path, qint64 size);

private:
   qint64 computeSize(const QString& path)
   {
       qint64 totalSize = 0;
       QDirIterator it(path, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
       int counter = 0;
       while (it.hasNext())
       {
           it.next();
           totalSize += it.fileInfo().size();
           // 处理完 1000 个文件后，让出CPU一会儿
           if (++counter % 1000 == 0)
           {
               QThread::yieldCurrentThread();
           }
       }
       return totalSize;
   }
   QString m_path;
};

class SizeProgressDelegate : public QStyledItemDelegate
{
   Q_OBJECT
public:
   explicit SizeProgressDelegate(QFileSystemModel* model, QObject* parent = nullptr)
       : QStyledItemDelegate(parent), model(model) {}

    void loadCacheFromFile(const QString& file)
    {
        QString cacheFile = file;
        if (cacheFile.isEmpty())
        {
            //cacheFile = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "./ProgressBar_Cache.json";
            cacheFile = QCoreApplication::applicationDirPath() + "./ProgressBar_Cache.json";
        }
        QFile f(cacheFile);
        if (!f.open(QIODevice::ReadOnly)) return;
        QByteArray data = f.readAll();
        f.close();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isObject()) return;
        QJsonObject obj = doc.object();
        sizeCache.clear();
        for (auto it = obj.begin(); it != obj.end(); ++it)
        {
            sizeCache.insert(it.key(), static_cast<qint64>(it.value().toDouble()));
        }
    }

    void saveCacheToFile(const QString& file)
    {
        QString cacheFile = file;
        if (cacheFile.isEmpty())
        {
            //cacheFile = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "./ProgressBar_Cache.json";
            cacheFile = QCoreApplication::applicationDirPath() + "./ProgressBar_Cache.json";
        }
        QJsonObject obj;
        for (auto it = sizeCache.begin(); it != sizeCache.end(); ++it)
        {
            obj.insert(it.key(), static_cast<double>(it.value()));
        }
        QJsonDocument doc(obj);
        QDir().mkpath(QFileInfo(cacheFile).absolutePath());
        QFile f(cacheFile);
        if (f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
            f.write(doc.toJson(QJsonDocument::Compact));
            f.close();
        }
    }

   bool isDiskRoot(const QString& path) const
   {
       QList<QStorageInfo> storageInfoList = QStorageInfo::mountedVolumes();
       QDir d(path);
       QString canonicalPath = d.canonicalPath();
       for (const QStorageInfo& storageInfo : storageInfoList)
       {
           QDir rd(storageInfo.rootPath());
           if (canonicalPath == rd.canonicalPath()) return true;
       }
       return false;
   }

   qint64 getItemSizeBytes(const QModelIndex& index) const
   {
       QString path = model->filePath(index);
       if (sizeCache.contains(path)) return sizeCache.value(path);

       QFileInfo info(path);
       qint64 result = 0;
       if (info.isFile())
       {
           result = info.size();
           sizeCache.insert(path, result);
       }
       else if (info.isDir())
       {
           if (isDiskRoot(path))
           {
               QList<QStorageInfo> storageInfoList = QStorageInfo::mountedVolumes();
               for (const QStorageInfo& storageInfo : storageInfoList)
               {
                   if (path.startsWith(storageInfo.rootPath()))
                   {
                       result = storageInfo.bytesTotal();
                       break;
                   }
               }
               sizeCache.insert(path, result);
           }
           else
           {
               // 如果目录没有计算，则异步提交任务
               if (!pendingPaths.contains(path))
               {
                   pendingPaths.insert(path);
                   DirectorySizeWorker* worker = new DirectorySizeWorker(path);
                   connect(worker, &DirectorySizeWorker::sizeComputed,
                           this, [this](const QString& p, qint64 s)
                   {
                       sizeCache.insert(p, s);
                       pendingPaths.remove(p);

                       // 批量刷新接口
                       const_cast<SizeProgressDelegate*>(this)->emitRequestUpdate();
                       const_cast<SizeProgressDelegate*>(this)->saveCacheToFile("");
                   });
                   QThreadPool::globalInstance()->start(worker);
               }
               result = 0;
           }
       }
       return result;
   }

   QString formatSize(qint64 bytes) const
   {
       double size = bytes;
       QString unit;
       if (size < 1024 * 1024)
       {
           size /= 1024.0;
           unit = "KB";
       }
       else if (size < 1024 * 1024 * 1024)
       {
           size /= (1024.0 * 1024.0);
           unit = "MB";
       }
       else
       {
           size /= (1024.0 * 1024.0 * 1024.0);
           unit = "GB";
       }
       return QString::number(size, 'f', 1) + " " + unit;
   }

   void paint(QPainter* painter, const QStyleOptionViewItem& option,
                                    const QModelIndex& index) const
   {
       qint64 currentSizeBytes = getItemSizeBytes(index);
       qint64 maxSizeBytes = 0;
       QModelIndex parentIndex = index.parent();
       int rowCount = model->rowCount(parentIndex);
       for (int i = 0; i < rowCount; ++i)
       {
           QModelIndex siblingIndex = model->index(i, index.column(), parentIndex);
           qint64 siblingSize = getItemSizeBytes(siblingIndex);
           if (siblingSize > maxSizeBytes) maxSizeBytes = siblingSize;
       }

       int progress = 0;
       if (maxSizeBytes > 0)
           progress = static_cast<int>((double(currentSizeBytes) / maxSizeBytes) * 100);
       else
           progress = 0;

       QString text = formatSize(currentSizeBytes);

       QRect rect = option.rect;
       int radius = rect.height() / 2;    // 设定一个参考圆角半径

       painter->save();
       painter->setRenderHint(QPainter::Antialiasing, true);

       // 绘制背景: 带一点灰色的半透明效果，类似毛玻璃效果
       QColor backgroundColor(247, 247, 247, 247);
       painter->setPen(Qt::NoPen);
       painter->setBrush(backgroundColor);
       painter->drawRoundedRect(rect, radius, radius);

       // 绘制进度条: 浅紫色
       int progressWidth = rect.width() * progress / 100;
       QRect progressRect(rect.x(), rect.y(), progressWidth, rect.height());
       QColor progressColor(200, 150, 255);    // 浅紫色

       if (progressWidth > 0)
       {
           // 动态调整圆角半径
           // 确保左右两侧始终保持圆角
           int dynamicRadius = qMin(radius, progressWidth / 2);
           painter->setBrush(progressColor);
           painter->drawRoundedRect(progressRect, dynamicRadius, dynamicRadius);
       }

       // 绘制进度文本
       painter->setPen(Qt::black);
       painter->drawText(rect, Qt::AlignCenter, text);

       painter->restore();
   }

    QHash<QString, qint64>& getSizeCache() { return sizeCache; }
    QSet<QString>& getPendingPaths() { return pendingPaths; }

signals:
   void requestUpdate();

private:
   void emitRequestUpdate() { emit requestUpdate(); }
   QFileSystemModel* model;
   mutable QHash<QString, qint64> sizeCache;
   mutable QSet<QString> pendingPaths;
};

#endif // OCCUPANCYDISPLAY_H
