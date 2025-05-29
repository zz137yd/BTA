/*menuBar = new QMenuBar();
QPalette pal = this->palette();
QString bgColor = pal.color(QPalette::Window).name();
menuBar->setStyleSheet(QString("QMenuBar { background-color: %1; }").arg(bgColor));
this->setMenuBar(menuBar);

QMenu* menu1 = new QMenu("分类");
menuBar->addMenu(menu1);

QAction* act1 = new QAction("工作");
QAction* act2 = new QAction("个人");
QAction* act3 = new QAction("关键内容");
QAction* act4 = new QAction("娱乐");
menu1->addAction(act1);
menu1->addAction(act2);
menu1->addAction(act3);
menu1->addAction(act4);
connect(act1, &QAction::triggered, this, &MainWindow::handle);
connect(act2, &QAction::triggered, this, &MainWindow::handle);
connect(act3, &QAction::triggered, this, &MainWindow::handle);
connect(act4, &QAction::triggered, this, &MainWindow::handle);*/

// 搜索
/*searchLineEdit = new QLineEdit(this);
searchLineEdit->setFixedHeight(47);
QFont fonts = searchLineEdit->font();
fonts.setPointSize(11);
searchLineEdit->setFont(fonts);

buttonLayout->addWidget(searchLineEdit);
//connect(searchLineEdit, &QLineEdit::returnPressed, this, &MainWindow::fileSearch);*/


/*leftView->setContextMenuPolicy(Qt::CustomContextMenu);
connect(leftView, &QTreeView::customContextMenuRequested, this, [this](const QPoint &pos) {
    QModelIndex index = leftView->indexAt(pos);
    if (!index.isValid()) return;

    QMenu menu;
    QAction *addFavAct = new QAction("加入收藏", &menu);
    menu.addAction(addFavAct);
    connect(addFavAct, &QAction::triggered, this, &MainWindow::slotAddToFavorites);
    menu.exec(leftView->viewport()->mapToGlobal(pos));
});*/

/*void MainWindow::handle()
{
    QAction* act = qobject_cast<QAction*>(sender());
    if (!act) return;
    QString category = act->text();
    openFavoritesDialog(category);
}

void MainWindow::slotAddToFavorites()
{
    QModelIndex index = leftView->currentIndex();
    if (!index.isValid()) return;

    QString path = fileModel->filePath(index);
    QStringList categories = {"工作", "个人", "关键内容", "娱乐"};

    // 使用自定义的 QInputDialog 对象
    QInputDialog inputDialog(this);
    inputDialog.setWindowTitle("加入收藏");
    inputDialog.setLabelText("请选择收藏类别：");
    inputDialog.setComboBoxItems(categories);
    inputDialog.setComboBoxEditable(false);
    // 设置较大的字体
    QFont font("Microsoft YaHei", 12);
    inputDialog.setFont(font);
    // 调整对话框的初始尺寸
    inputDialog.resize(400, 200);

    if (inputDialog.exec() == QDialog::Accepted) {
        QString category = inputDialog.textValue();
        if (category.isEmpty()) return;
        if (!favorites[category].contains(path))
        {
            favorites[category].append(path);
            QMessageBox::information(this, "加入收藏", QString("成功将路径 \"%1\" 加入到“%2”收藏中。").arg(path, category));
        }
        else
        {
            QMessageBox::information(this, "加入收藏", QString("路径 \"%1\" 已经存在于“%2”收藏中。").arg(path, category));
        }
    }
}


void MainWindow::openFavoritesDialog(const QString &category)
{
    QDialog dialog(this);
    dialog.setWindowTitle(QString("收藏 - %1").arg(category));
    // 设置对话框初始尺寸和最小尺寸（用户可拖动改变大小）
    dialog.resize(800, 600);
    dialog.setMinimumSize(600, 400);

    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);

    // 用于显示当前路径的标签，并调整字体大小
    QLabel *pathLabel = new QLabel(QString("收藏列表: %1").arg(category));
    QFont labelFont = pathLabel->font();
    labelFont.setPointSize(12);
    pathLabel->setFont(labelFont);
    mainLayout->addWidget(pathLabel);

    // “上一级”按钮，初始隐藏（在收藏列表模式下无意义）
    QPushButton *upButton = new QPushButton("上一级");
    upButton->setFont(QFont("Microsoft YaHei", 12));
    upButton->hide();
    mainLayout->addWidget(upButton);

    // 使用堆栈Widget管理两种视图：收藏列表和文件浏览视图
    QStackedWidget *stack = new QStackedWidget;
    mainLayout->addWidget(stack);

    // 视图1：收藏列表（QListWidget）
    QListWidget *favList = new QListWidget;
    favList->setFont(QFont("Microsoft YaHei", 12));
    // 填充该类别下收藏的路径
    QStringList favPaths = favorites.value(category);
    for (const QString &path : favPaths)
    {
        QFileInfo info(path);
        QListWidgetItem *item = new QListWidgetItem(info.fileName());
        item->setData(Qt::UserRole, path);
        if (info.isDir())
            item->setIcon(QIcon(":/folder.png"));
        else
            item->setIcon(QIcon(":/file.png"));
        favList->addItem(item);
    }
    stack->addWidget(favList);

    // 视图2：文件浏览视图（QTreeView）
    QTreeView *fileView = new QTreeView;
    fileView->setModel(fileModel);
    fileView->setFont(QFont("Microsoft YaHei", 12));
    stack->addWidget(fileView);

    // 当前浏览路径，空表示在收藏列表模式
    QString currentPath = "";

    // 点击收藏列表项
    connect(favList, &QListWidget::itemDoubleClicked, this, [=, &currentPath, &pathLabel, &stack, &upButton](QListWidgetItem *item) mutable {
        QString path = item->data(Qt::UserRole).toString();
        QFileInfo info(path);
        if (info.isDir()) {
            currentPath = path;
            pathLabel->setText(path);
            upButton->show();
            QModelIndex idx = fileModel->index(path);
            fileView->setRootIndex(idx);
            stack->setCurrentWidget(fileView);
        } else {
            QDesktopServices::openUrl(QUrl::fromLocalFile(path));
        }
    });

    // 在文件浏览视图中双击，目录则进入，文件则打开
    connect(fileView, &QTreeView::doubleClicked, this, [=, &currentPath, &pathLabel](const QModelIndex &index) mutable {
        QString path = fileModel->filePath(index);
        QFileInfo info(path);
        if (info.isDir()) {
            currentPath = path;
            pathLabel->setText(path);
            QModelIndex idx = fileModel->index(path);
            fileView->setRootIndex(idx);
        } else {
            QDesktopServices::openUrl(QUrl::fromLocalFile(path));
        }
    });

    // “上一级”按钮逻辑
    connect(upButton, &QPushButton::clicked, this, [=, &currentPath, &pathLabel, &stack, &favList]() mutable {
        if (currentPath.isEmpty()) return;
        QDir dir(currentPath);
        if (dir.cdUp()) {
            currentPath = dir.absolutePath();
            pathLabel->setText(currentPath);
            QModelIndex idx = fileModel->index(currentPath);
            fileView->setRootIndex(idx);
        } else {
            // 已到顶级目录，返回收藏列表模式
            currentPath = "";
            pathLabel->setText(QString("收藏列表: %1").arg(category));
            upButton->hide();
            stack->setCurrentWidget(favList);
        }
    });

    dialog.exec();
}*/

// 顶部菜单栏
/*#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QPalette>

#include <QStringList>
#include <QInputDialog>
#include <QStackedWidget>
#include <QMessageBox>*/


/*  // 压缩
void MainWindow::onCompressRequested(const QString& format)
{
    clearCutStateIfNeeded();
    QModelIndex index = leftView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(this, tr("压缩失败"), tr("请先选中文件或文件夹"));
        return;
    }

    QString currentPath = fileModel->filePath(index);
    if (currentPath.isEmpty()) {
        QMessageBox::warning(this, tr("压缩失败"), tr("路径为空"));
        return;
    }

    if (format == "zip") {
        handleZipCompress(currentPath);
    }
    else if (format == "7z") {
        handle7zCompress(currentPath);
    }
    else if (format == "tar.gz") {
        handleTarGzCompress(currentPath);
    }
    else {
        QMessageBox::warning(this, tr("压缩失败"), tr("未知的压缩格式：%1").arg(format));
    }
}

// 剪切
void MainWindow::onCutRequested()
{
    QModelIndex idx = leftView->currentIndex();
    if (!idx.isValid()) return;

    // 先清空旧状态
    clipPaths.clear();
    // 记录这次剪切的路径
    clipPaths.append(fileModel->filePath(idx));
    clipIsCut = true;

    // 立即刷新，让这行变灰
    leftView->viewport()->update();
    rightView->viewport()->update();
}


// 复制
void MainWindow::onCopyRequested()
{
    QModelIndex idx = leftView->currentIndex();
    if (!idx.isValid()) return;
    clipPaths.clear();
    clipPaths.append(fileModel->filePath(idx));
    clipIsCut = false;
}

// 粘贴
void MainWindow::onPasteRequested()
{
    if (clipPaths.isEmpty()) return;

    QString dstDir = fileModel->filePath(leftView->rootIndex());
    for (const QString &src : clipPaths) {
        QFileInfo fi(src);
        QString dstPath = dstDir + QDir::separator() + fi.fileName();
        if (clipIsCut) {
            // 剪切：重命名
            QFile::rename(src, dstPath);
        } else {
            // 复制：文件或目录
            if (fi.isDir())
                copyDirectory(src, dstPath);
            else
                QFile::copy(src, dstPath);
        }
    }
    if (clipIsCut)
        clipPaths.clear();    // 剪切一次后清空
    updateRightView();

    if (clipIsCut) {
        // 粘贴完成后清空剪切状态
        clipPaths.clear();
        clipIsCut = false;
    }
    updateRightView();

    // 刷新视图，去掉灰显
    leftView->viewport()->update();
    rightView->viewport()->update();
}

// 删除
bool MainWindow::moveToRecycleBin(const QString& path)
{
#ifdef Q_OS_WIN
    // SHFILEOPSTRUCTW 要求双空结尾
    std::wstring wpath = path.toStdWString();
    std::vector<wchar_t> from(wpath.begin(), wpath.end());
    from.push_back(L'\0');
    from.push_back(L'\0');

    SHFILEOPSTRUCTW fileOp;
    fileOp.wFunc = FO_DELETE;
    fileOp.pFrom = from.data();
    fileOp.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_SILENT;

    int res = SHFileOperationW(&fileOp);
    return (res == 0 && !fileOp.fAnyOperationsAborted);
#else
    Q_UNUSED(path);
    return false;
#endif
}

bool MainWindow::moveToTrash(const QString& path)
{
    // 尝试使用 gio trash（GNOME），或 osascript（macOS）
#ifdef Q_OS_MAC
    QProcess p;
    QString script = QString("tell application \"Finder\" to delete POSIX file \"%1\"").arg(path);
    p.start("osascript", QStringList() << "-e" << script);
    p.waitForFinished();
    return (p.exitCode() == 0);
#else
    QProcess p;
    p.start("gio", QStringList() << "trash" << path);
    p.waitForFinished();
    return (p.exitCode() == 0);
#endif
}

void MainWindow::onTrashRequested()
{
    clearCutStateIfNeeded();

    if (!currentContextIndex.isValid()) return;
    QString path = fileModel->filePath(currentContextIndex);
    bool ok = false;

#ifdef Q_OS_WIN
    ok = moveToRecycleBin(path);
#else
    ok = moveToTrash(path);
#endif

    if (!ok) {
        QMessageBox::warning(this, tr("操作失败"),
                             tr("无法将 \"%1\" 移到回收站").arg(path));
    } else {
        // 刷新视图
        updateRightView();
    }
}

// 彻底删除
void MainWindow::onDeleteRequested()
{
    clearCutStateIfNeeded();

    QModelIndex idx = leftView->currentIndex();
    if (!idx.isValid()) return;

    QString path = fileModel->filePath(idx);
    QFileInfo fi(path);
    auto btn = QMessageBox::question(
        this, tr("删除"), tr("确认删除 \"%1\"？").arg(fi.fileName()),
        QMessageBox::Yes|QMessageBox::No);
    if (btn == QMessageBox::Yes) {
        bool ok = fi.isDir()
            ? QDir(path).removeRecursively()
            : QFile::remove(path);
        if (!ok)
            QMessageBox::warning(this, tr("删除失败"), tr("无法删除选中文件/文件夹"));
        updateRightView();
    }
}*/

/*void MainWindow::onPropertiesRequested()
{
    clearCutStateIfNeeded();

    QModelIndex idx = leftView->currentIndex();
    if (!idx.isValid()) return;
    QFileInfo fi(fileModel->filePath(idx));

    QString info;
    info += tr("路径： %1\n").arg(fi.absoluteFilePath());
    info += tr("类型： %1\n").arg(fi.isDir() ? tr("文件夹") : tr("文件"));
    if (fi.isFile())
        info += tr("大小： %1 字节\n").arg(fi.size());
    // 用 birthTime() 代替已弃用的 created()
    info += tr("创建： %1\n").arg(
        fi.birthTime().toString("yyyy-MM-dd hh:mm:ss"));
    info += tr("修改： %1\n").arg(
        fi.lastModified().toString("yyyy-MM-dd hh:mm:ss"));
    info += tr("只读： %1\n").arg(
        fi.isWritable() ? tr("否") : tr("是"));

    QMessageBox::information(this, tr("属性"), info);
}*/

/*void MainWindow::onPropertiesRequested()
{
    clearCutStateIfNeeded();

    QModelIndex idx = leftView->currentIndex();
    if (!idx.isValid()) return;

    QString path = fileModel->filePath(idx);
    std::wstring wPath = path.toStdWString();

    SHELLEXECUTEINFOW sei;
    sei.cbSize = sizeof(sei);
    sei.fMask = SEE_MASK_INVOKEIDLIST;
    sei.hwnd = reinterpret_cast<HWND>(this->winId());
    sei.lpVerb = L"properties";
    sei.lpFile = wPath.c_str();
    sei.nShow = SW_SHOW;

    ShellExecuteExW(&sei);
}*/

/*void MainWindow::openFavoritesDialog(const QString &category)
{
    QDialog dialog(this);
    dialog.setWindowTitle(QString("收藏 - %1").arg(category));
    // 设置对话框初始尺寸和最小尺寸（用户可拖动改变大小）
    dialog.resize(800, 600);
    dialog.setMinimumSize(600, 400);

    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);

    // 用于显示当前路径的标签，并调整字体大小
    QLabel *pathLabel = new QLabel(QString("收藏列表: %1").arg(category));
    QFont labelFont = pathLabel->font();
    labelFont.setPointSize(12);
    pathLabel->setFont(labelFont);
    mainLayout->addWidget(pathLabel);

    // “上一级”按钮，初始隐藏（在收藏列表模式下无意义）
    QPushButton *upButton = new QPushButton("上一级");
    upButton->setFont(QFont("Microsoft YaHei", 12));
    upButton->hide();
    mainLayout->addWidget(upButton);

    // 使用堆栈Widget管理两种视图：收藏列表和文件浏览视图
    QStackedWidget *stack = new QStackedWidget;
    mainLayout->addWidget(stack);

    // 视图1：收藏列表（QListWidget）
    QListWidget *favList = new QListWidget;
    favList->setFont(QFont("Microsoft YaHei", 12));
    // 填充该类别下收藏的路径
    QStringList favPaths = favorites.value(category);
    for (const QString &path : favPaths)
    {
        QFileInfo info(path);
        QListWidgetItem *item = new QListWidgetItem(info.fileName());
        item->setData(Qt::UserRole, path);
        if (info.isDir())
            item->setIcon(QIcon(":/folder.png"));
        else
            item->setIcon(QIcon(":/file.png"));
        favList->addItem(item);
    }
    stack->addWidget(favList);

    // 视图2：文件浏览视图（QTreeView）
    QTreeView *fileView = new QTreeView;
    fileView->setModel(fileModel);
    fileView->setFont(QFont("Microsoft YaHei", 12));
    stack->addWidget(fileView);

    // 当前浏览路径，空表示在收藏列表模式
    QString currentPath = "";

    // 点击收藏列表项
    connect(favList, &QListWidget::itemDoubleClicked, this, [=, &currentPath, &pathLabel, &stack, &upButton](QListWidgetItem *item) mutable {
        QString path = item->data(Qt::UserRole).toString();
        QFileInfo info(path);
        if (info.isDir()) {
            currentPath = path;
            pathLabel->setText(path);
            upButton->show();
            QModelIndex idx = fileModel->index(path);
            fileView->setRootIndex(idx);
            stack->setCurrentWidget(fileView);
        } else {
            QDesktopServices::openUrl(QUrl::fromLocalFile(path));
        }
    });

    // 在文件浏览视图中双击，目录则进入，文件则打开
    connect(fileView, &QTreeView::doubleClicked, this, [=, &currentPath, &pathLabel](const QModelIndex &index) mutable {
        QString path = fileModel->filePath(index);
        QFileInfo info(path);
        if (info.isDir()) {
            currentPath = path;
            pathLabel->setText(path);
            QModelIndex idx = fileModel->index(path);
            fileView->setRootIndex(idx);
        } else {
            QDesktopServices::openUrl(QUrl::fromLocalFile(path));
        }
    });

    // “上一级”按钮逻辑
    connect(upButton, &QPushButton::clicked, this, [=, &currentPath, &pathLabel, &stack, &favList]() mutable {
        if (currentPath.isEmpty()) return;
        QDir dir(currentPath);
        if (dir.cdUp()) {
            currentPath = dir.absolutePath();
            pathLabel->setText(currentPath);
            QModelIndex idx = fileModel->index(currentPath);
            fileView->setRootIndex(idx);
        } else {
            // 已到顶级目录，返回收藏列表模式
            currentPath = "";
            pathLabel->setText(QString("收藏列表: %1").arg(category));
            upButton->hide();
            stack->setCurrentWidget(favList);
        }
    });

    dialog.exec();
}*/

/*void MainWindow::showContextMenu(const QPoint &pt)
{
    // pt 是相对于 viewport() 的坐标
    // 记录当前右键位置对应的 index（可能无效）
    currentContextIndex = leftView->viewport()->hasFocus()
                        ? leftView->indexAt(pt)
                        : rightView->indexAt(pt);

    // 计算全局坐标并弹菜单
    QWidget *vp = leftView->viewport()->hasFocus()
                  ? static_cast<QWidget*>(leftView->viewport())
                  : static_cast<QWidget*>(rightView->viewport());
    QPoint globalPos = vp->mapToGlobal(pt);
    rightClickMenu->exec(globalPos);
}*/

/*// 取得左侧视图当前显示的目录
QModelIndex rootIdx = leftView->rootIndex();
QString currentDir = fileModel->filePath(rootIdx);
if (currentDir.isEmpty())
    currentDir = rootPath;

// 通过重设过滤器迫使模型刷新
fileModel->setFilter(fileModel->filter());

// 重新设置左侧视图和右侧视图的根索引
leftView->setRootIndex(fileModel->index(currentDir));
updateRightView();*/

/*void MainWindow::handleZipCompress(const QString& sourcePath, const QString& renameTo)
{
    QString zipPath = sourcePath + ".zip";

#ifdef Q_OS_WIN
    // Windows 上使用 PowerShell
    QString psCommand = QString("powershell -ExecutionPolicy Bypass -Command \"Compress-Archive -LiteralPath '%1' -DestinationPath '%2' -Force\"")
                            .arg(sourcePath, zipPath);
#elif defined(Q_OS_MAC) || defined(Q_OS_LINUX)
    // macOS / Linux 上使用 zip 命令
    QString psCommand = QString("zip -r '%1' '%2'").arg(zipPath, sourcePath);
#endif

    QProcess::startDetached(psCommand);
}

// 7Z 压缩
void MainWindow::handle7zCompress(const QString& sourcePath, const QString& renameTo)
{
    QString archivePath = sourcePath + ".7z";
    QString exe = "7z"; // 确保环境变量里有 7z

    QStringList args = { "a", archivePath, sourcePath };
    QProcess::startDetached(exe, args);
}

// tar.gz 压缩
void MainWindow::handleTarGzCompress(const QString& sourcePath, const QString& renameTo)
{
    QString archivePath = sourcePath + ".tar.gz";

#ifdef Q_OS_WIN
    // Windows：用 7z 两步走
    // 1) 打 tar
    QString tarPath = sourcePath + ".tar";
    QProcess::startDetached("7z", QStringList{ "a", "-ttar", tarPath, sourcePath });
    // 2) gzip
    QProcess::startDetached("7z", QStringList{ "a", "-tgzip", archivePath, tarPath });
    // 可选：删除中间 tar 文件
    QProcess::startDetached("cmd.exe", QStringList{ "/c", QString("del \"%1\"").arg(tarPath) });
#elif defined(Q_OS_MAC) || defined(Q_OS_LINUX)
    // macOS/Linux：直接 tar -czf
    QFileInfo fi(sourcePath);
    QString parent = fi.absolutePath();
    QString name   = fi.fileName();
    // cd 到父目录再打包，保证压缩包内没有多余路径
    QStringList args = { "-czf", archivePath, "-C", parent, name };
    QProcess::startDetached("tar", args);
#endif
}*/

/*void MainWindow::Compressions(const QString &format)
{
    // 获取所有选中的索引（只取第一列）
    QModelIndexList selected = leftView->selectionModel()->selectedIndexes();
    QModelIndexList files;
    for (const QModelIndex &idx : selected)
    {
        if (idx.column() == 0) files.append(idx);
    }

    if (files.isEmpty())
    {
        QMessageBox::warning(this, tr("压缩失败"), tr("请先选中文件或文件夹"));
        return;
    }

    // 如果只有一个项目，直接压缩该项目
    if (files.size() == 1)
    {
        QString path = fileModel->filePath(files.first());
        if (path.isEmpty())
        {
            QMessageBox::warning(this, tr("压缩失败"), tr("路径为空"));
            return;
        }

        if (format == "zip")            {   handleZipCompress(path);     }
        else if (format == "7z")        {   handle7zCompress(path);      }
        else if (format == "tar.gz")    {   handleTarGzCompress(path);   }
        else
            QMessageBox::warning(this, tr("压缩失败"), tr("未知的压缩格式：%1").arg(format));
    }
    else
    {
        // 多个项目：在当前目录创建一个临时目录，将所有选中项拷贝进去，再压缩该临时目录
        QString dstDir = fileModel->filePath(leftView->rootIndex());
        if (dstDir.isEmpty())
            dstDir = rootPath;
        QString tempFolderName = "Archive_" + QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
        QString tempFolderPath = QDir(dstDir).filePath(tempFolderName);
        QDir().mkpath(tempFolderPath);

        // 拷贝所有选中文件/文件夹
        for (const QModelIndex &idx : files)
        {
            QString src = fileModel->filePath(idx);
            QFileInfo fi(src);
            QString dest = QDir(tempFolderPath).filePath(fi.fileName());
            if (fi.isDir())
                copyDirectory(src, dest);
            else
                QFile::copy(src, dest);
        }

        // 压缩临时目录
        if (format == "zip")            {   handleZipCompress(tempFolderPath);     }
        else if (format == "7z")        {   handle7zCompress(tempFolderPath);      }
        else if (format == "tar.gz")    {   handleTarGzCompress(tempFolderPath);   }
        else
            QMessageBox::warning(this, tr("压缩失败"), tr("未知的压缩格式：%1").arg(format));
        // 注意：压缩完成后可选择删除临时目录，可用 QDir(tempFolderPath).removeRecursively();
    }
}*/


/*
// ZIP 压缩
void MainWindow::handleZipCompress(const QString& sourcePath, const QString& archivePath)
{
#ifdef Q_OS_WIN
    QString psCommand = QString("powershell -ExecutionPolicy Bypass -Command \"Compress-Archive -LiteralPath '%1' -DestinationPath '%2' -Force\"")
            .arg(sourcePath, archivePath);
    QProcess* proc = new QProcess(this);
    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [=](int, QProcess::ExitStatus){ proc->deleteLater();});
    proc->start(psCommand);
#elif defined(Q_OS_MAC) || defined(Q_OS_LINUX)
    QStringList args = { "-r", archivePath, sourcePath };
    QProcess* proc = new QProcess(this);
    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [=](int, QProcess::ExitStatus){ proc->deleteLater();});
    proc->start("zip", args);
#endif
}

// 7z压缩
void MainWindow::handle7zCompress(const QString& sourcePath, const QString& archivePath)
{
    QString exe = "7z"; // 需确保环境变量有7z
    QStringList args = { "a", archivePath, sourcePath };

    QProcess* proc = new QProcess(this);
    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [=](int, QProcess::ExitStatus){ proc->deleteLater(); });
    proc->start(exe, args);
}

// tar压缩
void MainWindow::handleTarGzCompress(const QString& sourcePath, const QString& archivePath)
{
#ifdef Q_OS_WIN
    // Windows：用7z两步走
    QString tarPath = archivePath;
    tarPath.chop(3); // 去掉.gz，得到 .tar
    QProcess* procTar = new QProcess(this);
    connect(procTar, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [=](int, QProcess::ExitStatus){
    // gzip
    QProcess* procGz = new QProcess(this);
    connect(procGz, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [=](int, QProcess::ExitStatus){
        // 可选：删除中间tar文件
        QProcess::startDetached("cmd.exe", QStringList{ "/c", QString("del \"%1\"").arg(tarPath) });
        procGz->deleteLater(); });
        procGz->start("7z", QStringList{ "a", "-tgzip", archivePath, tarPath });
        procTar->deleteLater();
    });
    procTar->start("7z", QStringList{ "a", "-ttar", tarPath, sourcePath });
#elif defined(Q_OS_MAC) || defined(Q_OS_LINUX)
    QFileInfo fi(sourcePath);
    QString parent = fi.absolutePath();
    QString name = fi.fileName();
    QStringList args = { "-czf", archivePath, "-C", parent, name };
    QProcess* proc = new QProcess(this);
    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [=](int, QProcess::ExitStatus){ proc->deleteLater(); });
    proc->start("tar", args);
#endif
}*/

/*void MainWindow::handleSearchFinished()
{
    QStringList results = m_searchWatcher.result();

    // 切到 flat list 模式
    searchModel->setStringList(results);

    // 为每一行设置图标
    for (int row = 0; row < results.size(); ++row)
    {
        const QString &path = results.at(row);
        QFileInfo fi(path);

        QModelIndex idx = searchModel->index(row, 0);

        QIcon icon = fi.isDir()
            ? QIcon(":/folder.png")
            : QIcon(":/file.png");
        searchModel->setData(idx, icon, Qt::DecorationRole);
    }

    leftView->setModel(searchModel);
    isSearch = true;
}*/

/*void MainWindow::showContextMenu(const QPoint& pos)
{
    QModelIndex index = leftView->indexAt(pos);

    if (index.isValid()) currentContextIndex = index;
    else currentContextIndex = QModelIndex();

    rightClickMenu->cutChecked(clipIsCut);
    rightClickMenu->itemsBlock(leftPath);
    rightClickMenu->exec(leftView->viewport()->mapToGlobal(pos));
}*/

/*void MainWindow::openFile()
{
    QModelIndex index = leftView->currentIndex();
    if (index.isValid())
    {
        openAsAdmin = false;
        leftView_doubleClicked(index);
    }
}*/

/*void MainWindow::openFile()
{
    if (currentContextPath.isEmpty()) return;

    QFileInfo fi(currentContextPath);

    if (fi.isDir())
    {
        if (isSearch)
        {
            isSearch = false;
            leftView->setModel(fileModel);
            //interfaceStyle();
            leftView->header()->restoreState(savedHeaderState);

            leftView->setItemDelegate(nullptr);
            leftView->setItemDelegate(new SpacingDelegate(17, this));
        }

        leftPath = currentContextPath;

        QModelIndex idx = fileModel->index(leftPath);
        if (idx.isValid())
        {
            leftView->setRootIndex(idx);
            updateStack(leftPath, pathStack);
            updateButtons(buttons, pathStack);
        }
    }
    else
        QDesktopServices::openUrl(QUrl::fromLocalFile(currentContextPath));
}
*/

/*void MainWindow::showContextMenu(const QPoint& pos)
{
    QModelIndex idx = leftView->indexAt(pos);

    if (idx.isValid())
    {
        currentContextIndex = idx;

        currentContextPath = isSearch
            ? searchModel->item(idx.row(), 0)->text()
            : fileModel->filePath(idx);
        qDebug() << currentContextPath;

        rightClickMenu->cutChecked(clipIsCut);
        rightClickMenu->itemsBlock(currentContextPath);
    }
    else
    {
        // 5) 点击在空白／列表末尾
        currentContextIndex = QModelIndex();

        rightClickMenu->cutChecked(false);
        rightClickMenu->whiteBlock(leftPath);
    }

    rightClickMenu->exec(leftView->viewport()->mapToGlobal(pos));
}*/

// 搜索
/*void MainWindow::search()
{
    // 只有当 leftPath 非空时才搜
    if (leftPath.isEmpty()) return;

    // 读取关键字
    QString key = searchEdit->text().trimmed();
    if (key.isEmpty()) return ;

    // 如果上一次搜索还在跑，就取消它
    if (searchWatcher.isRunning())
    {
        searchWatcher.cancel();
        searchWatcher.waitForFinished();
    }

    // 异步遍历当前目录及其子目录
    auto future = QtConcurrent::run([path = leftPath, key]() {
        QStringList matched;
        QDirIterator it(path,
                        QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot,
                        QDirIterator::Subdirectories);
        while (it.hasNext())
        {
            if (QThread::currentThread()->isInterruptionRequested()) break;
            it.next();
            QString name = it.fileName();
            if (name.contains(key, Qt::CaseInsensitive)) matched << it.filePath();
        }
        return matched;
    });

    searchWatcher.setFuture(future);
}

void MainWindow::handleSearchFinished()
{
    QStringList results = searchWatcher.result();

    searchModel->clear();
    searchModel->setHorizontalHeaderLabels({("Results")});

    for (const QString &path : results)
    {
        QFileInfo fi(path);
        QStandardItem* item = new QStandardItem;
        item->setText(path);
        item->setIcon(fi.isDir()
                      ? QIcon(":/folder.png")
                      : QIcon(":/file.png"));
        searchModel->appendRow(item);
    }

    leftView->setModel(searchModel);
    leftView->setItemDelegate(new SpacingDelegate(27, this));
    isSearch = true;
}*/

/*bool MainWindow::check7zAvailable()
{
    return !QStandardPaths::findExecutable("7z").isEmpty();
}

void MainWindow::runCompressProcess(const QString &program,
                                    const QStringList &arguments,
                                    const QString &archivePath)
{
    QApplication::processEvents();  // 强制刷新 UI

    if (compressProc) {
        compressProc->deleteLater();
        compressProc = nullptr;
    }

    compressProc = new QProcess(this);
    compressProc->setProgram(program);
    compressProc->setArguments(arguments);
    compressProc->setProcessChannelMode(QProcess::MergedChannels);

    // 创建进度框
    progressDialog = new QProgressDialog(tr("正在压缩，请稍候..."), tr("取消"), 0, 0, this);
    progressDialog->setWindowTitle(tr("压缩中"));
    progressDialog->setCancelButtonText(tr("取消"));
    progressDialog->setMinimumDuration(0);
    progressDialog->setWindowModality(Qt::ApplicationModal);
    progressDialog->setRange(0, 0);  // 设置为无限循环模式

    connect(progressDialog, &QProgressDialog::canceled, this, [=]() {
        if (compressProc && compressProc->state() == QProcess::Running) {
            compressProc->kill();
            compressProc->waitForFinished();
            QMessageBox::warning(this, tr("压缩已取消"), tr("压缩进程被取消。"));
        }
        progressDialog->deleteLater();
        progressDialog = nullptr;
    });

    connect(compressProc, &QProcess::readyReadStandardOutput, this, [=]() {
        QString output = compressProc->readAllStandardOutput();
        QStringList lines = output.split("\n");
        for (const QString &line : lines) {
            if (!line.trimmed().isEmpty()) {
                progressDialog->setLabelText(tr("正在压缩: %1").arg(line.trimmed()));
                break;
            }
        }
    });

    connect(compressProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [=](int code, QProcess::ExitStatus status) {
        if (progressDialog) {
            progressDialog->close();
            progressDialog->deleteLater();
            progressDialog = nullptr;
        }

        compressProc->deleteLater();
        compressProc = nullptr;

        if (status == QProcess::NormalExit && code == 0) {
            QMessageBox::information(this, tr("压缩完成"), tr("压缩成功：%1").arg(archivePath));
        } else {
            QMessageBox::critical(this, tr("压缩失败"), tr("压缩进程异常退出。"));
        }
    });

    compressProc->start();
}

// 统一的完成回调
void MainWindow::onCompressFinished(int exitCode, QProcess::ExitStatus status)
{
    if (progressDialog) {
        progressDialog->close();
        progressDialog->deleteLater();
        progressDialog = nullptr;
    }

    if (compressProc) {
        compressProc->deleteLater();
        compressProc = nullptr;
    }

    if (status == QProcess::NormalExit && exitCode == 0) {
        QMessageBox::information(this, tr("压缩完成"), tr("压缩已成功完成！"));
    } else {
        QMessageBox::warning(this, tr("压缩失败"),
                             tr("压缩进程异常退出（错误码 %1）").arg(exitCode));
    }
}*/

/*void MainWindow::Compressions(const QString &format)
{
    // 获取选中项
    QModelIndexList selected = leftView->selectionModel()->selectedIndexes();
    QModelIndexList files;
    for (const QModelIndex &idx : selected)
        if (idx.column() == 0) files.append(idx);

    if (files.isEmpty()) {
        QMessageBox::warning(this, tr("压缩失败"), tr("请先选中文件或文件夹"));
        return;
    }

    QString ext;
    if (format == "zip") ext = ".zip";
    else if (format == "7z") ext = ".7z";
    else if (format == "tar.gz") ext = ".tar.gz";
    else {
        QMessageBox::warning(this, tr("压缩失败"), tr("未知的压缩格式：%1").arg(format));
        return;
    }

    QString compressTargetPath;
    QString suggestName;
    QString targetDir;

    if (files.size() == 1) {
        compressTargetPath = fileModel->filePath(files.first());
        if (compressTargetPath.isEmpty()) {
            QMessageBox::warning(this, tr("压缩失败"), tr("路径为空"));
            return;
        }
        suggestName = QFileInfo(compressTargetPath).fileName() + ext;
        targetDir = QFileInfo(compressTargetPath).absolutePath();
    } else {
        QString dstDir = fileModel->filePath(leftView->rootIndex());
        if (dstDir.isEmpty())
            dstDir = rootPath;
        QString tempFolderName = "Archive_" + QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
        QString tempFolderPath = QDir(dstDir).filePath(tempFolderName);
        QDir().mkpath(tempFolderPath);

        for (const QModelIndex &idx : files) {
            QString src = fileModel->filePath(idx);
            QFileInfo fi(src);
            QString dest = QDir(tempFolderPath).filePath(fi.fileName());
            if (fi.isDir())
                copyDirectory(src, dest);
            else
                QFile::copy(src, dest);
        }
        compressTargetPath = tempFolderPath;
        suggestName = tempFolderName + ext;
        targetDir = dstDir; // 在当前目录下生成压缩包
    }

    // 弹出重命名对话框
    QInputDialog dlg(this);
    dlg.setWindowTitle(tr("压缩包重命名"));
    dlg.setLabelText(tr("输入压缩包新名称（含扩展名）："));
    dlg.setTextValue(suggestName);
    dlg.setOkButtonText(tr("确定"));
    dlg.setCancelButtonText(tr("取消"));

    dlg.resize(400, 300);
    font.setFamilies(QStringList() << "Microsoft YaHei" << "Malgun Gothic" << "Segoe UI");
    font.setPointSize(10);
    dlg.setFont(font);

    QString renameTo;
    if (dlg.exec() == QDialog::Accepted) {
        renameTo = dlg.textValue().trimmed();
    }
    if (renameTo.isEmpty())
        return;

    QString archivePath = QDir(targetDir).filePath(renameTo);

    // 调用异步压缩
    if (format == "zip")            { handleZipCompress(compressTargetPath, archivePath); }
    else if (format == "7z")        { handle7zCompress(compressTargetPath, archivePath); }
    else if (format == "tar.gz")    { handleTarGzCompress(compressTargetPath, archivePath); }
    if (format == "zip") {
        QString program = "zip";
        QStringList args = {"-r", archivePath, "."}; // 压缩当前目录
        QDir::setCurrent(compressTargetPath); // 切换工作目录（压缩里面内容）
        runCompressProcess(program, args, archivePath);
    }
    else if (format == "7z") {
        if (!check7zAvailable()) {
            QMessageBox::warning(this, "压缩失败", "未检测到 zip 命令，请确认已安装并配置到系统环境变量。");
            return;
        }
        QString program = "7z";
        QStringList args = {"a", archivePath, compressTargetPath};
        runCompressProcess(program, args, archivePath);
    }
    else if (format == "tar.gz") {
        QString program = "tar";
        QStringList args = {"-czf", archivePath, "-C", compressTargetPath, "."};;
        runCompressProcess(program, args, archivePath);
    }

}*/

/*void MainWindow::properties()
{
    leftView->setFocus();

    clearCutStateIfNeeded();

    QModelIndex idx = leftView->currentIndex();
    if (!idx.isValid()) return;

    QString path = fileModel->filePath(idx);
    qDebug() << "属性请求路径：" << path << "，是否存在：" << QFileInfo(path).exists();
    std::wstring wPath = path.toStdWString();

    SHELLEXECUTEINFOW sei;
    sei.cbSize = sizeof(sei);
    sei.fMask = SEE_MASK_INVOKEIDLIST;
    sei.hwnd = reinterpret_cast<HWND>(this->winId());
    sei.lpVerb = L"properties";
    sei.lpFile = wPath.c_str();
    sei.nShow = SW_SHOW;

    ShellExecuteExW(&sei);
}*/

/*// MacOS / Linux
bool MainWindow::moveToTrash(const QString& path)
{
    // 尝试使用 gio trash（GNOME），或 osascript（macOS）
#ifdef Q_OS_MAC
    QProcess p;
    QString script = QString("tell application \"Finder\" to delete POSIX file \"%1\"").arg(path);
    p.start("osascript", QStringList() << "-e" << script);
    p.waitForFinished();
    return (p.exitCode() == 0);
#else
    QProcess p;
    p.start("gio", QStringList() << "trash" << path);
    p.waitForFinished();
    return (p.exitCode() == 0);
#endif
}

// 移动到回收站
void MainWindow::toRecyleBin()
{
    QModelIndexList selected = leftView->selectionModel()->selectedIndexes();
    QModelIndexList files;
    for (const QModelIndex &idx : selected)
    {
        if (idx.column() == 0) files.append(idx);
    }
    if (files.isEmpty()) return;

    QStringList paths;
    for (const QModelIndex &idx : files)
    {
        paths << fileModel->filePath(idx);
    }
    bool allOk = true;
    for (const QString &p : paths)
    {
#ifdef Q_OS_WIN
        if (!moveToRecycleBin(p))
            allOk = false;
#else
        if (!moveToTrash(p))
            allOk = false;
#endif
    }
    if (!allOk)
        QMessageBox::warning(this, tr("操作失败"), tr("部分选项无法移到回收站"));
    else
        updateRightView();
}

// 彻底删除
void MainWindow::del()
{
    QModelIndexList selected = leftView->selectionModel()->selectedIndexes();
    QModelIndexList files;
    for (const QModelIndex &idx : selected)
    {
        if (idx.column() == 0) files.append(idx);
    }
    if (files.isEmpty()) return;

    // 汇总文件名以供提示
    QStringList names;
    QStringList paths;
    for (const QModelIndex &idx : files)
    {
        QString p = fileModel->filePath(idx);
        QFileInfo fi(p);
        names << fi.fileName();
        paths << p;
    }
    QString info = names.join(", ");
    auto btn = QMessageBox::question(this, tr("删除"),
                tr("确认删除以下项：\n%1").arg(info),
                QMessageBox::Yes | QMessageBox::No);
    if (btn == QMessageBox::Yes)
    {
        bool allOk = true;
        for (const QString &p : paths)
        {
            QFileInfo fi(p);
            bool ok = fi.isDir() ? QDir(p).removeRecursively() : QFile::remove(p);
            if (!ok)
                allOk = false;
        }
        if (!allOk) QMessageBox::warning(this, tr("删除失败"), tr("部分文件/文件夹删除失败"));
        updateRightView();
    }
}*/

/*void MainWindow::paste()
{
    leftView->setFocus();

    if (clipPaths.isEmpty()) return;

    // 目标目录：如果有选中项则取其所在目录，否则取左侧当前根目录
    QString dstDir;
    QModelIndexList selected = leftView->selectionModel()->selectedIndexes();
    if (!selected.isEmpty())
    {
        QModelIndex idx = selected.first();
        dstDir = QFileInfo(fileModel->filePath(idx)).absolutePath();
    }
    else
        dstDir = fileModel->filePath(leftView->rootIndex());

    for (const QString &src : clipPaths)
    {
        QFileInfo fi(src);
        QString dstPath = QDir(dstDir).filePath(fi.fileName());
        if (clipIsCut)
            QFile::rename(src, dstPath);
        else
        {
            if (fi.isDir()) copyDirectory(src, dstPath);
            else QFile::copy(src, dstPath);
        }
    }

    // 粘贴完成后清除剪切状态
    if (clipIsCut)
    {
        clipPaths.clear();
        clipIsCut = false;
    }
    updateRightView();
    leftView->viewport()->update();
    rightView->viewport()->update();
}*/

/*void paint(QPainter *painter, const QStyleOptionViewItem &option,
           const QModelIndex &index) const override {
    qint64 currentSizeBytes = getItemSizeBytes(index);
    qint64 maxSizeBytes = 0;
    QModelIndex parentIndex = index.parent();
    int rowCount = model->rowCount(parentIndex);
    for (int i = 0; i < rowCount; ++i) {
        QModelIndex siblingIndex = model->index(i, index.column(), parentIndex);
        qint64 siblingSize = getItemSizeBytes(siblingIndex);
        if (siblingSize > maxSizeBytes)
            maxSizeBytes = siblingSize;
    }

    int progress = 0;
    if (maxSizeBytes > 0)
        progress = static_cast<int>((double(currentSizeBytes) / maxSizeBytes) * 100);
    else
        progress = 0;

    QString text = formatSize(currentSizeBytes);
    QStyleOptionProgressBar progressBarOption;
    progressBarOption.rect = option.rect;
    progressBarOption.minimum = 0;
    progressBarOption.maximum = 100;
    progressBarOption.progress = progress;
    progressBarOption.text = text;
    progressBarOption.textVisible = true;
    progressBarOption.textAlignment = Qt::AlignCenter;
    QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);
}*/

/*void MainWindow::del()
{
    QModelIndexList selected = leftView->selectionModel()->selectedIndexes();
    QModelIndexList files;
    for (const QModelIndex &idx : selected)
        if (idx.column() == 0) files.append(idx);
    if (files.isEmpty()) return;

    QStringList names, paths;
    qint64 totalSize = 0;
    for (const QModelIndex &idx : files) {
        QString p = fileModel->filePath(idx);
        QFileInfo fi(p);
        names << fi.fileName();
        paths << p;
        if (fi.isDir()) {
            QDirIterator it(p, QDir::Files | QDir::NoDotAndDotDot | QDir::Dirs, QDirIterator::Subdirectories);
            while (it.hasNext()) {
                QFileInfo sfi(it.next());
                if (sfi.isFile()) totalSize += sfi.size();
            }
        } else {
            totalSize += fi.size();
        }
    }
    QString info = names.join(", ");
    auto btn = QMessageBox::question(this, tr("删除"),
                tr("确认删除以下项：\n%1").arg(info),
                QMessageBox::Yes | QMessageBox::No);
    if (btn != QMessageBox::Yes) return;

    QThread* thread = new QThread;
    FileDeleteTask* worker = new FileDeleteTask(paths, totalSize);
    worker->moveToThread(thread);

    QDialog* dialog = nullptr;
    QProgressBar* progressBar = nullptr;
    QLabel* label = nullptr;

    if (totalSize > 4.7 * 1024 * 1024 * 1024) {
        dialog = new QDialog(this);
        dialog->setWindowTitle("正在删除...");
        dialog->setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
        dialog->setWindowModality(Qt::NonModal);
        dialog->resize(500, 120);

        QVBoxLayout* layout = new QVBoxLayout(dialog);
        label = new QLabel(QString("正在删除，总大小：%1 GB").arg(totalSize / 1024.0 / 1024 / 1024, 0, 'f', 2));
        progressBar = new QProgressBar(dialog);
        progressBar->setRange(0, 100);
        layout->addWidget(label);
        layout->addWidget(progressBar);

        dialog->show();

        connect(worker, &FileDeleteTask::progress, this, [=](qint64 deleted, qint64 total) {
            int percent = total > 0 ? int(100.0 * deleted / total) : 0;
            progressBar->setValue(percent);
        });
    } else if (totalSize > 1 * 1024 * 1024 * 1024) {
        dialog = new QDialog(this);
        dialog->setWindowTitle("正在删除...");
        dialog->setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
        dialog->setWindowModality(Qt::NonModal);
        dialog->resize(400, 90);

        QVBoxLayout* layout = new QVBoxLayout(dialog);
        label = new QLabel("正在删除，请稍候...");
        layout->addWidget(label);

        dialog->show();
    }

    connect(thread, &QThread::started, [=]() { worker->doDelete(false); });
    connect(worker, &FileDeleteTask::finished, this, [=](bool success, const QString &err) {
        if (dialog) dialog->close();
        if (!success) QMessageBox::warning(this, "删除失败", err);
        else updateRightView();
        thread->quit();
        thread->wait();
        worker->deleteLater();
        thread->deleteLater();
        if (dialog) dialog->deleteLater();
    });
    thread->start();
}*/

// 显示区域某列居中对齐
/*CenterAlignDelegate* delegate = new CenterAlignDelegate(this);
leftView->setItemDelegateForColumn(1, delegate);
rightView->setItemDelegateForColumn(1, delegate);*/

/*void MainWindow::fileView_doubleClicked(const QModelIndex& index)
{
    QString path = fileModel->filePath(index);
    QFileInfo info(path);

    if (info.isDir())
    {
        mainPath = path;
        fPath = path;
        fIdx = fileModel->index(fPath);
        fileView->setRootIndex(fIdx);

        updateStack(fPath, fPathStack);
        updateButtons(fButtons, fPathStack);
    }
    else QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}*/

/*clearCutStateIfNeeded();
QModelIndex idx = leftView->currentIndex();
if (!idx.isValid()) return;
QString path = fileModel->filePath(idx);
// 直接用 QProcess 调用
QProcess::startDetached(application, QStringList() << path);*/

/*void FileCopyTask::doCopy()
{
    qint64 copied = 0;
    for (int i = 0; i < m_srcs.size(); ++i)
    {
        QFileInfo fi(m_srcs[i]);
        emit currentFile(fi.fileName());
        if (fi.isDir())
        {
            QDir srcDir(m_srcs[i]);
            QDir dstDir(m_dsts[i]);
            srcDir.mkpath(dstDir.absolutePath());
            QDirIterator it(m_srcs[i], QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);

            while (it.hasNext())
            {
                QString srcFile = it.next();
                QString relPath = QDir(m_srcs[i]).relativeFilePath(srcFile);
                QString dstFile = dstDir.filePath(relPath);
                QFileInfo sfi(srcFile);
                emit currentFile(sfi.fileName());
                if (sfi.isDir())
                    QDir().mkpath(dstFile);
                else
                {
                    QFile srcF(srcFile);
                    QFile dstF(dstFile);
                    srcF.open(QIODevice::ReadOnly);
                    dstF.open(QIODevice::WriteOnly);
                    const qint64 blockSize = 16 * 1024 * 1024;
                    while (!srcF.atEnd())
                    {
                        QByteArray buf = srcF.read(blockSize);
                        dstF.write(buf);
                        copied += buf.size();
                        emit progress(copied, m_total);
                        QCoreApplication::processEvents();
                    }
                    srcF.close();
                    dstF.close();
                }
            }
        }
        else
        {
            QFile srcF(m_srcs[i]);
            QFile dstF(m_dsts[i]);
            if (!srcF.open(QIODevice::ReadOnly) || !dstF.open(QIODevice::WriteOnly))
            {
                emit finished(false, QString("Unable to open file: %1").arg(m_srcs[i]));
                return;
            }
            const qint64 blockSize = 16 * 1024 * 1024;
            while (!srcF.atEnd())
            {
                QByteArray buf = srcF.read(blockSize);
                dstF.write(buf);
                copied += buf.size();
                emit progress(copied, m_total);
                QCoreApplication::processEvents();
            }
            srcF.close();
            dstF.close();
        }
    }
    emit finished(true, QString());
}

void MainWindow::paste()
{
    leftView->setFocus();
    if (clipPaths.isEmpty()) return;

    // Target Directory
    QString dstDir;
    QModelIndexList selected = leftView->selectionModel()->selectedIndexes();
    if (!selected.isEmpty())
        dstDir = QFileInfo(fileModel->filePath(selected.first())).absolutePath();
    else
        dstDir = fileModel->filePath(leftView->rootIndex());

    // Calculate all source and destination paths to be copied, as well as the total size
    QStringList srcs, dsts;
    qint64 totalSize = 0;
    for (const QString& src : clipPaths)
    {
        QFileInfo fi(src);
        QString dstPath = QDir(dstDir).filePath(fi.fileName());
        srcs << src;
        dsts << dstPath;
        if (fi.isDir())
        {
            QDirIterator it(src, QDir::Files | QDir::NoDotAndDotDot | QDir::Dirs, QDirIterator::Subdirectories);
            while (it.hasNext())
            {
                QFileInfo sfi(it.next());
                if (sfi.isFile()) totalSize += sfi.size();
            }
        }
        else totalSize += fi.size();
    }

    // Asynchronous replication
    QThread* thread = new QThread;
    FileCopyTask* worker = new FileCopyTask(srcs, dsts, totalSize);
    worker->moveToThread(thread);

    QDialog* dialog = nullptr;
    QProgressBar* progressBar = nullptr;
    QLabel* label = nullptr;

    // 4.7GB = 4.7 * 1024 * 1024 * 1024
    if (totalSize > 4.7 * 1024 * 1024 * 1024)
    {
        // Dialog box with progress bar
        dialog = new QDialog(this);
        dialog->setWindowTitle("Paste...");
        dialog->setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
        dialog->setWindowModality(Qt::NonModal);
        dialog->resize(500, 120);

        QVBoxLayout* layout = new QVBoxLayout(dialog);
        label = new QLabel("Pasting, please wait...");
        progressBar = new QProgressBar(dialog);
        progressBar->setRange(0, 100);
        layout->addWidget(label);
        layout->addWidget(progressBar);

        dialog->show();

        connect(worker, &FileCopyTask::progress, this, [=](qint64 copied, qint64 total) {
            int percent = total > 0 ? int(100.0 * copied / total) : 0;
            progressBar->setValue(percent);
            dialog->adjustSize();
        });
        connect(worker, &FileCopyTask::currentFile, this, [=](const QString& filename) {
            label->setText(QString("Pasting %1, please wait...").arg(filename));
            dialog->setFixedHeight(170);
            dialog->adjustSize();
            if (dialog->width() > 800)
                dialog->resize(800, dialog->width());
        });
    }
    else if (totalSize > 1 * 1024 * 1024 * 1024)
    {
        // A dialog box that only displays one sentence
        dialog = new QDialog(this);
        dialog->setWindowTitle("Pasting...");
        dialog->setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
        dialog->setWindowModality(Qt::NonModal);
        dialog->resize(400, 90);

        QVBoxLayout* layout = new QVBoxLayout(dialog);
        label = new QLabel("Pasting, please wait...");
        layout->addWidget(label);

        dialog->show();

        connect(worker, &FileCopyTask::currentFile, this, [=](const QString& filename) {
            label->setText(QString("Pasting %1, please wait...").arg(filename));
        });
    }

    connect(thread, &QThread::started, worker, &FileCopyTask::doCopy);
    connect(worker, &FileCopyTask::finished, this, [=](bool success, const QString &err) {
        if (dialog) dialog->close();
        if (!success) QMessageBox::warning(this, "Paste failed: ", err);
        else
        {
            // Clear the cut state after pasting is completed
            if (clipIsCut)
            {
                clipPaths.clear();
                clipIsCut = false;
            }
            updateRightView();
            leftView->viewport()->update();
            rightView->viewport()->update();
        }
        thread->quit();
        thread->wait();
        worker->deleteLater();
        thread->deleteLater();
        if (dialog) dialog->deleteLater();
    });
    thread->start();

    // Direct asynchronous copy of less than 1GB, no dialog box
    if (totalSize <= 1 * 1024 * 1024 * 1024)
    {
        connect(thread, &QThread::started, worker, &FileCopyTask::doCopy);
        connect(worker, &FileCopyTask::finished, this, [=](bool success, const QString &err) {
            if (!success) QMessageBox::warning(this, "Paste failed: ", err);
            else
            {
                if (clipIsCut)
                {
                    clipPaths.clear();
                    clipIsCut = false;
                }
                updateRightView();
                leftView->viewport()->update();
                rightView->viewport()->update();
            }
            thread->quit();
            thread->wait();
            worker->deleteLater();
            thread->deleteLater();
        });
        thread->start();
    }
}*/

/*void MainWindow::paste()
{
    leftView->setFocus();
    if (clipPaths.isEmpty()) return;

    // 目标目录
    QString dstDir;
    QModelIndexList selected = leftView->selectionModel()->selectedIndexes();
    if (!selected.isEmpty())
        dstDir = QFileInfo(fileModel->filePath(selected.first())).absolutePath();
    else
        dstDir = fileModel->filePath(leftView->rootIndex());

    // 计算所有待复制的源和目标路径，以及总大小
    QStringList srcs, dsts;
    qint64 totalSize = 0;
    for (const QString &src : clipPaths) {
        QFileInfo fi(src);
        QString dstPath = QDir(dstDir).filePath(fi.fileName());
        srcs << src;
        dsts << dstPath;
        if (fi.isDir()) {
            QDirIterator it(src, QDir::Files | QDir::NoDotAndDotDot | QDir::Dirs, QDirIterator::Subdirectories);
            while (it.hasNext()) {
                QFileInfo sfi(it.next());
                if (sfi.isFile()) totalSize += sfi.size();
            }
        } else {
            totalSize += fi.size();
        }
    }

    // 异步复制
    QThread* thread = new QThread;
    FileCopyTask* worker = new FileCopyTask(srcs, dsts, totalSize);
    worker->moveToThread(thread);

    QDialog* dialog = nullptr;
    QProgressBar* progressBar = nullptr;
    QLabel* label = nullptr;

    // 4.7GB = 4.7*1024*1024*1024
    if (totalSize > 4.7 * 1024 * 1024 * 1024) {
        // 带进度条对话框
        dialog = new QDialog(this);
        dialog->setWindowTitle("正在粘贴...");
        dialog->setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
        dialog->setWindowModality(Qt::NonModal);
        dialog->resize(500, 120);

        QVBoxLayout* layout = new QVBoxLayout(dialog);
        label = new QLabel("正在粘贴，请稍候...");
        progressBar = new QProgressBar(dialog);
        progressBar->setRange(0, 100);
        layout->addWidget(label);
        layout->addWidget(progressBar);

        dialog->show();

        connect(worker, &FileCopyTask::progress, this, [=](qint64 copied, qint64 total) {
            int percent = total > 0 ? int(100.0 * copied / total) : 0;
            progressBar->setValue(percent);
            dialog->adjustSize();
        });
        connect(worker, &FileCopyTask::currentFile, this, [=](const QString& filename) {
            label->setText(QString("正在粘贴 %1，请稍候...").arg(filename));
            dialog->setFixedHeight(170);
            dialog->adjustSize();
            if (dialog->width() > 800)
                dialog->resize(800, dialog->width());
        });
    } else if (totalSize > 1 * 1024 * 1024 * 1024) {
        // 只显示一句话的对话框
        dialog = new QDialog(this);
        dialog->setWindowTitle("正在粘贴...");
        dialog->setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
        dialog->setWindowModality(Qt::NonModal);
        dialog->resize(400, 90);

        QVBoxLayout* layout = new QVBoxLayout(dialog);
        label = new QLabel("正在粘贴，请稍候...");
        layout->addWidget(label);

        dialog->show();

        connect(worker, &FileCopyTask::currentFile, this, [=](const QString& filename) {
            label->setText(QString("正在粘贴 %1，请稍候...").arg(filename));
        });
    }

    connect(thread, &QThread::started, worker, &FileCopyTask::doCopy);
    connect(worker, &FileCopyTask::finished, this, [=](bool success, const QString &err) {
        if (dialog) dialog->close();
        if (!success) QMessageBox::warning(this, "粘贴失败", err);
        else {
            // 粘贴完成后清除剪切状态
            if (clipIsCut) {
                clipPaths.clear();
                clipIsCut = false;
            }
            updateRightView();
            leftView->viewport()->update();
            rightView->viewport()->update();
        }
        thread->quit();
        thread->wait();
        worker->deleteLater();
        thread->deleteLater();
        if (dialog) dialog->deleteLater();
    });
    thread->start();

    // 小于1GB直接异步复制，无对话框
    if (totalSize <= 1 * 1024 * 1024 * 1024) {
        connect(thread, &QThread::started, worker, &FileCopyTask::doCopy);
        connect(worker, &FileCopyTask::finished, this, [=](bool success, const QString &err) {
            if (!success) QMessageBox::warning(this, "粘贴失败", err);
            else {
                if (clipIsCut) {
                    clipPaths.clear();
                    clipIsCut = false;
                }
                updateRightView();
                leftView->viewport()->update();
                rightView->viewport()->update();
            }
            thread->quit();
            thread->wait();
            worker->deleteLater();
            thread->deleteLater();
        });
        thread->start();
    }
}*/

/*void MainWindow::addToFavorites()
{
    loadFavoritesFromFile();

    QModelIndex index = leftView->currentIndex();
    if (!index.isValid()) return;

    QString path = fileModel->filePath(index);

    // Use custom QInputDialog object
    QInputDialog inputDialog(this);
    inputDialog.setWindowTitle("Add to favorites");
    inputDialog.setLabelText("Please select collection category: ");
    inputDialog.setComboBoxItems(categories);
    inputDialog.setComboBoxEditable(false);

    // Set a larger font
    font.setFamilies(QStringList() << "Microsoft YaHei" << "Malgun Gothic" << "Segoe UI");
    font.setPointSize(11);
    inputDialog.setFont(font);
    // Adjust the initial size of the dialog box
    inputDialog.resize(400, 300);

    QComboBox* combo = inputDialog.findChild<QComboBox*>();
    if (combo)
    {
        if (isDarkMode) comboboxQssDark(combo);
        else comboboxQss(combo);
        combo->setFont(font);
    }

    if (inputDialog.exec() == QDialog::Accepted)
    {
        QString category = inputDialog.textValue();
        if (category.isEmpty()) return;
        collect(category, path);
    }
}*/


/*void MainWindow::favoritesContextMenu(const QPoint& pos)
{
    QListWidgetItem* item = favoritesList->itemAt(pos);
    if (!item) return;

    QMenu menu;
    QAction* moveAct = menu.addAction("Move to...");
    QAction* delAct  = menu.addAction("Delete");
    QAction* act = menu.exec(favoritesList->viewport()->mapToGlobal(pos));
    if (act == moveAct) moveFavorite(item);
    else if (act == delAct) deleteFavorite(item);
}*/

/*void MainWindow::moveFavorite(QListWidgetItem* item)
{
    QString srcPath = item->data(Qt::UserRole).toString();

    QInputDialog dlg(this);
    dlg.setWindowTitle("Move Collection");
    dlg.setLabelText("Please select the target favorites: ");
    dlg.setComboBoxItems(categories);
    dlg.setComboBoxEditable(false);

    QFont dlgFont;
    dlgFont.setFamilies(QStringList() << "Microsoft YaHei" << "Malgun Gothic" << "Segoe UI");
    dlgFont.setPointSize(11);
    dlg.setFont(dlgFont);

    dlg.resize(400, 300);

    int defaultIndex = categories.indexOf(fCategory);
    if (defaultIndex >= 0)
        dlg.setTextValue(categories.at(defaultIndex));

    if (dlg.exec() != QDialog::Accepted) return;

    QString dstCategory = dlg.textValue();

    favorites[fCategory].removeAll(srcPath);
    if (!favorites[dstCategory].contains(srcPath))
        favorites[dstCategory].append(srcPath);

    saveFavoritesToFile();
    reloadFavoritesList();
}

void MainWindow::deleteFavorite(QListWidgetItem* item)
{
    QString path = item->data(Qt::UserRole).toString();
    auto ret = QMessageBox::question(
        this, "Delete Collection",
        QString("Are you sure you want to delete“%2”from“%1”?").arg(fCategory, item->text()),
        QMessageBox::Yes|QMessageBox::No);

    if (ret != QMessageBox::Yes) return;

    favorites[fCategory].removeAll(path);
    saveFavoritesToFile();
    reloadFavoritesList();    // Reload and display
}*/
