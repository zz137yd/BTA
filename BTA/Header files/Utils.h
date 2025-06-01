#ifndef UTILS_H
#define UTILS_H

// 1、只有左键双击才能生效
// 支持接收拖拽文件
#include <QTreeView>
#include <QMouseEvent>
#include <QDropEvent>
#include <QDrag>
#include <QDragEnterEvent>
#include <QMimeData>

// 2、居中左右对齐
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>

// 3、复制路径按钮样式
#include <QPushButton>

// 4、收藏下拉框样式
#include <QComboBox>

// 5、图片预览适配
#include <QLabel>
#include <QPixmap>
#include <QResizeEvent>

// 6、鼠标部分事件拦截
#include <QEvent>

// 7、显示内容行间距


// 1.自定义view (包含文件拖拽)
class MyLeftView : public QTreeView
{
    Q_OBJECT
public:
    explicit MyLeftView(QWidget* parent = nullptr) : QTreeView(parent)
    {
        // 支持接收拖拽文件
        setAcceptDrops(true);
        setDragDropMode(QAbstractItemView::DropOnly);
    }

signals:
    void filesDropped(const QStringList& paths, const QModelIndex& targetIndex);

protected:
    void mouseDoubleClickEvent(QMouseEvent* event) override
    {
        if (event->button() == Qt::LeftButton) QTreeView::mouseDoubleClickEvent(event);
        //else event->ignore();
    }

    void dragEnterEvent(QDragEnterEvent* event) override
    {
        if (event->mimeData()->hasUrls())
            event->acceptProposedAction();
        else
            QTreeView::dragEnterEvent(event);
    }

    void dragMoveEvent(QDragMoveEvent* event) override
    {
        if (event->mimeData()->hasUrls())
            event->acceptProposedAction();
        else
            QTreeView::dragMoveEvent(event);
    }

    void dropEvent(QDropEvent* event) override
    {
        if (event->mimeData()->hasUrls())
        {
            QStringList paths;
            for (const QUrl& url : event->mimeData()->urls())
            {
                if (url.isLocalFile()) paths << url.toLocalFile();
            }
            QModelIndex targetIndex = indexAt(event->pos());
            emit filesDropped(paths, targetIndex);
            event->acceptProposedAction();
        }
        else
            QTreeView::dropEvent(event);
    }

    void startDrag(Qt::DropActions supportedActions) override
    {
        QModelIndexList indexes = selectedIndexes();
        if (indexes.isEmpty()) return;

        QMimeData* mimeData = model()->mimeData(indexes);
        if (!mimeData) return;

        QDrag* drag = new QDrag(this);
        drag->setMimeData(mimeData);

        // 强制只能复制
        drag->exec(Qt::CopyAction);
    }
};

class MyFileView : public QTreeView
{
    Q_OBJECT
public:
    explicit MyFileView(QWidget* parent = nullptr) : QTreeView(parent)
    {
        setAcceptDrops(true);
        setDragDropMode(QAbstractItemView::DropOnly);
        setDropIndicatorShown(true);
    }

signals:
    void filesDropped(const QStringList& paths, const QModelIndex& targetIndex);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override
    {
        if (event->mimeData()->hasUrls())
            event->acceptProposedAction();
        else
            QTreeView::dragEnterEvent(event);
    }

    void dragMoveEvent(QDragMoveEvent* event) override
    {
        if (event->mimeData()->hasUrls())
            event->acceptProposedAction();
        else
            QTreeView::dragMoveEvent(event);
    }

    void dropEvent(QDropEvent* event) override
    {
        if (event->mimeData()->hasUrls())
        {
            QStringList paths;
            for (const QUrl& url : event->mimeData()->urls())
            {
                if (url.isLocalFile()) paths << url.toLocalFile();
            }
            QModelIndex targetIndex = indexAt(event->pos());
            emit filesDropped(paths, targetIndex);
            event->acceptProposedAction();
        }
        else
            QTreeView::dropEvent(event);
    }
};

// 居中对齐
class CenterAlignDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override
    {
        QStyledItemDelegate::initStyleOption(option, index);
        option->displayAlignment = Qt::AlignCenter;
    }
};

// 左对齐
class LeftAlignDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override
    {
        QStyledItemDelegate::initStyleOption(option, index);
        option->displayAlignment = Qt::AlignLeft;
    }
};

// 右对齐
class RightAlignDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override
    {
        QStyledItemDelegate::initStyleOption(option, index);
        option->displayAlignment = Qt::AlignRight;
    }
};

// 样式
inline void buttonQss(QPushButton* button)
{
    button->setStyleSheet(
        // 默认状态
        "QPushButton {"
        "   background-color: #ffffff;"      // 透明背景
        "   color: #424242;"                    // 深灰色文字
        "   border: 1px solid rgba(0,0,0,0.1);" // 极细浅灰色边框
        "   border-radius: 6px;"                // 圆角
        "   padding: 8px 12px;"                 // 内边距
        "}"

        // 鼠标悬停状态（轻微亮起）
        "QPushButton:hover {"
        "   background-color: rgba(0,0,0,0.03);" // 极浅灰色半透明
        "   border-color: rgba(0,0,0,0.15);"     // 稍微加深边框
        "}"

        // 鼠标按下状态（极简点击效果）
        "QPushButton:pressed {"
        "   background-color: rgba(0,0,0,0.05);" // 比悬停稍深的灰色
        "   border: 1px solid transparent;"      /* 移除边框 */
        "   padding-top: 9px;"                   // 轻微下沉效果
        "   padding-bottom: 7px;"
        "}"
    );
}

// 深色模式
inline void buttonQssDark(QPushButton* button)
{
    button->setStyleSheet(
        "QPushButton {"
        "   background-color: #3c4b4f;"
        "   color: #d0e0dc;"
        "   border: 1px solid #5a6e6a;"
        "   border-radius: 6px;"
        "   padding: 8px 12px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #4c5f63;"
        "   border-color: #7a8e8a;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #2e3b3f;"
        "   border: 1px solid transparent;"
        "   padding-top: 9px;"
        "   padding-bottom: 7px;"
        "}"
    );
}

// 4、收藏下拉框样式
inline void comboboxQss(QComboBox* box)
{
    box->setStyleSheet(R"(
        QComboBox {
            background-color: #ffffff;
            border: 1px solid #e0e0e0;
            border-radius: 6px;
            padding: 8px 7px 8px 16px;
            font-size: 11pt;
            color: #424242;
            selection-background-color: #2196F3;
            selection-color: white;
        }
        QComboBox:hover {
            border: 1px solid transparent;
            background-color: rgba(33, 150, 243, 0.1);
            box-shadow: 0 2px 5px rgba(0, 0, 0, 0.2);
        }
        QComboBox:focus {
            border-color: #2196F3;
            outline: none;
            box-shadow: 0 0 8px rgba(33, 150, 243, 0.4);
        }
        QComboBox::drop-down {
            background-color: transparent;
            width: 30px;
        }
        QComboBox QAbstractItemView {
            border: none;
            background-color: #ffffff;
            selection-background-color: #bbdefb;
            selection-color: #424242;
            outline: none;
            box-shadow: 0 2px 5px rgba(0, 0, 0, 0.15);
        }
        QComboBox QAbstractItemView::item {
            padding: 12px 20px;
            color: #424242;
            font-weight: 400;
        }
        QComboBox QAbstractItemView::item:hover {
            background-color: #f5f5f5;
        }
        QComboBox QAbstractItemView::separator {
            height: 1px;
            background-color: #e0e0e0;
            margin: 8px 0;
        }
        QComboBox::down-arrow {
            image: url(:/up-1.png);
            width: 17px;
            height: 17px;
            margin-right: 27px;
            margin-top: 2px;
        }
        QComboBox::down-arrow:on {
            image: url(:/down-1.png);
        }
                       )");
}

inline void comboboxQssDark(QComboBox* box)
{
    box->setStyleSheet(R"(
        QComboBox {
            background-color: #3c4b4f;
            border: 1px solid #5a6e6a;
            border-radius: 6px;
            padding: 8px 7px 8px 16px;
            font-size: 11pt;
            color: #d0e0dc;
            selection-background-color: #4c5f63;
            selection-color: #ffffff;
        }
        QComboBox:hover {
            border: 1px solid #7a8e8a;
            background-color: #4c5f63;
            /* box-shadow: 0 2px 5px rgba(0,0,0,0.3); */
        }
        QComboBox:focus {
            border-color: #2196F3;
            outline: none;
            /* box-shadow: 0 0 8px rgba(33,150,243,0.4); */
        }
        QComboBox::drop-down {
            background-color: transparent;
            width: 30px;
        }
        QComboBox QAbstractItemView {
            border: none;
            background-color: #2e3b3f;
            selection-background-color: #4c5f63;
            selection-color: #ffffff;
            outline: none;
            /* box-shadow: 0 2px 5px rgba(0,0,0,0.25); */
        }
        QComboBox QAbstractItemView::item {
            padding: 12px 20px;
            color: #d0e0dc;
            font-weight: 400;
        }
        QComboBox QAbstractItemView::item:hover {
            background-color: #3c4b4f;
        }
        QComboBox QAbstractItemView::separator {
            height: 1px;
            background-color: #5a6e6a;
            margin: 8px 0;
        }
        QComboBox::down-arrow {
            image: url(:/up-2.png);
            width: 17px;
            height: 17px;
            margin-right: 27px;
            margin-top: 2px;
        }
        QComboBox::down-arrow:on {
            image: url(:/down-2.png);
        }
    )");
}

// 图片预览适配
class ScalableImageLabel : public QLabel
{
public:
    explicit ScalableImageLabel(QWidget* parent = nullptr)
        : QLabel(parent)
    {
        setAlignment(Qt::AlignCenter);
    }

    // Override the setPixmap method (without writing override)
    void setPixmap(const QPixmap& pix)
    {
        originalPixmap = pix;
        updateScaledPixmap();
    }

protected:
    // 每次控件改变大小时重新计算比例
    void resizeEvent(QResizeEvent* event) override
    {
        QLabel::resizeEvent(event);
        updateScaledPixmap();
    }

private:
    QPixmap originalPixmap;

    void updateScaledPixmap()
    {
        if (!originalPixmap.isNull())
            QLabel::setPixmap(originalPixmap.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
};
// 鼠标部分事件拦截
class NoClickFilter : public QObject
{
    Q_OBJECT
public:
    explicit NoClickFilter(QObject* parent = nullptr) : QObject(parent) {}

protected:
    bool eventFilter(QObject* watched, QEvent* event) override
    {
        // 只拦截鼠标相关事件
        if (event->type() == QEvent::MouseButtonPress ||
            event->type() == QEvent::MouseButtonRelease ||
            event->type() == QEvent::MouseButtonDblClick ||
            event->type() == QEvent::KeyPress ||
            event->type() == QEvent::KeyRelease)
        {
            return true; // Shield
        }
        return QObject::eventFilter(watched, event);
    }
};

class RightClickDoubleClickFilter : public QObject
{
    Q_OBJECT
public:
    explicit RightClickDoubleClickFilter(QObject* parent = nullptr)
      : QObject(parent) {}

protected:
    bool eventFilter(QObject* watched, QEvent* ev) override
    {
        // 如果是鼠标右键双击事件就屏蔽掉
        if (ev->type() == QEvent::MouseButtonDblClick)
        {
            auto* me = static_cast<QMouseEvent*>(ev);
            if (me->button() == Qt::RightButton) return true;
        }

        // 允许所有其他事件
        return QObject::eventFilter(watched, ev);
    }
};

// 显示内容行间距
class SpacingDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit SpacingDelegate(int extraHeight = 7, QObject* parent = nullptr)
        : QStyledItemDelegate(parent), m_extra(extraHeight) {}

    QSize sizeHint(const QStyleOptionViewItem& opt, const QModelIndex& idx) const override
    {
        QSize s = QStyledItemDelegate::sizeHint(opt, idx);
        return QSize(s.width(), s.height() + m_extra);
    }

private:
    int m_extra;
};

#endif // UTILS_H
