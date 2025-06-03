#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QThreadPool::globalInstance()->setMaxThreadCount(2);

    this->resize(1600, 900);
    this->setWindowTitle("BTA   ----    在做重要操作前，记得备份！");

    // 中心部件
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // 文件显示
    fileModel = new MyFileSystemModelEx(this);
    fileModel->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden);
    fileModel->setRootPath(rootPath);

    // leftView设置了只有左键双击才能打开文件夹和文件
    leftView = new MyLeftView(this);
    leftView->setModel(fileModel);
    leftView->setRootIndex(fileModel->index(rootPath));                  // 默认显示的路径
    leftView->setExpandsOnDoubleClick(false);                            // 禁止双击展开
    leftView->setContextMenuPolicy(Qt::CustomContextMenu);               // 启用自定义右键菜单
    // 设置列宽策略，在最后一列右侧留出空间
    leftView->header()->setStretchLastSection(false);
    leftView->setSelectionMode(QAbstractItemView::ExtendedSelection);    // 可多选
    leftView->setItemDelegate(new SpacingDelegate(17, this));

    // 拖拽事件过滤
    leftView->setDragEnabled(true);
    leftView->setAcceptDrops(true);
    //leftView->viewport()->setAcceptDrops(true);
    //leftView->setDragDropMode(QAbstractItemView::DropOnly);
    leftView->setDropIndicatorShown(true);
    leftView->setDragDropMode(QAbstractItemView::DragDrop);
    //leftView->viewport()->installEventFilter(this);

    // rightView只用来显示leftview的上级目录以及可以展开目录
    rightView = new QTreeView(this);
    rightView->setModel(fileModel);
    rightView->setRootIndex(fileModel->index(rootPath));
    rightView->setContextMenuPolicy(Qt::CustomContextMenu);
    rightView->setItemDelegate(new SpacingDelegate(17, this));

    // 设置显示的字体
    //font.setFamily("Segoe UI");
    //font.setFamilies({"Microsoft YaHei", "Malgun Gothic"});
    font.setFamilies(QStringList() << "Microsoft YaHei" << "Malgun Gothic" << "Segoe UI");
    leftView->setFont(font);
    rightView->setFont(font);

    // 设置列宽策略，在最后一列右侧留出空间
    leftView->header()->setStretchLastSection(false);
    rightView->header()->setStretchLastSection(false);

    // 允许用户调整列宽
    leftView->header()->setSectionsMovable(true);
    rightView->header()->setSectionsMovable(true);

    // 调整文件显示
    viewAdjust();

    // 允许用户调整列宽
    leftView->header()->setSectionsMovable(true);
    rightView->header()->setSectionsMovable(true);

    // 绘制进度条
    sizeDelegate = new SizeProgressDelegate(fileModel, this);
    sizeDelegate->loadCacheFromFile("");
    connect(sizeDelegate, &SizeProgressDelegate::requestUpdate, [this]() {
        leftView->viewport()->update();
        rightView->viewport()->update();
    });
    leftView->setItemDelegateForColumn(1, sizeDelegate);
    rightView->setItemDelegateForColumn(1, sizeDelegate);

    // 保存界面配置
    savedHeaderState = leftView->header()->saveState();

    // 绑定双击打开, 单击预览
    //connect(leftView, &QTreeView::doubleClicked, this, &MainWindow::leftView_doubleClicked);
    connect(leftView, &QTreeView::doubleClicked, this, [this](const QModelIndex &idx){
        if (isSearch)
        {
            QString path = searchModel->data(idx, Qt::DisplayRole).toString();
            QDesktopServices::openUrl(QUrl::fromLocalFile(path));
        }
        else
            leftView_doubleClicked(idx);
    });

    // 前进后退按钮
    backButton = new QPushButton(this);
    forwardButton = new QPushButton(this);

    // 设置为可选中的状态
    backButton->setCheckable(true);
    forwardButton->setCheckable(true);

    backButton->setFixedHeight(buttonHeight);
    backButton->setFixedWidth(buttonHeight);
    forwardButton->setFixedHeight(buttonHeight);
    forwardButton->setFixedWidth(buttonHeight);

    backButton->setIcon(QIcon(":/left-1.png"));
    forwardButton->setIcon(QIcon(":/right-1.png"));

    // 按钮样式
    QString style = "QPushButton { background-color: transparent; border: none; }";
    backButton->setStyleSheet(style);
    forwardButton->setStyleSheet(style);

    buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(backButton);
    buttonLayout->addWidget(forwardButton);

    connect(backButton, &QPushButton::clicked, this, &MainWindow::backClicked);
    connect(forwardButton, &QPushButton::clicked, this, &MainWindow::forwardClicked);


    // 刷新按钮
    refreshButton = new QPushButton(this);
    refreshButton->setFixedHeight(buttonHeight);
    refreshButton->setFixedWidth(74);
    refreshButton->setIcon(QIcon(":/refresh.png"));
    refreshButton->setStyleSheet("background-color: transparent; border: none;");

    buttonLayout->addWidget(refreshButton);

    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::refreshView);


    // 路径按钮
    for (int i = 0; i < 7; ++i)
    {
        QToolButton* button = new QToolButton(this);

        // 按钮长度随路径长度变化
        button->setFixedWidth(defaultPathButtonWidth);
        button->setFixedHeight(buttonHeight);
        button->setToolButtonStyle(Qt::ToolButtonTextOnly);

        font = button->font();
        font.setPointSize(fontSize);
        font.setFamilies(QStringList() << "Microsoft YaHei" << "Malgun Gothic" << "Segoe UI");
        button->setFont(font);

        button->setStyleSheet(
            "QToolButton { background-color: transparent; border: none; }"
            "QToolButton:hover { background-color: rgba(200, 200, 200, 0.5); }"
        );

        connect(button, &QPushButton::clicked, this, &MainWindow::pathClicked);

        buttons.push_back(button);
        buttonLayout->addWidget(button);
    }
    // 添加一个可伸缩的空白区域
    buttonLayout->addStretch();


    // 复制路径按钮
    copyPathButton = new QPushButton(this);
    copyPathButton->setFixedHeight(buttonHeight);
    copyPathButton->setFixedWidth(buttonWidth);
    copyPathButton->setText("复制路径");

    // 设置字体
    font = copyPathButton->font();
    font.setPointSize(fontSize);
    copyPathButton->setFont(font);

    buttonQss(copyPathButton);

    buttonLayout->addWidget(copyPathButton);
    connect(copyPathButton, &QPushButton::clicked, this, &MainWindow::copyPath);


    // 是否预览
    previewButton = new QPushButton(this);
    previewButton->setFixedHeight(buttonHeight);
    previewButton->setFixedWidth(227);
    previewButton->setText("关闭预览");

    font = previewButton->font();
    font.setPointSize(fontSize);
    previewButton->setFont(font);

    // 默认关闭预览
    isPreview = false;

    buttonQss(previewButton);

    buttonLayout->addWidget(previewButton);
    connect(previewButton, &QPushButton::clicked, this, &MainWindow::preview);


    // 深色模式
    darkButton = new QPushButton();
    darkButton->setFixedHeight(buttonHeight);
    darkButton->setFixedWidth(buttonWidth);
    darkButton->setText("深色模式");

    font = darkButton->font();
    font.setPointSize(fontSize);
    darkButton->setFont(font);

    buttonQss(darkButton);

    buttonLayout->addWidget(darkButton);
    connect(darkButton, &QPushButton::clicked, this, &MainWindow::changeDark);


    // 设置收藏夹内只有路径
    fButton = new QPushButton(this);
    fButton->setFixedHeight(buttonHeight);
    fButton->setFixedWidth(buttonWidth);
    fButton->setText("仅路径");

    font = fButton->font();
    font.setPointSize(fontSize);
    fButton->setFont(font);

    buttonQss(fButton);

    // 分类收藏下拉框
    collectComboBox = new QComboBox(this);
    font = collectComboBox->font();
    font.setPointSize(fontSize);
    collectComboBox->setFont(font);

    collectComboBox->addItem("工作");
    collectComboBox->addItem("个人");
    collectComboBox->addItem("重要");
    collectComboBox->addItem("休闲");

    collectComboBox->setFixedHeight(buttonHeight);
    collectComboBox->setFixedWidth(buttonWidth + 1);
    comboboxQss(collectComboBox);

    buttonLayout->addWidget(fButton);
    buttonLayout->addWidget(collectComboBox);

    connect(fButton, &QPushButton::clicked, this, &MainWindow::onlyPath);
    connect(collectComboBox, QOverload<const QString&>::of(&QComboBox::activated), this, &MainWindow::favoritesDialog);


    // 文件排序
    sortComboBox = new QComboBox(this);
    font = sortComboBox->font();
    font.setPointSize(fontSize);
    sortComboBox->setFont(font);

    sortComboBox->addItem("名称");
    sortComboBox->addItem("大小");
    sortComboBox->addItem("创建时间");
    sortComboBox->addItem("修改时间");
    sortComboBox->addItem("类型");

    sortComboBox->insertSeparator(sortComboBox->count());
    sortComboBox->addItem("降序");
    sortComboBox->addItem("升序");
    sortComboBox->addItem("默认");

    sortComboBox->setFixedHeight(buttonHeight);
    sortComboBox->setFixedWidth(200);
    comboboxQss(sortComboBox);

    // 初始化映射表
    sortMap["名称"] = 0;
    sortMap["大小"] = 1;
    sortMap["创建时间"] = fileModel->columnCount() - 2;
    sortMap["修改时间"] = fileModel->columnCount() - 1;
    sortMap["类型"] = 2;

    currentSort = "名字";
    currentOrder = Qt::AscendingOrder;

    buttonLayout->addWidget(sortComboBox);
    connect(sortComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::fileSort);


    // 搜索
    searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("在文件夹内搜索");
    searchEdit->setFixedHeight(buttonHeight);
    searchEdit->setFixedWidth(194);

    font = searchEdit->font();
    font.setPointSize(fontSize);
    font.setFamilies(QStringList() << "Microsoft YaHei" << "Malgun Gothic" << "Segoe UI");
    searchEdit->setFont(font);

    buttonLayout->addWidget(searchEdit);
    searchEdit->setAttribute(Qt::WA_InputMethodEnabled, false);
    connect(searchEdit, &QLineEdit::returnPressed, this, &MainWindow::search);

    searchModel = new QStandardItemModel(this);
    searchModel->setHorizontalHeaderLabels({("Results")});
    connect(&searchWatcher, &QFutureWatcher<QStringList>::finished,
            this, &MainWindow::handleSearchFinished);


    // 右键菜单
    leftView->setContextMenuPolicy(Qt::CustomContextMenu);
    rightClickMenu = new RightClickMenu(this);
    categories = QStringList({"工作", "个人", "重要", "休闲"});

    // 快捷键
    this->addAction(rightClickMenu->getRefresh());
    this->addAction(rightClickMenu->getCollections());
    this->addAction(rightClickMenu->getNewDirectory());
    this->addAction(rightClickMenu->getCut());
    this->addAction(rightClickMenu->getCopy());
    this->addAction(rightClickMenu->getPaste());
    this->addAction(rightClickMenu->getRecyleBin());
    this->addAction(rightClickMenu->getDel());
    this->addAction(rightClickMenu->getRename());
    this->addAction(rightClickMenu->getProperties());

    QAction* newDirectoryShortcut = new QAction(this);
    newDirectoryShortcut->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_N));
    this->addAction(newDirectoryShortcut);

    QShortcut* pasteShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_V), this);

    QShortcut* propertiesShortcut = new QShortcut(QKeySequence(Qt::ALT + Qt::Key_Return), this);

    // 右键触发时显示菜单
    connect(leftView, &MyLeftView::filesDropped, this, &MainWindow::handleExternalDropped);
    connect(leftView, &QWidget::customContextMenuRequested, this, &MainWindow::showContextMenu);

    // 绑定
    connect(rightClickMenu, &RightClickMenu::refreshRequested, this, &MainWindow::refreshInterface);
    connect(rightClickMenu, &RightClickMenu::openFileRequested, this, &MainWindow::openFile);
    connect(rightClickMenu, &RightClickMenu::administratorRequested, this, &MainWindow::administratorOpen);

    connect(rightClickMenu, &RightClickMenu::openWithRequested, this, &MainWindow::openWith);
    connect(rightClickMenu, &RightClickMenu::compressionRequested, this, &MainWindow::Compressions);

    connect(rightClickMenu, &RightClickMenu::collectRequested, this, &MainWindow::addToFavorites);
    connect(rightClickMenu, &RightClickMenu::createNewRequested, this, &MainWindow::createNew);
    //connect(rightClickMenu, &RightClickMenu::createNewDirectoryRequested, this, &MainWindow::createDirectory);
    connect(newDirectoryShortcut, &QAction::triggered, this, &MainWindow::createDirectory);

    connect(rightClickMenu, &RightClickMenu::copyFilePathRequested, this, &MainWindow::copyFilePath);
    connect(rightClickMenu, &RightClickMenu::createShortcutRequested, this, &MainWindow::createShortCut);
    connect(rightClickMenu, &RightClickMenu::cutRequested, this, &MainWindow::cut);
    connect(rightClickMenu, &RightClickMenu::copyRequested, this, &MainWindow::copy);
    connect(rightClickMenu, &RightClickMenu::pasteRequested, this, [this]() {
        paste("");
    });
    connect(pasteShortcut, &QShortcut::activated, this, [this]() {
        paste("");
    });
    connect(rightClickMenu, &RightClickMenu::recyleBinRequested, this, [this]() {
        this->contextMenuView = leftView;
        this->contextMenuIndex = leftView->currentIndex();
        toRecyleBin();
    });
    connect(rightClickMenu, &RightClickMenu::deleteRequested, this, [this]() {
        this->contextMenuView = leftView;
        this->contextMenuIndex = leftView->currentIndex();
        del(QStringList(), true);
    });
    connect(rightClickMenu, &RightClickMenu::renameRequested, this, &MainWindow::rename);
    connect(rightClickMenu, &RightClickMenu::propertiesRequested, this, &MainWindow::properties);
    connect(propertiesShortcut, &QShortcut::activated, this, &MainWindow::properties);

    // 文件预览
    connect(leftView, &QTreeView::clicked, this, &MainWindow::previewFile);


    // 整体布局
    splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(leftView);
    splitter->addWidget(rightView);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 1);

    layout = new QVBoxLayout(centralWidget);
    layout->addLayout(buttonLayout);
    layout->addWidget(splitter);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 所有界面样式调整
void MainWindow::interfaceStyle()
{
    // 设置列宽策略，在最后一列右侧留出空间
    leftView->header()->setStretchLastSection(false);
    rightView->header()->setStretchLastSection(false);

    // leftView设置了只有左键双击才能打开文件夹和文件
    leftView = new MyLeftView(this);
    leftView->setModel(fileModel);
    leftView->setRootIndex(fileModel->index(rootPath));
    leftView->setExpandsOnDoubleClick(false);                            // 无法双击展开
    leftView->setContextMenuPolicy(Qt::CustomContextMenu);               // 启用自定义右键菜单
    leftView->setSelectionMode(QAbstractItemView::ExtendedSelection);    // 可多选
    leftView->setItemDelegate(new SpacingDelegate(17, this));

    // rightView只用来显示leftview的上级目录以及可以展开目录
    rightView = new QTreeView(this);
    rightView->setModel(fileModel);
    rightView->setRootIndex(fileModel->index(rootPath));
    rightView->setContextMenuPolicy(Qt::CustomContextMenu);
    rightView->setItemDelegate(new SpacingDelegate(17, this));

    // 设置显示的字体
    //font.setFamily("Segoe UI");
    //font.setFamilies({"Microsoft YaHei", "Malgun Gothic"});
    font.setFamilies(QStringList() << "Microsoft YaHei" << "Malgun Gothic" << "Segoe UI");
    leftView->setFont(font);
    rightView->setFont(font);

    // 允许用户调整列宽
    leftView->header()->setSectionsMovable(true);
    rightView->header()->setSectionsMovable(true);

    // 调整文件显示
    viewAdjust();

    // 显示区域某列居中对齐
    /*CenterAlignDelegate* delegate = new CenterAlignDelegate(this);
    leftView->setItemDelegateForColumn(1, delegate);
    rightView->setItemDelegateForColumn(1, delegate);*/

    // 绘制进度条
    sizeDelegate = new SizeProgressDelegate(fileModel, this);
    sizeDelegate->loadCacheFromFile("");
    connect(sizeDelegate, &SizeProgressDelegate::requestUpdate, [this]() {
        leftView->viewport()->update();
        rightView->viewport()->update();
    });
    leftView->setItemDelegateForColumn(1, sizeDelegate);
    rightView->setItemDelegateForColumn(1, sizeDelegate);
}

// 调整文件显示区域每列的宽度
void MainWindow::viewAdjust()
{
    leftView->header()->moveSection(leftView->header()->visualIndex(3), 1);
    leftView->header()->moveSection(leftView->header()->visualIndex(1), 2);
    rightView->header()->moveSection(rightView->header()->visualIndex(3), 1);
    rightView->header()->moveSection(rightView->header()->visualIndex(1), 2);

    leftView->setColumnWidth(0, 470);
    leftView->setColumnWidth(1, 270);
    leftView->setColumnWidth(2, 100);
    leftView->setColumnWidth(3, 300);
    rightView->setColumnWidth(0, 470);
    rightView->setColumnWidth(1, 270);
    rightView->setColumnWidth(2, 100);
    rightView->setColumnWidth(3, 300);
}

// 拖拽事件过滤
bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == leftView->viewport())
    {
        if (event->type() == QEvent::DragEnter)
        {
            QDragEnterEvent* dragEvent = static_cast<QDragEnterEvent*>(event);

            // 只有leftPath非空且是文件拖拽才允许
            if (!leftPath.isEmpty() && dragEvent->mimeData()->hasUrls())
                dragEvent->acceptProposedAction();
            else
                dragEvent->ignore();
            return true;
        }
        if (event->type() == QEvent::DragMove)
        {
            QDragMoveEvent* dragEvent = static_cast<QDragMoveEvent*>(event);
            if (!leftPath.isEmpty() && dragEvent->mimeData()->hasUrls())
                dragEvent->acceptProposedAction();
            else
                dragEvent->ignore();
            return true;
        }
        if (event->type() == QEvent::Drop)
        {
            QDropEvent* dropEvent = static_cast<QDropEvent*>(event);
            if (!leftPath.isEmpty() && dropEvent->mimeData()->hasUrls())
            {
                QStringList paths;
                for (const QUrl& url : dropEvent->mimeData()->urls())
                {
                    if (url.isLocalFile()) paths << url.toLocalFile();
                }

                QModelIndex idx = leftView->indexAt(dropEvent->pos());
                QString dstDir;
                if (idx.isValid() && fileModel->isDir(idx))
                    dstDir = fileModel->filePath(idx);
                else
                    dstDir = fileModel->filePath(leftView->rootIndex());

                clipPaths = paths;
                clipIsCut = false;
                rightClickMenu->isCopy = true;
                paste(dstDir);

                dropEvent->acceptProposedAction();
            }
            else dropEvent->ignore();
            return true;
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

// 拖拽粘贴
void MainWindow::handleExternalDropped(const QStringList& paths, const QModelIndex& targetIndex)
{
    if (paths.isEmpty()) return;

    QWidget* p = (contextMenuView == fileView && favoritesDialogPtr)
                 ? static_cast<QWidget*>(favoritesDialogPtr)
                 : this;

    QAbstractItemView* senderView = qobject_cast<QAbstractItemView*>(sender());
    if (!senderView) return;

    QString dstDir;
    if (targetIndex.isValid() && fileModel->isDir(targetIndex))
        dstDir = fileModel->filePath(targetIndex);
    else if (senderView == leftView)
        dstDir = fileModel->filePath(leftView->rootIndex());
    else if (senderView == fileView)
        dstDir = fileModel->filePath(fileView->rootIndex());
    else
        dstDir = QString();

    if (dstDir.isEmpty() || !QDir(dstDir).exists())
    {
        QMessageBox::warning(p, "Unable to paste", "No valid directory");
        return;
    }

    contextMenuView = senderView;
    contextMenuIndex = targetIndex;

    clipPaths = paths;
    clipIsCut = false;
    rightClickMenu->isCopy = true;

    paste(dstDir);
}

// 更新存储路径的栈
void MainWindow::updateStack(const QString& path, QStack<QString>& stack)
{
    QStringList seg = path.split("/", QString::SkipEmptyParts);

    if (stack.isEmpty())
    {
        stack.push(seg[0]);
        for (int i = 1; i < seg.size(); ++i)
        {
            stack.push("/" + seg[i]);
        }
    }
    else
    {
        int ps = stack.size();
        int ss = seg.size();

        // 继续增加路径
        if (ss > ps)
        {
            for (int i = ps; i < ss; ++i)
            {
                stack.push("/" + seg[i]);
            }
        }
        // 处理回退
        // 如果回退到根目录, 栈中有""，长度为1
        // 点击进入一个磁盘后, seg长度为1, 要处理相同的情况
        else
        {
            if (!stack.isEmpty())
            {
                stack.pop();
                if (ss == ps) stack.push(seg[0]);
                else if(stack.isEmpty()) stack.push("");
            }
        }
    }
}

// 更新路径按钮
void MainWindow::updateButtons(QVector<QToolButton*>& bs, QStack<QString>& stack)
{
    if (stack.size() == 1 && stack.top().isEmpty())
    {
        bs[0]->setText("");
        return ;
    }

    QList<QString> segments = stack.toList();
    int counts = bs.size();
    int sz = segments.size();

    for (int i = 0; i < counts; ++i)
    {
        bs[i]->setText("");
    }

    QFontMetrics metrics(bs[0]->font());

    int i = 0;
    while (i < sz && i < counts)
    {
        QString seg = segments.at(i);
        if (seg.startsWith("/")) seg = seg.mid(1);

        // 计算当前路径长度并实时调整按钮宽度
        int currentWidth = metrics.boundingRect(seg).width();
        int buttonWidth = defaultPathButtonWidth;

        if (currentWidth < defaultPathButtonWidth && defaultPathButtonWidth - currentWidth < moreWidth)
            buttonWidth = defaultPathButtonWidth + moreWidth;

        else if (currentWidth >= defaultPathButtonWidth && currentWidth < maxPathButtonWidth - moreWidth)
            buttonWidth = currentWidth + moreWidth;

        else if (currentWidth >= maxPathButtonWidth - moreWidth)
            buttonWidth = maxPathButtonWidth;

        bs[i]->setFixedWidth(buttonWidth);
        QString text = metrics.elidedText(seg, Qt::ElideRight, buttonWidth);

        bs[i]->setText(text);
        ++i;
    }
}

// 更新右边区域
void MainWindow::updateRightView()
{
    QModelIndex leftRoot = leftView->rootIndex();
    leftPath = fileModel->filePath(leftRoot);
    QDir dir(leftPath);

    if (dir.cdUp()) rightPath = dir.absolutePath();
    else rightPath = rootPath;

    rightView->setRootIndex(fileModel->index(rightPath));
}

// 双击
void MainWindow::leftView_doubleClicked(const QModelIndex &index)
{
    backFlag = false;

    filePath = fileModel->filePath(index);
    QFileInfo fileInfo(filePath);

    if (fileInfo.isDir())
    {
        leftPath = filePath;
        leftView->setRootIndex(fileModel->index(leftPath));
        leftView->setFocus();

        /*if (fileModel->rootPath().isEmpty() && fileInfo.dir().absolutePath() != filePath)
        {
            QString parentDir = fileInfo.dir().absolutePath();
            rightView->setRootIndex(fileModel->index(parentDir));
        }*/

        QString parentDir = fileInfo.dir().absolutePath();
        if (parentDir != filePath) rightView->setRootIndex(fileModel->index(parentDir));

        updateStack(filePath, pathStack);
        updateButtons(buttons, pathStack);
    }
    else if (fileInfo.isFile())
    {
        QString ext = fileInfo.suffix().toLower();
        static const QSet<QString> alwaysNormal = {"png","jpg","jpeg","bmp","gif","tiff","webp","txt"};

        if (openAsAdmin && !alwaysNormal.contains(ext))
        {
            bool success = false;
#ifdef Q_OS_WIN
            // 获取文件关联程序
            QString appPath = getAssociatedSW(filePath);
            if (!appPath.isEmpty())
            {
                QString param = QString("\"%1\"").arg(filePath);
                success = runAsAdmin(appPath, param);
            }
#elif defined(Q_OS_LINUX)
            QString cmd = QString("xdg-open \"%1\"").arg(filePath);
            success = (system(QString("pkexec bash -c '%1'").arg(cmd).toUtf8()) == 0);
#endif
            if (!success) QMessageBox::warning(this, "Error", "Unable to open the file with administrator privileges");
        }
        else QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
    }
}

// 后退按钮
void MainWindow::backClicked()
{
    // 从搜索界面回到主程序
    if (isSearch)
    {
        leftView->setModel(fileModel);
        leftView->setRootIndex(fileModel->index(leftPath));
        leftView->header()->restoreState(savedHeaderState);
        leftView->setItemDelegate(nullptr);
        leftView->setItemDelegate(new SpacingDelegate(17, this));

        isSearch = false;
        searchEdit->clear();
        return ;
    }

    backFlag = true;

    //forwardButton->setChecked(true);
    //QTimer::singleShot(47, [this]() { forwardButton->setChecked(false); });

    QModelIndex currentRoot = leftView->rootIndex();
    QString currentPath = fileModel->filePath(currentRoot);
    QDir dir(currentPath);

    if (currentPath == "") return ;
    else if (dir.cdUp())
    {
        QString newPath = dir.absolutePath();
        leftView->setRootIndex(fileModel->index(newPath));
        leftPath = newPath;
        updateRightView();
    }
    else
    {
        leftPath = rootPath;
        leftView->setRootIndex(fileModel->index(leftPath));
    }
    updateStack(leftPath, pathStack);
    updateButtons(buttons, pathStack);
}

// 前进按钮
void MainWindow::forwardClicked()
{
    if (backFlag)
    {
        QStringList fileSeg = filePath.split("/", QString::SkipEmptyParts);
        QStringList leftSeg = leftPath.split("/", QString::SkipEmptyParts);
        int sz = leftSeg.size();

        if (fileSeg.size() > sz)
        {
            QString oldLeftPath = leftPath;

            QString nextSegment = fileSeg[sz];
            QString newLeftPath;

            if (oldLeftPath.isEmpty()) newLeftPath = nextSegment;
            else
            {
                if (oldLeftPath.endsWith("/"))
                    newLeftPath = oldLeftPath + nextSegment;
                else
                    newLeftPath = oldLeftPath + "/" + nextSegment;
            }

            leftPath = newLeftPath;
            rightPath = oldLeftPath;

            leftView->setRootIndex(fileModel->index(leftPath));
            rightView->setRootIndex(fileModel->index(rightPath));
            updateStack(leftPath, pathStack);
        }
        updateButtons(buttons, pathStack);
    }
}

// 刷新
void MainWindow::refreshView()
{
    if (!fileModel) return;

    if (contextMenuView == leftView || contextMenuView == rightView)
    {
        QString leftRootPath = fileModel->filePath(leftView->rootIndex());
        QString rightRootPath = fileModel->filePath(rightView->rootIndex());

        // 清理缓存
        if (sizeDelegate)
        {
            QHash<QString, qint64>& cache = const_cast<QHash<QString, qint64>&>(sizeDelegate->getSizeCache());
            QList<QString> keys = cache.keys();
            for (const QString& key : keys)
            {
                if (key.startsWith(leftRootPath) || key.startsWith(rightRootPath))
                    cache.remove(key);
            }
            QSet<QString>& pending = const_cast<QSet<QString>&>(sizeDelegate->getPendingPaths());
            QList<QString> pendingList = pending.values();
            for (const QString& key : pendingList)
            {
                if (key.startsWith(leftRootPath) || key.startsWith(rightRootPath))
                    pending.remove(key);
            }
        }

        // 刷新主程序视图
        leftView->setRootIndex(fileModel->index(leftRootPath));
        leftView->viewport()->update();

        updateRightView();
        rightView->viewport()->update();
    }
    else if (contextMenuView == fileView && fileView)
    {
        QString fileRootPath = fileModel->filePath(fileView->rootIndex());

        // 清理缓存
        if (sizeDelegate)
        {
            QHash<QString, qint64>& cache = const_cast<QHash<QString, qint64>&>(sizeDelegate->getSizeCache());
            QList<QString> keys = cache.keys();
            for (const QString& key : keys)
            {
                if (key.startsWith(fileRootPath))
                    cache.remove(key);
            }
            QSet<QString>& pending = const_cast<QSet<QString>&>(sizeDelegate->getPendingPaths());
            QList<QString> pendingList = pending.values();
            for (const QString& key : pendingList)
            {
                if (key.startsWith(fileRootPath))
                    pending.remove(key);
            }
        }

        // 刷新收藏夹视图
        fileView->setRootIndex(fileModel->index(fileRootPath));
        fileView->viewport()->update();
    }
}

// 点击路径按钮
void MainWindow::pathClicked()
{
    QToolButton* button = qobject_cast<QToolButton*>(sender());
    if (!button) return;

    int index = -1;
    for (int i = 0; i < buttons.size(); ++i)
    {
        if (buttons[i] == button)
        {
            index = i;
            break;
        }
    }

    if (index == -1 || button->text().isEmpty()) return;

    while (pathStack.size() > index + 1)
    {
        pathStack.pop();
    }

    QString path;
    QList<QString> seg = pathStack.toList();
    for (const QString& s : seg)
    {
        path += s;
    }

    if (isSearch)
    {
        leftView->setModel(fileModel);
        leftView->header()->restoreState(savedHeaderState);
        leftView->setItemDelegate(nullptr);
        leftView->setItemDelegate(new SpacingDelegate(17, this));

        isSearch = false;
        searchEdit->clear();
    }

    leftView->setRootIndex(fileModel->index(path));
    leftPath = path;

    QDir dir(leftPath);
    if (dir.cdUp()) rightPath = dir.absolutePath();
    else rightPath = rootPath;

    rightView->setRootIndex(fileModel->index(rightPath));

    backFlag = true;

    updateButtons(buttons, pathStack);
}

// 复制当前路径
void MainWindow::copyPath()
{
    QString currentPath;
    QList<QString> segments = pathStack.toList();

    for (const QString& seg : segments)
    {
        currentPath += seg;
    }

    if (!currentPath.isEmpty())
    {
        clipboard = QApplication::clipboard();
        clipboard->setText(currentPath);
        QToolTip::showText(QCursor::pos(), "已复制", this);
    }
}

// 是否开启预览
void MainWindow::preview()
{
    if (isPreview)
    {
        isPreview = false;
        previewButton->setText("关闭预览");
    }
    else
    {
        isPreview = true;
        previewButton->setText("预览");
    }
}


// 深色模式
void MainWindow::changeDark()
{
    isDarkMode = !isDarkMode;

    if(isDarkMode)
    {
        QString eyeStyle = R"(
            QWidget {
                background-color: #2e3b3f;    /* 深绿灰色 */
                color: #d0e0dc;               /* 柔和的浅绿色，带有白色文字 */
            }

            QPushButton {
                background-color: #3c4b4f;
                color: #d0e0dc;
                border: 1px solid #5a6e6a;
                border-radius: 4px;
                padding: 5px;
            }

            QPushButton:hover {
                background-color: #4c5f63;
            }

            QLineEdit, QTextEdit {
                background-color: #394947;
                color: #e0f0ec;
                border: 1px solid #60706c;
            }

            QMenuBar, QMenu {
                background-color: #2f3d3f;
                color: #d0e0dc;
            }

            QMenu::item:selected {
                background-color: #4c5f63;
            }

            QStatusBar {
                background-color: #2f3d3f;
                color: #a0b0ac;
            }

            QHeaderView::section {
                background-color: #3a4b4f;
                color: #d0e0dc;
                padding: 4px;
            }

            QToolTip {
                background-color: #3f5054;
                color: #d0e0dc;
                border: 1px solid #6a807c;
            }

            QScrollBar:vertical, QScrollBar:horizontal {
                background-color: #2e3b3f;
                width: 10px;
            }

            QScrollBar::handle:vertical, QScrollBar::handle:horizontal {
                background-color: #5a6e6a;
                border-radius: 4px;
            }

            QScrollBar::add-line, QScrollBar::sub-line {
                background: none;
            }

            QDialogButtonBox QPushButton {
                min-width: 80px;
                min-height: 28px;
                padding: 6px 16px;
                font-size: 11pt;
                border-radius: 4px;
                background-color: #3c4b4f;
                color: #d0e0dc;
                border: 1px solid #5a6e6a;
            }

            QDialogButtonBox QPushButton:hover {
                background-color: #4c5f63;
            }
        )";

        qApp->setStyleSheet(eyeStyle);
        backButton->setIcon(QIcon(":/left-2.png"));
        forwardButton->setIcon(QIcon(":/right-2.png"));
        darkButton->setText("正常模式");

        buttonQssDark(copyPathButton);
        buttonQssDark(previewButton);
        buttonQssDark(darkButton);
        buttonQssDark(fButton);

        comboboxQssDark(collectComboBox);
        comboboxQssDark(sortComboBox);
    }
    else
    {
        // Restore default style
        qApp->setStyleSheet("");
        backButton->setIcon(QIcon(":/left-1.png"));
        forwardButton->setIcon(QIcon(":/right-1.png"));
        darkButton->setText("深色模式");

        buttonQss(copyPathButton);
        buttonQss(previewButton);
        buttonQss(darkButton);
        buttonQss(fButton);

        comboboxQss(collectComboBox);
        comboboxQss(sortComboBox);
    }
}

// 打开收藏夹
void MainWindow::onlyPath()
{
    if (isOnlyPath) isOnlyPath = false;
    else isOnlyPath = true;
}

void MainWindow::favoritesView()
{
    favoritesList->setSpacing(5);

    fileView->header()->setSectionsMovable(true);

    fileView->setColumnWidth(0, 470);
    fileView->setColumnWidth(1, 200);
    fileView->setColumnWidth(2, 270);
    fileView->setColumnWidth(3, 300);

    LeftAlignDelegate* leftDelegate = new LeftAlignDelegate(fileView);
    fileView->setItemDelegateForColumn(1, leftDelegate);
    fileView->setItemDelegate(new SpacingDelegate(17, this));

    fileView->header()->moveSection(fileView->header()->visualIndex(3), 1);
    fileView->header()->moveSection(fileView->header()->visualIndex(1), 2);

    fileView->header()->setStretchLastSection(true);
}

void MainWindow::reloadFavoritesList()
{
    favoritesList->clear();
    QStringList favPaths = favorites.value(fCategory);
    for (const QString& path : favPaths)
    {
        QFileInfo info(path);
        QString disp = info.fileName();
        if (disp.isEmpty())
        {
            if (path.length() >= 2 && path[1] == QChar(':')) disp = path.left(1);
            else disp = path;
        }

        QListWidgetItem* item = new QListWidgetItem(disp);
        item->setData(Qt::UserRole, path);
        item->setIcon(QIcon(info.isDir()? ":/folder.png" : ":/file.png"));
        favoritesList->addItem(item);
    }
}

void MainWindow::copyFavoritePath(const QList<QListWidgetItem*>& selectedItems)
{
    if (selectedItems.isEmpty()) return;

    QStringList paths;
    for (QListWidgetItem* item : selectedItems)
    {
        QString path = item->data(Qt::UserRole).toString();
        if (!path.isEmpty()) paths << path;
    }

    if (!paths.isEmpty())
    {
        QString allPaths = paths.join("\n");
        QClipboard* clipboard = QApplication::clipboard();
        clipboard->setText(allPaths);

        QString message = (paths.size() == 1) ?
            QString("已复制路径 : %1").arg(paths.first()) :
            QString("已复制 %1 个文件路径").arg(paths.size());

        QToolTip::showText(QCursor::pos(), message, favoritesList, QRect(), 2000);
    }
}

void MainWindow::moveFavorite(const QList<QListWidgetItem*>& items)
{
    if (items.isEmpty()) return;

    QInputDialog dlg(favoritesDialogPtr);
    dlg.setWindowTitle("移动收藏");
    dlg.setLabelText("请选择目标收藏夹 : ");
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

    for (QListWidgetItem* item : items)
    {
        QString srcPath = item->data(Qt::UserRole).toString();
        favorites[fCategory].removeAll(srcPath);
        if (!favorites[dstCategory].contains(srcPath))
            favorites[dstCategory].append(srcPath);
    }

    saveFavoritesToFile();
    reloadFavoritesList();
}

void MainWindow::deleteFavorite(const QList<QListWidgetItem*>& items)
{
    if (items.isEmpty()) return;

    QStringList names;
    for (QListWidgetItem* item : items)
    {
        names << item->text();
    }

    auto ret = QMessageBox::question(
        favoritesDialogPtr,
        "删除收藏",
        QString("您确定要从 “%1” 删除 \n %2 吗?")
            .arg(fCategory, names.join(", ")),
        QMessageBox::Yes|QMessageBox::No);

    if (ret != QMessageBox::Yes) return;

    for (QListWidgetItem* item : items)
    {
        QString path = item->data(Qt::UserRole).toString();
        favorites[fCategory].removeAll(path);
    }
    saveFavoritesToFile();
    reloadFavoritesList();
}

void MainWindow::moveFavorite(QListWidgetItem* item)
{
    moveFavorite(QList<QListWidgetItem*>{item});
}

void MainWindow::deleteFavorite(QListWidgetItem* item)
{
    deleteFavorite(QList<QListWidgetItem*>{item});
}

void MainWindow::favoritesContextMenu(const QPoint& pos)
{
    QList<QListWidgetItem*> selectedItems = favoritesList->selectedItems();
    if (selectedItems.isEmpty()) return;

    QMenu menu;
    QAction* moveAct = menu.addAction("移动");
    QAction* delAct  = menu.addAction("删除");
    QAction* copyPathAct = menu.addAction("复制路径");
    QAction* act = menu.exec(favoritesList->viewport()->mapToGlobal(pos));
    if (act == moveAct) moveFavorite(selectedItems);
    else if (act == delAct) deleteFavorite(selectedItems);
    else if (act == copyPathAct) copyFavoritePath(selectedItems);
}

void MainWindow::favoriteItems(QListWidgetItem* item)
{
    initialPath = item->data(Qt::UserRole).toString();
    QFileInfo info(initialPath);

    upButton->removeEventFilter(buttonFilter);

    if (info.isDir())
    {
        fIdx = fileModel->index(initialPath);
        fileView->setRootIndex(fIdx);
        fStack->setCurrentWidget(fileView);

        upButton->show();
        downButton->show();
        fLabel->hide();

        mainPath = initialPath;
        fPath = initialPath;

        for (QToolButton* b : fButtons)
        {
            b->setEnabled(true);
        }

        updateStack(fPath, fPathStack);
        updateButtons(fButtons, fPathStack);
    }
    else QDesktopServices::openUrl(QUrl::fromLocalFile(initialPath));
}

void MainWindow::upClicked()
{
    upFlag = true;

    if (isFSearch)
    {
        reloadFavoritesList();
        fPathStack.clear();

        fileView->setModel(fileModel);

        QModelIndex idx = fileModel->index(fPath);
        if (!idx.isValid() && !initialPath.isEmpty())
            idx = fileModel->index(initialPath);
        fileView->setRootIndex(idx);

        fileView->setItemDelegate(nullptr);
        fileView->setItemDelegate(new SpacingDelegate(17, this));

        for (QToolButton* b : fButtons)
        {
            b->setText("");
        }

        fSearchEdit->clear();
        isFSearch = false;

        updateStack(fPath, fPathStack);
        updateButtons(fButtons, fPathStack);
        reloadFavoritesList();
        return ;
    }

    QDir dir(fPath);

    // 判断当前路径是否包含在initialPath中
    QString normInit = QDir::cleanPath(initialPath);
    QString normCur = QDir::cleanPath(fPath);

    // 分割为目录层级
    QStringList initParts = normInit.split('/', Qt::SkipEmptyParts);
    QStringList curParts = normCur.split('/', Qt::SkipEmptyParts);

    // 如果当前路径不是 initialPath 的子目录，或层级更浅，则回到收藏列表
    bool flag = (!normCur.startsWith(normInit) || curParts.size() < initParts.size());

    if (fPath == initialPath || flag)
    {
        fPath = "";
        fLabel->setText(QString("收藏夹 : %1").arg(fCategory));
        fLabel->show();

        upButton->installEventFilter(buttonFilter);
        fStack->setCurrentWidget(favoritesList);

        for (QToolButton* b : fButtons)
        {
            b->setText("");
            b->setEnabled(false);
        }
        favoritesList->clearFocus();
        fPathStack.clear();
    }
    else if (fPath != initialPath)
    {
        dir.cdUp();
        fPath = dir.absolutePath();
        fIdx = fileModel->index(fPath);
        fileView->setRootIndex(fIdx);

        updateStack(fPath, fPathStack);
        updateButtons(fButtons, fPathStack);
    }

    reloadFavoritesList();
}

void MainWindow::downClicked()
{
    if (upFlag)
    {
        upButton->removeEventFilter(buttonFilter);

        QStringList mainSeg = mainPath.split("/", QString::SkipEmptyParts);
        QStringList fSeg = fPath.split("/", QString::SkipEmptyParts);
        int sz = fSeg.size();

        if (fPath == "")
        {
            upButton->show();
            fLabel->hide();

            fPath = mainSeg[0] + "/" + mainSeg[1];
            fIdx = fileModel->index(fPath);
            fileView->setRootIndex(fIdx);
            fStack->setCurrentWidget(fileView);

            for (QToolButton* b : fButtons)
            {
                b->setEnabled(true);
            }
        }
        else if (fPath == mainPath) return ;
        else if (mainSeg.size() > sz)
        {
            QString oldPath = fPath;
            QString nextSegment = mainSeg[sz];
            QString newFPath;

            if (oldPath.isEmpty()) newFPath = nextSegment;
            else
            {
                if(oldPath.endsWith("/"))
                    newFPath = oldPath + nextSegment;
                else
                    newFPath = oldPath + "/" + nextSegment;
            }

            fPath = newFPath;
            fIdx = fileModel->index(fPath);
            fileView->setRootIndex(fIdx);
        }

        updateStack(fPath, fPathStack);
        updateButtons(fButtons, fPathStack);
    }
}

void MainWindow::fileView_doubleClicked(const QModelIndex& index)
{
    QString path;
    if (isFSearch) path = index.data(Qt::UserRole).toString();
    else path = fileModel->filePath(index);

    QFileInfo info(path);

    if (isFSearch)
    {
#ifdef Q_OS_WIN
        if (info.isDir())
        {
            // 用资源管理器打开文件夹
            QProcess::startDetached("explorer.exe", QStringList() << QDir::toNativeSeparators(path));
        }
        else if (info.isFile())
        {
            // 弹出“打开方式”对话框
            QString command = QString("rundll32.exe shell32.dll,OpenAs_RunDLL %1").arg(QDir::toNativeSeparators(path));
            QProcess::startDetached(command);
        }
#else
        // Other platforms
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
#endif
    }
    else
    {
        if (info.isDir())
        {
            mainPath = path;
            fPath = path;
            fIdx = fileModel->index(fPath);
            fileView->setRootIndex(fIdx);
            fileView->setFocus();

            updateStack(fPath, fPathStack);
            updateButtons(fButtons, fPathStack);
        }
        else if (info.isFile())
        {
            QString ext = info.suffix().toLower();
            static const QSet<QString> alwaysNormal = {"png","jpg","jpeg","bmp","gif","tiff","webp","txt"};

            if (openAsAdmin && !alwaysNormal.contains(ext))
            {
                bool success = false;
#ifdef Q_OS_WIN
                // 获取文件关联程序
                QString appPath = getAssociatedSW(path);
                if (!appPath.isEmpty())
                {
                    QString param = QString("\"%1\"").arg(path);
                    success = runAsAdmin(appPath, param);
                }
#elif defined(Q_OS_LINUX)
                QString cmd = QString("xdg-open \"%1\"").arg(path);
                success = (system(QString("pkexec bash -c '%1'").arg(cmd).toUtf8()) == 0);
#endif
                if (!success)
                    QMessageBox::warning(this, "错误", "无法以管理员权限打开文件");
            }
            else
            {
                QDesktopServices::openUrl(QUrl::fromLocalFile(path));
            }
        }
    }
}

void MainWindow::fPathClicked()
{
    QToolButton* button = qobject_cast<QToolButton*>(sender());
    if (!button) return;

    int index = -1;
    for (int i = 0; i < fButtons.size(); ++i)
    {
        if (fButtons[i] == button)
        {
            index = i;
            break;
        }
    }

    if (index == -1 || button->text().isEmpty()) return;

    while (fPathStack.size() > index + 1)
    {
        fPathStack.pop();
    }

    QString path;
    QList<QString> seg = fPathStack.toList();
    for (const QString& s : seg)
    {
        path += s;
    }

    if (isFSearch)
    {
        fileView->setModel(fileModel);
        fileView->setItemDelegate(nullptr);
        fileView->setItemDelegate(new SpacingDelegate(17, this));

        isFSearch = false;
        fSearchEdit->clear();
    }

    fileView->setRootIndex(fileModel->index(path));
    mainPath = path;
    fPath = path;

    updateButtons(fButtons, fPathStack);
}

void MainWindow::fSearch()
{
    isFSearch = true;

    const QString key = fSearchEdit->text().trimmed();

    // 如果关键字为空，则恢复原始列表/视图
    if (key.isEmpty())
    {
        isFSearch = false;

        // 如果当前在收藏夹列表中，则重新加载所有收藏夹
        if (fStack->currentWidget() == favoritesList)
        {
            reloadFavoritesList();
            fPathStack.clear();

            for (QToolButton* b : fButtons)
            {
                b->setText("");
                b->setEnabled(false);
            }
        }
        // 如果在文件视图中，请复用主程序搜索来清除
        else
        {
            fileView->setModel(fileModel);
            QModelIndex idx = fileModel->index(fPath);
            if (idx.isValid()) fileView->setRootIndex(idx);
            //restoreBrowserView();
        }
        return;
    }

    // 如果仍在运行，取消
    if (fSearchWatcher.isRunning())
    {
        fSearchWatcher.cancel();
        fSearchWatcher.waitForFinished();
    }

    // 区分收藏夹列表和文件视图
    if (fStack->currentWidget() == favoritesList)
    {
        // 获取当前类别的所有路径
        QStringList all = favorites.value(fCategory);

        // 异步过滤，保留章节名包含关键词
        auto future = QtConcurrent::run([all, key]()
        {
            QStringList matched;
            for (const QString& path : all)
            {
                QFileInfo fi(path);
                QString name = fi.fileName();
                if (name.contains(key, Qt::CaseInsensitive)) matched << path;
            }
            return matched;
        });
        fSearchWatcher.setFuture(future);
    }
    else
    {
        auto future = QtConcurrent::run([path = fPath, key]() {
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

        fSearchWatcher.setFuture(future);
    }
}

void MainWindow::handleFSearchFinished()
{
    QStringList results = fSearchWatcher.result();

    if (fStack->currentWidget() == favoritesList)
    {
        // 收藏夹列表下, 刷新favorites list
        favoritesList->clear();
        for (const QString& path : results)
        {
            QFileInfo fi(path);
            QListWidgetItem* item = new QListWidgetItem(fi.fileName());
            item->setData(Qt::UserRole, path);
            item->setIcon(QIcon(fi.isDir() ? ":/folder.png" : ":/file.png"));
            item->setToolTip(path);
            favoritesList->addItem(item);
        }
    }
    else
    {
        // 文件视图下，刷新file view
        fSearchModel->clear();
        fSearchModel->setHorizontalHeaderLabels({("Results")});
        for (const QString& path : results)
        {
            QFileInfo fi(path);
            QStandardItem* item = new QStandardItem;
            item->setText(path);
            item->setData(path, Qt::UserRole);
            item->setIcon(fi.isDir() ? QIcon(":/folder.png") : QIcon(":/file.png"));
            item->setToolTip(path);
            fSearchModel->appendRow(item);
        }
        fileView->setModel(fSearchModel);
        fileView->setItemDelegate(new SpacingDelegate(17, this));

        // 自适应长度
        if (fileView->header())
            fileView->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    }
    isFSearch = true;
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (favoritesDialogPtr) favoritesDialogPtr->close();
    QMainWindow::closeEvent(event);
}

void MainWindow::favoritesDialog(const QString& category)
{
    loadFavoritesFromFile();
    fPathStack.clear();

    fDialog = new QDialog(nullptr);
    fDialog ->setWindowTitle(QString("收藏夹 - %1").arg(fCategory));
    fDialog ->resize(1600, 900);
    fDialog ->setMinimumSize(1024, 768);

    // 能够在收藏夹对话框和主程序之间拖放文件
    //fDialog->setWindowFlags(fDialog->windowFlags() | Qt::WindowStaysOnTopHint);

    favoritesDialogPtr = fDialog;

    Qt::WindowFlags flags = fDialog ->windowFlags();
    flags |= Qt::WindowMaximizeButtonHint | Qt::WindowMinimizeButtonHint | Qt::WindowSystemMenuHint;
    fDialog ->setWindowFlags(flags);

    fDialog ->setWindowModality(Qt::NonModal);

    fCategory = category;

    funcLayout = new QHBoxLayout();
    mainLayout = new QVBoxLayout(fDialog);

    auto* clickFilter = new RightClickDoubleClickFilter(this);

    // 用于显示当前路径的标签
    fLabel = new QLabel(QString("收藏夹列表 : %1").arg(fCategory));
    fdFont = fLabel->font();
    fdFont.setPointSize(11);
    fdFont.setFamilies(QStringList() << "Microsoft YaHei" << "Malgun Gothic" << "Segoe UI");
    fLabel->setFont(fdFont);

    // 上级下级按钮, 默认隐藏
    upButton = new QPushButton();
    downButton = new QPushButton();

    if (!isOnlyPath)
    {
        upButton->setFont(fdFont);
        upButton->hide();
        downButton->setFont(fdFont);
        downButton->hide();

        QString style = "QPushButton { background-color: transparent; border: none; }";
        upButton->setStyleSheet(style);
        downButton->setStyleSheet(style);

        upButton->setFixedHeight(30);
        upButton->setFixedWidth(30);
        downButton->setFixedHeight(30);
        downButton->setFixedWidth(47);

        upButton->setIcon(QIcon(":/left-1.png"));
        downButton->setIcon(QIcon(":/right-1.png"));

        upButton->setAutoDefault(false);
        upButton->setDefault(false);

        downButton->setAutoDefault(false);
        downButton->setDefault(false);
    }

    // 使用堆栈Widget管理两种视图：收藏列表和文件视图
    fStack = new QStackedWidget;

    // 收藏列表（QListWidget）
    favoritesList = new QListWidget;
    favoritesList->setFont(fdFont);
    favoritesList->setMouseTracking(true);    // 启用鼠标追踪

    QStringList favPaths = favorites.value(fCategory);
    for (const QString& path : favPaths)
    {
        QFileInfo info(path);
        QString disp = info.fileName();
        if (disp.isEmpty())
        {
            if (path.length() >= 2 && path[1] == QChar(':')) disp = path.left(1);
            else disp = path;
        }

        QListWidgetItem* item = new QListWidgetItem(disp);
        item->setData(Qt::UserRole, path);
        if (info.isDir())
            item->setIcon(QIcon(":/folder.png"));
        else
            item->setIcon(QIcon(":/file.png"));
        favoritesList->addItem(item);
    }
    fStack->addWidget(favoritesList);
    favoritesList->setSelectionMode(QAbstractItemView::ExtendedSelection);

    // 右键菜单
    favoritesList->viewport()->installEventFilter(clickFilter);
    favoritesList->setContextMenuPolicy(Qt::CustomContextMenu);

    // 文件视图（QTreeView）
    fileView = new MyFileView;
    fileView->setModel(fileModel);
    fileView->setFont(fdFont);
    fStack->addWidget(fileView);
    fileView->setContextMenuPolicy(Qt::CustomContextMenu);

    // 调整显示样式
    favoritesView();

    // 能拖拽
    fileView->setDragEnabled(true);
    fileView->setAcceptDrops(true);
    fileView->setDropIndicatorShown(true);
    fileView->setDragDropMode(QAbstractItemView::DragDrop);

    if (!isOnlyPath)
    {
        funcLayout->addWidget(upButton);
        funcLayout->addWidget(downButton);
    }
    funcLayout->addWidget(fLabel);

    // 进度条
    fSizeDelegate = new SizeProgressDelegate(fileModel, fDialog);
    fSizeDelegate->loadCacheFromFile("");
    connect(fSizeDelegate, &SizeProgressDelegate::requestUpdate, [this]() {
        fileView->viewport()->update();
    });
    fileView->setItemDelegateForColumn(1,fSizeDelegate);

    fileView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    fileView->header()->setStretchLastSection(false);
    fileView->header()->setSectionsMovable(true);

    // 路径按钮
    for (QToolButton* b : fButtons)
    {
        funcLayout->removeWidget(b);
        b->deleteLater();
    }
    fButtons.clear();

    for (int i = 0; i < 11; ++i)
    {
        QToolButton* button = new QToolButton(fDialog);

        button->setFixedWidth(defaultPathButtonWidth);
        button->setFixedHeight(buttonHeight);
        button->setToolButtonStyle(Qt::ToolButtonTextOnly);

        fdFont = button->font();
        fdFont.setPointSize(11);
        fdFont.setFamilies(QStringList() << "Microsoft YaHei" << "Malgun Gothic" << "Segoe UI");
        button->setFont(fdFont);

        button->setStyleSheet(
            "QToolButton { background-color: transparent; border: none; }"
            "QToolButton:hover { background-color: rgba(200, 200, 200, 0.5); }"
        );

        connect(button, &QPushButton::clicked, this, &MainWindow::fPathClicked);

        fButtons.push_back(button);
        funcLayout->addWidget(button);
    }
    funcLayout->addStretch();

    // 搜索
    fSearchEdit = new QLineEdit();
    fSearchEdit->setFixedHeight(buttonHeight);
    fSearchEdit->setFixedWidth(470);

    fdFont = fSearchEdit->font();
    fdFont.setPointSize(fontSize);
    fdFont.setFamilies(QStringList() << "Microsoft YaHei" << "Malgun Gothic" << "Segoe UI");
    fSearchEdit->setFont(fdFont);

    fSearchModel = new QStandardItemModel(fDialog);

    funcLayout->addWidget(fSearchEdit);

    mainLayout->addLayout(funcLayout);
    mainLayout->addWidget(fStack);

    fileView->viewport()->installEventFilter(clickFilter);

    // 绑定
    connect(favoritesList, &QListWidget::itemDoubleClicked, this, &MainWindow::favoriteItems);
    connect(favoritesList, &QListWidget::customContextMenuRequested,
            this, &MainWindow::favoritesContextMenu);
    connect(favoritesList, &QListWidget::itemEntered, this, [](QListWidgetItem* item) {
        if (item)
        {
            QString path = item->data(Qt::UserRole).toString();
            item->setToolTip(path);    // 提示完整路径
        }
    });

    connect(fSearchEdit, &QLineEdit::returnPressed, this, &MainWindow::fSearch);
    connect(&fSearchWatcher, &QFutureWatcher<QStringList>::finished,
            this, &MainWindow::handleFSearchFinished);

    connect(fileView, &QTreeView::doubleClicked, this, &MainWindow::fileView_doubleClicked);
    connect(fileView, &QTreeView::customContextMenuRequested, this, &MainWindow::showContextMenu);
    connect(fileView, &MyFileView::filesDropped, this, &MainWindow::handleExternalDropped);
    connect(upButton, &QPushButton::clicked, this, &MainWindow::upClicked);
    connect(downButton, &QPushButton::clicked, this, &MainWindow::downClicked);

    // 快捷键
    QShortcut* fAddToFavorites = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_T), fDialog);
    connect(fAddToFavorites , &QShortcut::activated, this, [this]() {
        this->contextMenuView = fileView;
        this->contextMenuIndex = fileView->currentIndex();
        addToFavorites();
    });

    QShortcut* fNewDirShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_N), fDialog);
    connect(fNewDirShortcut, &QShortcut::activated, this, [this]() {
        this->contextMenuView = fileView;
        this->contextMenuIndex = fileView->currentIndex();
        createDirectory();
    });

    QShortcut* fCutShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_X), fDialog);
    connect(fCutShortcut, &QShortcut::activated, this, [this]() {
        this->contextMenuView = fileView;
        this->contextMenuIndex = fileView->currentIndex();
        cut();
    });

    QShortcut* fCopyShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_C), fDialog);
    connect(fCopyShortcut, &QShortcut::activated, this, [this]() {
        this->contextMenuView = fileView;
        this->contextMenuIndex = fileView->currentIndex();
        copy();
    });

    QShortcut* fPasteShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_V), fDialog);
    connect(fPasteShortcut, &QShortcut::activated, this, [this]() {
        this->contextMenuView = fileView;
        this->contextMenuIndex = fileView->currentIndex();
        paste("");
    });

    QShortcut* fDeleteShortcut = new QShortcut(QKeySequence(Qt::Key_Delete), fDialog);
    connect(fDeleteShortcut, &QShortcut::activated, this, [this]() {
        this->contextMenuView = fileView;
        this->contextMenuIndex = fileView->currentIndex();
        toRecyleBin();
    });

    QShortcut* fRecycleBinShortcut = new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Delete), fDialog);
    connect(fRecycleBinShortcut, &QShortcut::activated, this, [this]() {
        this->contextMenuView = fileView;
        this->contextMenuIndex = fileView->currentIndex();
        del(QStringList(), true);
    });

    QShortcut* fRenameShortcut = new QShortcut(QKeySequence(Qt::Key_F2), fDialog);
    connect(fRenameShortcut, &QShortcut::activated, this, [this]() {
        this->contextMenuView = fileView;
        this->contextMenuIndex = fileView->currentIndex();
        rename();
    });

    QShortcut* fPropertiesShortcut = new QShortcut(QKeySequence(Qt::ALT + Qt::Key_Return), fDialog);
    connect(fPropertiesShortcut, &QShortcut::activated, this, [this]() {
        this->contextMenuView = fileView;
        this->contextMenuIndex = fileView->currentIndex();
        properties();
    });

    fDialog ->show();
}

// 文件排序
void MainWindow::fileSort()
{
    QString selectedText = sortComboBox->currentText();

    if (selectedText == "默认")
    {
        currentSort = "文件名";
        currentOrder = Qt::AscendingOrder;
    }

    if (sortMap.contains(selectedText)) currentSort = selectedText;

    if (selectedText == "降序") currentOrder = Qt::DescendingOrder;

    if (selectedText == "升序") currentOrder = Qt::AscendingOrder;

    fileModel->sort(sortMap[currentSort], currentOrder);
    return;
}

// 搜索
void MainWindow::search()
{
    // 仅当 leftPath 不为空时搜索, 跳过磁盘搜索
    if (leftPath.isEmpty()) return;

    // 读取关键字
    QString key = searchEdit->text().trimmed();
    if (key.isEmpty())
    {
        if (isSearch)
        {
            leftView->setModel(fileModel);
            QModelIndex idx = fileModel->index(leftPath);
            if (idx.isValid()) leftView->setRootIndex(idx);

            leftView->header()->restoreState(savedHeaderState);
            leftView->setItemDelegate(nullptr);
            leftView->setItemDelegate(new SpacingDelegate(17, this));

            isSearch = false;
        }
        return;
    }

    // 如果之前的搜索仍在运行，取消掉
    if (searchWatcher.isRunning())
    {
        searchWatcher.cancel();
        searchWatcher.waitForFinished();
    }

    // 异步遍历当前目录及其子目录
    auto future = QtConcurrent::run([path = leftPath, key]()
    {
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
    searchModel->setHorizontalHeaderLabels({("结果")});

    for (const QString& path : results)
    {
        QFileInfo fi(path);
        QStandardItem* item = new QStandardItem;
        item->setText(path);
        item->setIcon(fi.isDir()
                      ? QIcon(":/folder.png")
                      : QIcon(":/file.png"));
        item->setToolTip(path);
        searchModel->appendRow(item);
    }

    leftView->setModel(searchModel);
    leftView->setItemDelegate(new SpacingDelegate(27, this));

    // 自适应长度
    if (leftView->header())
        leftView->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);

    isSearch = true;
}

// 调用自定义右键菜单
void MainWindow::showContextMenu(const QPoint& pos)
{
    // 识别是哪个视图
    QAbstractItemView* senderView = qobject_cast<QAbstractItemView*>(sender());
    if (!senderView) return;
    QModelIndex index = senderView->indexAt(pos);

    contextMenuView = senderView;
    contextMenuIndex = index;

    // 判断是否是搜索模式
    if (isSearch || (senderView == fileView && isFSearch))
    {
        if (!index.isValid()) return;
        QString path;

        if (isSearch)
            path = index.data(Qt::ToolTipRole).toString();
        else if (isFSearch)
            path = index.data(Qt::UserRole).toString();

        QMenu menu;
        QAction* copyPathAction = menu.addAction("复制路径");
        QAction* sel = menu.exec(senderView->viewport()->mapToGlobal(pos));
        if (sel == copyPathAction)
        {
            QClipboard* clipboard = QApplication::clipboard();
            clipboard->setText(path);
            QToolTip::showText(QCursor::pos(), QString("已复制 : %1").arg(path));
        }
        return;
    }

    // 根据 senderView 设置 currentContextIndex
    if (index.isValid())
    {
        currentContextIndex = index;
        rightClickMenu->cutChecked(clipIsCut);

        if (senderView == leftView)
            rightClickMenu->itemsBlock(leftPath);
        else if (senderView == fileView)
            rightClickMenu->itemsBlock(fPath);
        else
            rightClickMenu->itemsBlock("");
    }
    else
    {
        currentContextIndex = QModelIndex();
        if (senderView == leftView)
            rightClickMenu->whiteBlock(leftPath);
        else if (senderView == fileView)
            rightClickMenu->whiteBlock(mainPath);
        else
            rightClickMenu->whiteBlock("");
    }

    rightClickMenu->exec(senderView->viewport()->mapToGlobal(pos));
}

// 获取文件关联应用程序路径
QString MainWindow::getAssociatedSW(const QString& path)
{
    QFileInfo fi(path);
    QString ext = fi.suffix();

    // 从注册表中获取关联程序
    QSettings regExt(QString("HKEY_CLASSES_ROOT\\.%1").arg(ext), QSettings::NativeFormat);
    QString typeName = regExt.value("Default").toString();

    QSettings regCmd(QString("HKEY_CLASSES_ROOT\\%1\\shell\\open\\command").arg(typeName), QSettings::NativeFormat);
    QString cmd = regCmd.value("Default").toString();

    // 解析命令
    cmd.replace("\"%1\"", "\"%2\"").replace("%1", "%2");
    cmd = cmd.arg(fi.absoluteFilePath());

    // 提取程序路径
    QRegularExpression exeRegex("^\"?(.+?\\.exe)");
    QRegularExpressionMatch match = exeRegex.match(cmd);
    return match.hasMatch() ? match.captured(1) : "";
}

// 提权运行程序
bool MainWindow::runAsAdmin(const QString& path, const QString& params)
{
    SHELLEXECUTEINFO sei;
    memset(&sei, 0, sizeof(sei));
    sei.cbSize = sizeof(sei);
    sei.lpVerb = L"runas";
    sei.lpFile = path.toStdWString().c_str();
    sei.lpParameters = params.toStdWString().c_str();
    sei.nShow = SW_SHOW;

    if (!ShellExecuteEx(&sei))
    {
        DWORD err = GetLastError();
        if (err == ERROR_CANCELLED)
            qDebug() << "用户取消UAC提示";
        return false;
    }
    return true;
}

// 复制一整个目录用作压缩和粘贴
void MainWindow::copyDirectory(const QString& srcPath, const QString& dstPath)
{
    QDir srcDir(srcPath);
    if (!srcDir.exists()) return;

    QDir dstDir(dstPath);
    if (!dstDir.exists())
        QDir().mkpath(dstPath);

    // 列出所有文件和子目录
    QFileInfoList entries = srcDir.entryInfoList(
        QDir::AllEntries | QDir::NoDotAndDotDot);
    for (const QFileInfo& fi : entries)
    {
        QString srcFilePath = fi.absoluteFilePath();
        QString dstFilePath = dstPath + QDir::separator() + fi.fileName();
        if (fi.isDir())
            copyDirectory(srcFilePath, dstFilePath);
        else
            QFile::copy(srcFilePath, dstFilePath);
    }
}

// 清除剪切状态
void MainWindow::clearCutStateIfNeeded()
{
    if (!clipIsCut) return;

    clipPaths.clear();
    clipIsCut = false;
    if (contextMenuView == leftView)
    {
        leftView->viewport()->update();
        rightView->viewport()->update();
    }
    else if (contextMenuView == fileView)
        fileView->viewport()->update();
}


// 刷新
void MainWindow::refreshInterface()
{
    refreshView();
}

// 打开
void MainWindow::openFile()
{
    if (!contextMenuView || !contextMenuIndex.isValid()) return;

    openAsAdmin = false;

    if (contextMenuView == leftView)
        leftView_doubleClicked(contextMenuIndex);
    else if (contextMenuView == fileView)
        fileView_doubleClicked(contextMenuIndex);
}

// 以管理员模式运行
void MainWindow::administratorOpen()
{
    if (!contextMenuView || !contextMenuIndex.isValid()) return;
    openAsAdmin = true;

    if (contextMenuView == leftView)
    {
        leftView_doubleClicked(contextMenuIndex);
    }
    else if (contextMenuView == fileView)
    {
        fileView_doubleClicked(contextMenuIndex);
    }
    openAsAdmin = false;
}

// 打开方式
void MainWindow::openWith(const QString& application)
{
    // 如果有“cut”之类的状态，先清理一下
    clearCutStateIfNeeded();

    QString path = fileModel->filePath(contextMenuIndex);

    QString appToRun;
    if (application == QLatin1String("windows"))
    {
        QFileInfo fi(path);
        QString startDir = fi.absolutePath();

        QWidget* p = (contextMenuView == fileView && favoritesDialogPtr)
                     ? static_cast<QWidget*>(favoritesDialogPtr)
                     : this;

        QString selectedApp = QFileDialog::getOpenFileName(
            p,
            "选择用来打开文件的程序",
            startDir,
            "所有文件 (*)"
        );

        if (selectedApp.isEmpty()) return;
        appToRun = selectedApp;
    }
    else appToRun = application;

    // 使用 QProcess 启动并传入当前文件路径
    QProcess::startDetached(appToRun, QStringList() << path);
}

// 压缩
// 如果本地有7z环境，则使用7z完成全部压缩
QString MainWindow::find7z()
{
    QStringList envPaths = qEnvironmentVariable("PATH").split(';', QString::SkipEmptyParts);
    envPaths << "C:/Program Files/7-Zip/"
             << "C:/Program Files (x86)/7-Zip/";
    for (const QString& dir : envPaths)
    {
        QString exe = QDir(dir).filePath("7z.exe");
        if (QFileInfo::exists(exe)) return exe;
    }
    return QString();
}

void MainWindow::renameCompressions(const QString &path)
{
    QFileInfo fi(path);

    QWidget* p = (contextMenuView == fileView && favoritesDialogPtr)
                 ? static_cast<QWidget*>(favoritesDialogPtr)
                 : this;

    QInputDialog dlg(p);
    dlg.setWindowTitle("重命名");
    dlg.setLabelText("输入新名称：");
    dlg.setTextValue(fi.fileName());
    dlg.setOkButtonText("确定");
    dlg.setCancelButtonText("取消");

    dlg.resize(400, 300);
    font.setFamilies(QStringList() << "Microsoft YaHei" << "Malgun Gothic" << "Segoe UI");
    font.setPointSize(10);
    dlg.setFont(font);

    if (dlg.exec() == QDialog::Accepted)
    {
        QString newName = dlg.textValue().trimmed();
        if (!newName.isEmpty() && newName != fi.fileName())
        {
            QString newPath = fi.absolutePath() + QDir::separator() + newName;
            if (!QFile::rename(path, newPath)) QMessageBox::warning(this, "Rename failed", "Unable to rename");
            else updateRightView();
        }
    }
}

bool isDirectoryEmpty(const QString& dirPath)
{
    QDir dir(dirPath);
    dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);
    return dir.entryList().isEmpty();
}

void MainWindow::Compressions(const QString &format)
{
    QWidget* p = (contextMenuView == fileView && favoritesDialogPtr)
                 ? static_cast<QWidget*>(favoritesDialogPtr)
                 : this;

    if (!contextMenuIndex.isValid())
    {
        QMessageBox::warning(p, "错误", "压缩前请选择文件或文件夹。");
        return;
    }

    QString srcPath = fileModel->filePath(contextMenuIndex);
    if (srcPath.isEmpty())
    {
        QMessageBox::warning(p, "错误", "无法获取所选项目的路径。");
        return;
    }

    QString dstPath = srcPath + "." + format;

    QString sevenZipPath = find7z();
    QString program;
    QStringList args;
    bool usePowerShell = false;
    bool useTar = false;

    if (format == "7z")
    {
        if (sevenZipPath.isEmpty())
        {
            QMessageBox::warning(p, "环境缺失", "未检测到7z环境。");
            return;
        }
        program = sevenZipPath;
        args << "a" << "-bsp2" << dstPath << srcPath;
    }
    else if (format == "zip")
    {
        if (!sevenZipPath.isEmpty())
        {
            program = sevenZipPath;
            args << "a" << "-tzip" << "-bsp2" << dstPath << srcPath;
        }
        else
        {
            QFileInfo srcInfo(srcPath);
            if (srcInfo.isDir() && isDirectoryEmpty(srcPath))
            {
                QString tempFilePath = QDir(srcPath).filePath(".can-be-deleted");
                if (!QFileInfo::exists(tempFilePath))
                {
                    QFile tempFile(tempFilePath);
                    tempFile.open(QIODevice::WriteOnly);
                    tempFile.close();
                }
            }
            program = "powershell";
            usePowerShell = true;
            args << "-Command"
                 << QString("Compress-Archive -Path \"%1\" -DestinationPath \"%2\" -Force")
                        .arg(srcPath, dstPath);
        }
    }
    else if (format == "tar")
    {
        if (!sevenZipPath.isEmpty())
        {

            program = sevenZipPath;
            args << "a" << "-ttar" << "-bsp2" << dstPath << srcPath;
        }
        else
        {
            program = "tar";
            useTar = true;
            args << "-cf" << dstPath << "-C" << QFileInfo(srcPath).absolutePath() << QFileInfo(srcPath).fileName();
        }
    }
    else
    {
        QMessageBox::warning(p, "不支持的格式", QString("不支持的格式 : %1").arg(format));
        return;
    }

    if (m_progressDialog)
    {
        m_progressDialog->close();
        m_progressDialog->deleteLater();
        m_progressDialog = nullptr;
    }

    m_progressDialog = new QProgressDialog(
        QString("压缩中 %1 …").arg(QFileInfo(dstPath).fileName()),
        "取消", 0, 100, p);

    m_progressDialog->setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
    m_progressDialog->setWindowModality(Qt::NonModal);
    m_progressDialog->setAutoClose(false);
    m_progressDialog->setAutoReset(false);

    connect(m_progressDialog, &QProgressDialog::canceled, p, [this]() {
        if (m_compressProc) m_compressProc->kill();
    });
    m_progressDialog->resize(600, 120);
    m_progressDialog->show();

    if (m_compressProc)
    {
        m_compressProc->kill();
        m_compressProc->deleteLater();
        m_compressProc = nullptr;
    }
    m_compressProc = new QProcess(p);

    // 7z/zip/tar 分支, 使用 7z
    if (!usePowerShell && !useTar)
    {
        auto updateProgress = [this](const QByteArray& output)
        {
            static QRegularExpression re(QStringLiteral("(\\d+)%"));
            QString out = QString::fromLocal8Bit(output);
            QRegularExpressionMatchIterator i = re.globalMatch(out);
            int lastPercent = -1;
            while (i.hasNext())
            {
                QRegularExpressionMatch match = i.next();
                lastPercent = match.captured(1).toInt();
            }
            if (lastPercent >= 0)
                m_progressDialog->setValue(lastPercent);
        };
        connect(m_compressProc, &QProcess::readyReadStandardError, p, [this, updateProgress]() {
            updateProgress(m_compressProc->readAllStandardError());
        });
        connect(m_compressProc, &QProcess::readyReadStandardOutput, p, [this, updateProgress]() {
            updateProgress(m_compressProc->readAllStandardOutput());
        });
    }

    // PowerShell 和系统 tar 分支与伪进度
    if (usePowerShell || useTar)
    {
        if (m_fakeProgressTimer)
        {
            m_fakeProgressTimer->stop();
            m_fakeProgressTimer->deleteLater();
            m_fakeProgressTimer = nullptr;
        }
        m_fakeProgressValue = 0;
        m_progressDialog->setValue(0);
        m_fakeProgressTimer = new QTimer(p);

        connect(m_fakeProgressTimer, &QTimer::timeout, this, [this]() {
            if (m_progressDialog->value() < 99)
            {
                m_fakeProgressValue += 1;
                m_progressDialog->setValue(m_fakeProgressValue);
            }
            else
                m_fakeProgressTimer->stop();
        });
        m_fakeProgressTimer->start(50);    // 大约5秒后进度条会满
    }

    // 结束处理
    connect(m_compressProc,
        QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        this, [=](int exitCode, QProcess::ExitStatus status) {
            Q_UNUSED(status)
            if (m_fakeProgressTimer)
            {
                m_fakeProgressTimer->stop();
                m_fakeProgressTimer->deleteLater();
                m_fakeProgressTimer = nullptr;
            }
            m_progressDialog->setValue(100);
            m_progressDialog->close();

            if (exitCode == 0)
            {
                QMessageBox::information(p, "已完成", QString("压缩文件 : \n%1").arg(dstPath));
                renameCompressions(dstPath);
            }
            else
                QMessageBox::warning(p, "失败", QString("压缩失败 (退出吗 %1)").arg(exitCode));

            m_compressProc->deleteLater();
            m_compressProc = nullptr;
        });

    m_progressDialog->setValue(0);
    m_compressProc->start(program, args);
}

// 加入收藏
void MainWindow::saveFavoritesToFile()
{
    QJsonObject root;

    for (auto it = favorites.begin(); it != favorites.end(); ++it)
    {
        QJsonArray pathsArray;
        for (const QString& path : it.value())
            pathsArray.append(path);

        root[it.key()] = pathsArray;
    }

    QJsonDocument doc(root);
    QFile file("./favorites.json");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
    }
}

void MainWindow::deleteFileFromFavorites()
{
    loadFavoritesFromFile();

    // 1. 先获取当前选中的文件/文件夹路径
    QModelIndex index = leftView->currentIndex();
    if (!index.isValid()) return;

    QString path = fileModel->filePath(index);

    // 2. 弹出对话框，让用户选择要移除的收藏类别
    QInputDialog inputDialog(this);
    inputDialog.setWindowTitle("移除收藏");
    inputDialog.setLabelText("请选择要从哪个类别中移除 : ");
    inputDialog.setComboBoxItems(categories);
    inputDialog.setComboBoxEditable(false);
    // 使用同样的字体设置
    inputDialog.setFont(font);
    inputDialog.resize(400, 300);

    if (inputDialog.exec() != QDialog::Accepted)
        return;

    QString category = inputDialog.textValue();
    if (category.isEmpty())
        return;

    // 3. 从 favorites 中删除，并保存到文件
    // removeAll 会返回删除的元素个数
    int removed = favorites[category].removeAll(path);
    if (removed > 0)
    {
        saveFavoritesToFile();
        QMessageBox::information(
            this,
            "移除收藏",
            QString("已成功将路径 \"%1\" 从“%2”收藏中移除。").arg(path, category)
        );
    }
    else
    {
        QMessageBox::information(
            this,
            "移除收藏",
            QString("路径 \"%1\" 不在“%2”收藏中。").arg(path, category)
        );
    }
}

void MainWindow::loadFavoritesFromFile()
{
    favorites.clear();

    QFile file("./favorites.json");
    if (!file.exists()) return;

    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QByteArray data = file.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        file.close();

        QJsonObject root = doc.object();
        for (const QString& category : root.keys())
        {
            QJsonArray arr = root[category].toArray();
            QStringList pathList;
            for (const QJsonValue& val : arr)
                pathList.append(val.toString());

            favorites[category] = pathList;
        }
    }

    for (const QString& c : categories)
    {
        if (!favorites.contains(c))
            favorites[c] = QStringList();
    }
}

void MainWindow::collect(const QString& category, const QString& path)
{
    QWidget* p = (contextMenuView == fileView && favoritesDialogPtr)
                 ? static_cast<QWidget*>(favoritesDialogPtr)
                 : this;

    if (!favorites[category].contains(path))
    {
        favorites[category].append(path);
        saveFavoritesToFile();
        QMessageBox::information(p, "加入收藏", QString("成功将路径 \"%1\" 加入到“%2”收藏中。").arg(path, category));
    }
    else
        QMessageBox::information(p, "加入收藏", QString("路径 \"%1\" 已经存在于“%2”收藏中。").arg(path, category));
}

void MainWindow::addToFavorites()
{
    loadFavoritesFromFile();

    // 获取所有选定的项目 (仅取column==0以防止重复)
    QModelIndexList selected = contextMenuView->selectionModel()->selectedIndexes();
    QSet<QString> uniquePaths;
    for (const QModelIndex& idx : selected)
    {
        if (idx.column() == 0)
            uniquePaths.insert(fileModel->filePath(idx));
    }
    if (uniquePaths.isEmpty()) return;

    // 弹出对话框选择喜欢的类别
    QWidget* p = (contextMenuView == fileView && favoritesDialogPtr)
                 ? static_cast<QWidget*>(favoritesDialogPtr)
                 : this;

    QInputDialog inputDialog(p);
    inputDialog.setWindowTitle("加入收藏夹");
    inputDialog.setLabelText("请选择收藏夹类别 : ");
    inputDialog.setComboBoxItems(categories);
    inputDialog.setComboBoxEditable(false);

    // 设置更大的字体
    font.setFamilies(QStringList() << "Microsoft YaHei" << "Malgun Gothic" << "Segoe UI");
    font.setPointSize(11);
    inputDialog.setFont(font);
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
        // 批量添加所有选定的文件/文件夹
        for (const QString& path : uniquePaths)
        {
            collect(category, path);
        }
    }
}

// 新建
void MainWindow::createNew(const QString& type)
{
    // —— 1. 取当前目录 ——
    QModelIndex currIdx = contextMenuView->rootIndex();
    QString dirPath = fileModel->filePath(currIdx);
    if (dirPath.isEmpty() || !QDir(dirPath).exists()) return;

    // —— 2. 根据 type 决定后缀、默认名，以及是否用 COM ——
    QString suffix, defaultName;
    bool useCom = false;

    if      (type == "directory") { defaultName = tr("新建文件夹"); }
    else if (type == "image")     { suffix = ".png";  defaultName = tr("新建图像"); }
    else if (type == "txt")       { suffix = ".txt";  defaultName = tr("新建文本文档"); }
    else if (type == "docx")      { suffix = ".docx"; defaultName = tr("新建Word文档");  useCom = true; }
    else if (type == "pdf")       { suffix = ".pdf";  defaultName = tr("新建PDF文档"); }
    else if (type == "pptx")      { suffix = ".pptx"; defaultName = tr("新建PPT文稿");  useCom = true; }
    else if (type == "xlsx")      { suffix = ".xlsx"; defaultName = tr("新建Excel表格");useCom = true; }
    else                          { return; }

    // —— 3. 弹输入框获取用户文件名 ——
    QWidget* p = (contextMenuView == fileView && favoritesDialogPtr)
                 ? static_cast<QWidget*>(favoritesDialogPtr)
                 : this;

    QInputDialog dlg(p);
    dlg.setWindowTitle("新建");
    dlg.setLabelText("输入名称 : ");
    dlg.setTextValue(defaultName);
    dlg.setOption(QInputDialog::NoButtons, false);
    dlg.setOkButtonText("确定");
    dlg.setCancelButtonText("取消");

    dlg.setFixedSize(400, 300);
    font.setFamilies(QStringList() << "Microsoft YaHei" << "Malgun Gothic" << "Segoe UI");
    font.setPointSize(10);
    dlg.setFont(font);

    // 弹出对话框
    if (dlg.exec() != QDialog::Accepted) return;

    // 得到输入并去除前后空白
    QString userInput = dlg.textValue().trimmed();
    if (userInput.isEmpty()) return;

    // —— 4. 去除多余后缀、准备 finalName/fullPath ——
    QFileInfo inputInfo(userInput);
    QString baseName  = inputInfo.completeBaseName();    // 支持多种扩展
    QString finalName = (type == "directory")
                        ? baseName
                        : baseName + suffix;

    QString fullPath;
    int counter = 1;

    // 保证不重名
    do {
        fullPath = QDir(dirPath).filePath(finalName);
        if (!QFileInfo::exists(fullPath)) break;
        finalName = QString("%1(%2)%3")
                        .arg(baseName)
                        .arg(counter++)
                        .arg(suffix);
    } while (true);

    // —— 5. 所有类型的统一异步创建 ——
    auto createFunc = [=]() -> bool {
        // 目录：同步创建
        if (type == "directory")
            return QDir(dirPath).mkdir(finalName);

        // 普通文件：同步创建
        else if (!useCom)
        {
            if (type == "pdf")
            {
                QPdfWriter writer(fullPath);
                writer.setPageSize(QPageSize(QPageSize::A4));
                QPainter painter(&writer);
                painter.end();
                return true;
            }
            else if (type == "image")
            {
                QImage img(100, 100, QImage::Format_ARGB32);
                img.fill(Qt::transparent);
                return img.save(fullPath, "PNG");
            }
            else if (type == "txt")
            {
                QFile file(fullPath);
                bool ok = file.open(QIODevice::WriteOnly | QIODevice::Text);
                file.close();
                return ok;
            }
        }

        // Office 文件：异步创建以避免阻塞
        if (useCom)
        {
#ifdef Q_OS_WIN
            bool ok = false;
            if (type == "docx")
            {
                QAxObject word("Word.Application");
                word.setProperty("Visible", false);
                auto docs = word.querySubObject("Documents");
                auto doc  = docs->querySubObject("Add()");
                doc->dynamicCall("SaveAs(const QString&)",
                                 QDir::toNativeSeparators(fullPath));
                doc->dynamicCall("Close()");
                word.dynamicCall("Quit()");
                ok = true;
            }
            else if (type == "xlsx")
            {
                QAxObject excel("Excel.Application");
                excel.setProperty("Visible", false);
                auto books = excel.querySubObject("Workbooks");
                auto book  = books->querySubObject("Add()");
                book->dynamicCall("SaveAs(const QString&)",
                                  QDir::toNativeSeparators(fullPath));
                book->dynamicCall("Close()");
                excel.dynamicCall("Quit()");
                ok = true;
            }
            else if (type == "pptx")
            {
                QAxObject ppt("PowerPoint.Application");
                ppt.setProperty("Visible", false);
                auto preses = ppt.querySubObject("Presentations");
                auto pres   = preses->querySubObject("Add()");
                pres->dynamicCall("SaveAs(const QString&)",
                                  QDir::toNativeSeparators(fullPath));
                pres->dynamicCall("Close()");
                ppt.dynamicCall("Quit()");
                ok = true;
            }
            return ok;
#else
            Q_UNUSED(fullPath);
            Q_UNUSED(type);
            return false;
#endif
        }
        return false;
    };

    // Use watcher to return to the main thread to refresh or pop up a warning after the background execution is completed
    QFutureWatcher<bool>* watcher = new QFutureWatcher<bool>(this);
    connect(watcher, &QFutureWatcher<bool>::finished, this, [=]() {
        bool ok = watcher->result();
        watcher->deleteLater();
        if (!ok)
        {
            QMessageBox::warning(p, "错误", QString("无法创建 : \n%1").arg(fullPath));
        }
        else
        {
            qDebug() << "创建成功 : " << fullPath;
            //refreshView();
        }
    });
    watcher->setFuture(QtConcurrent::run(createFunc));
}

// 新建文件夹
void MainWindow::createDirectory()
{
    QString type = "directory";
    createNew(type);
}

// 复制文件路径
void MainWindow::copyFilePath()
{
    //QModelIndex idx = leftView->currentIndex();
    //if (!idx.isValid()) return;
    QString path = fileModel->filePath(contextMenuIndex);
    QApplication::clipboard()->setText(path);

    QWidget* p = (contextMenuView == fileView && favoritesDialogPtr)
                 ? static_cast<QWidget*>(favoritesDialogPtr)
                 : this;

    QToolTip::showText(QCursor::pos(), QString("已复制 : %1").arg(path), p);
}

// 跨平台快捷方式
// Windows
#ifdef Q_OS_WIN
bool MainWindow::createWindowsShortcut(const QString& targetPath, const QString& linkPath)
{
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) return false;

    IShellLink* psl = nullptr;
    hr = CoCreateInstance(CLSID_ShellLink, nullptr,
                          CLSCTX_INPROC_SERVER, IID_IShellLinkW,
                          (LPVOID*)&psl);
    if (SUCCEEDED(hr))
    {
        psl->SetPath((LPCWSTR)targetPath.utf16());
        psl->SetWorkingDirectory((LPCWSTR)QFileInfo(targetPath).absolutePath().utf16());
        IPersistFile* ppf = nullptr;
        hr = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);
        if (SUCCEEDED(hr))
        {
            hr = ppf->Save((LPCWSTR)linkPath.utf16(), TRUE);
            ppf->Release();
        }
        psl->Release();
    }
    CoUninitialize();
    return SUCCEEDED(hr);
}
#endif

// Linux
#ifdef Q_OS_LINUX
bool MainWindow::createLinuxDesktopFile(const QString& targetPath, const QString& linkPath)
{
    QFile f(linkPath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    QTextStream out(&f);
    out << "[Desktop Entry]\n";
    out << "Type=Application\n";
    out << "Name=" << QFileInfo(targetPath).fileName() << "\n";
    out << "Exec=xdg-open \"" << targetPath << "\"\n";
    out << "Icon=\n";
    out << "Terminal=false\n";
    f.close();
    f.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
    return true;
}
#endif

// MacOS
#ifdef Q_OS_MAC
bool MainWindow::createMacAlias(const QString& targetPath, const QString& linkPath)
{
    QString parent = QFileInfo(linkPath).absolutePath();
    QString script = QString(
        "tell application \"Finder\"\n"
        " set targetFile to POSIX file \"%1\"\n"
        " set parentFolder to POSIX file \"%2\"\n"
        " make alias file to targetFile at parentFolder\n"
        "end tell"
    ).arg(targetPath, parent);

    QProcess p;
    p.start("osascript", QStringList() << "-e" << script);
    p.waitForFinished();
    return (p.exitCode() == 0);
}
#endif

// 创建快捷方式
void MainWindow::createShortCut()
{
    QWidget* p = (contextMenuView == fileView && favoritesDialogPtr)
                 ? static_cast<QWidget*>(favoritesDialogPtr)
                 : this;

    clearCutStateIfNeeded();
    if (!contextMenuIndex.isValid())
    {
        QMessageBox::warning(p, "失败", "没有选择文件或文件夹");
        return;
    }

    QString targetPath = fileModel->filePath(contextMenuIndex);
    QFileInfo fi(targetPath);
    QString name = fi.fileName() + tr(" - shortcut");
    QString dir = fileModel->filePath(contextMenuView->rootIndex());
    QString linkPath;

#ifdef Q_OS_WIN
    linkPath = QDir(dir).filePath(name + ".lnk");
    if (!createWindowsShortcut(targetPath, linkPath))
        QMessageBox::warning(p, "失败", "无法创建快捷方式");
#elif defined(Q_OS_LINUX)
    linkPath = QDir(dir).filePath(name + ".desktop");
    if (!createLinuxDesktopFile(targetPath, linkPath))
        QMessageBox::warning(p, "失败", "无法创建desktop快捷方式");
#elif defined(Q_OS_MAC)
    linkPath = QDir(dir).filePath(name + ".alias");
    if (!createMacAlias(targetPath, linkPath))
        QMessageBox::warning(p, "失败", "无法创建别名");
#endif

    // 刷新视图
    updateRightView();
}

// 剪切
bool MainWindow::isFileAvailableForCut(const QString& path, QString& errorDetail)
{
    QFileInfo fi(path);
    if (!fi.exists())
    {
        errorDetail = "文件不存在";
        return false;
    }
    if (!fi.isWritable())
    {
        errorDetail = "没有写入权限";
        return false;
    }
    if (fi.isFile())
    {
        QFile file(path);
        // 首先判断是否可以以写模式打开
        if (!file.open(QIODevice::ReadWrite))
        {
            // 判断是权限问题还是被占用
            // 尝试先以只读方式打开
            if (file.open(QIODevice::ReadOnly))
            {
                errorDetail = "该文件已被另一个程序占用";
                file.close();
            }
            else
                errorDetail = "文件被占用或者没有权限 (具体原因未知)";
            return false;
        }
        file.close();
    }
    if (fi.isSymLink() && !QFileInfo(fi.symLinkTarget()).exists())
    {
        errorDetail = "符号链接目标无法访问";
        return false;
    }
    // 检查网络/硬件问题 (例如无法访问的路径、不可用的驱动器号)
#ifdef Q_OS_WIN
    if (fi.absoluteFilePath().startsWith("\\\\") && !fi.isReadable())
    {
        errorDetail = "网络路径无法访问";
        return false;
    }
#endif
    return true;
}

void MainWindow::cut()
{
    QWidget* p = (contextMenuView == fileView && favoritesDialogPtr)
                 ? static_cast<QWidget*>(favoritesDialogPtr)
                 : this;

    QModelIndexList selected = contextMenuView->selectionModel()->selectedIndexes();
    QModelIndexList files;
    for (const QModelIndex& idx : selected)
    {
        if (idx.column() == 0) files.append(idx);
    }
    if (files.isEmpty()) return;

    QStringList errorFiles, errorDetails;
    for (const QModelIndex& idx : files)
    {
        QString path = fileModel->filePath(idx);
        QString detail;

        // 确定收藏夹文件视图是否正在打开文件夹
        if (favoritesDialogPtr && fileView->isVisible() && QFileInfo(path).isDir() && mainPath == path)
        {
            QMessageBox::warning(p, "无法剪切", "该文件夹正在被收藏夹使用，无法剪切。");
            return;
        }

        if (!isFileAvailableForCut(path, detail))
        {
            errorFiles << path;
            errorDetails << detail;
        }
    }

    if (!errorFiles.isEmpty())
    {
        QString msg;
        for (int i = 0; i < errorFiles.size(); ++i)
        {
            QFileInfo fi(errorFiles[i]);
            msg += QString("\n名称 : %1\n\n类型 : %2\n\n修改时间 : %3\n\n路径 : %4\n\n错误 : %5\n")
                .arg(fi.fileName())
                .arg(fi.isDir() ? "文件夹" : "文件")
                .arg(fi.lastModified().toString("yyyy-MM-dd hh:mm:ss"))
                .arg(fi.absoluteFilePath())
                .arg(errorDetails[i]);
        }
        QMessageBox::warning(p, "无法剪切", msg);
        return;
    }

    // 全部可以剪切
    clipPaths.clear();
    for (const QModelIndex& idx : files)
    {
        clipPaths.append(fileModel->filePath(idx));
    }
    clipIsCut = true;

    if (contextMenuView == leftView)
    {
        leftView->viewport()->update();
        rightView->viewport()->update();
    }
    else
        fileView->viewport()->update();

    rightClickMenu->isCopy = true;
}

// 复制
void MainWindow::copy()
{
    QModelIndexList selected = contextMenuView->selectionModel()->selectedIndexes();
    QModelIndexList files;
    for (const QModelIndex& idx : selected)
    {
        if (idx.column() == 0) files.append(idx);
    }
    if (files.isEmpty()) return;

    clipPaths.clear();
    for (const QModelIndex& idx : files)
    {
        clipPaths.append(fileModel->filePath(idx));
    }
    clipIsCut = false;

    rightClickMenu->isCopy = true;
}

// 粘贴
void FileCopyTask::doCopy()
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
                {
                    // 只保证目录存在，不要整体删除
                    QDir().mkpath(dstFile);
                }
                else
                {
                    // 仅当目标文件存在且源文件和目标文件路径不同时才覆盖
                    if (QFileInfo::exists(dstFile) && srcFile != dstFile && m_toOverwriteFiles.contains(dstFile))
                        QFile::remove(dstFile);
                    QFile srcF(srcFile);
                    QFile dstF(dstFile);
                    if (!srcF.open(QIODevice::ReadOnly) || !dstF.open(QIODevice::WriteOnly))
                    {
                        emit finished(false, QString("无法打开文件 : %1").arg(srcFile));
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
        }
        else
        {
            // 仅当目标文件存在且源文件和目标文件路径不同时才覆盖
            if (QFileInfo::exists(m_dsts[i]) && m_srcs[i] != m_dsts[i] && m_toOverwriteFiles.contains(m_dsts[i]))
                QFile::remove(m_dsts[i]);
            QFile srcF(m_srcs[i]);
            QFile dstF(m_dsts[i]);
            if (!srcF.open(QIODevice::ReadOnly) || !dstF.open(QIODevice::WriteOnly))
            {
                emit finished(false, QString("无法打开文件 : %1").arg(m_srcs[i]));
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

void MainWindow::paste(const QString& dir)
{
    contextMenuView->setFocus();
    if (clipPaths.isEmpty()) return;

    QWidget* p = (contextMenuView == fileView && favoritesDialogPtr)
                 ? static_cast<QWidget*>(favoritesDialogPtr)
                 : this;

    // 目标目录
    QString dstDir = dir;
    QModelIndexList selected = contextMenuView->selectionModel()->selectedIndexes();
    if (!selected.isEmpty())
        dstDir = QFileInfo(fileModel->filePath(selected.first())).absolutePath();
    else
        dstDir = fileModel->filePath(contextMenuView->rootIndex());

    QStringList srcs, dsts;
    qint64 totalSize = 0;
    QStringList fileConflicts, dirConflicts;
    QSet<QString> toOverwriteFiles, toOverwriteDirs;

    for (const QString& src : clipPaths)
    {
        QFileInfo fi(src);
        QString dstPath = QDir(dstDir).filePath(fi.fileName());
        srcs << src;
        dsts << dstPath;

        if (QFileInfo::exists(dstPath))
        {
            if (fi.isDir() && QFileInfo(dstPath).isDir())
            {
                dirConflicts << QFileInfo(dstPath).fileName();
                toOverwriteDirs.insert(dstPath);
            }
            else if (fi.isFile() && QFileInfo(dstPath).isFile())
            {
                fileConflicts << QFileInfo(dstPath).fileName();
                toOverwriteFiles.insert(dstPath);
            }
        }

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

    // 是否覆盖
    if (!fileConflicts.isEmpty())
    {
        QString info = fileConflicts.join("\n");
        auto btn = QMessageBox::question(p, "文件覆盖确认",
            tr("有同名文件 : \n%1\n 是否覆盖? ").arg(info),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (btn != QMessageBox::Yes)
            return;
    }
    if (!dirConflicts.isEmpty())
    {
        QString info = dirConflicts.join("\n");
        auto btn = QMessageBox::question(p, "文件夹覆盖确认",
            tr("有同名文件夹 : \n%1\n 是否覆盖? ").arg(info),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (btn != QMessageBox::Yes)
            return;
    }

    if (clipIsCut)
    {
        cutMoveMap.clear();
        for (int i = 0; i < srcs.size(); ++i)
        {
            cutMoveMap[srcs[i]] = dsts[i];
        }
    }

    // 启动异步复制，传递需要覆盖的文件和文件夹
    QThread* thread = new QThread;
    FileCopyTask* worker = new FileCopyTask(srcs, dsts, totalSize, toOverwriteFiles, toOverwriteDirs);
    worker->moveToThread(thread);

    QDialog* dialog = nullptr;
    QProgressBar* progressBar = nullptr;
    QLabel* label = nullptr;

    if (totalSize > 4.7 * 1024 * 1024 * 1024)
    {
        dialog = new QDialog(p);
        dialog->setWindowTitle("粘贴中...");
        dialog->setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
        dialog->setWindowModality(Qt::NonModal);
        dialog->setFixedSize(400, 300);

        QVBoxLayout* layout = new QVBoxLayout(dialog);
        label = new QLabel("正在粘贴, 请稍候...");
        label->setWordWrap(true);    // 允许换行
        label->setAlignment(Qt::AlignCenter);    // 居中对齐
        progressBar = new QProgressBar(dialog);
        progressBar->setRange(0, 100);
        layout->addWidget(label);
        layout->addWidget(progressBar);

        dialog->show();

        connect(worker, &FileCopyTask::progress, this, [=](qint64 copied, qint64 total) {
            int percent = total > 0 ? int(100.0 * copied / total) : 0;
            progressBar->setValue(percent);
            //dialog->adjustSize();
        });
        connect(worker, &FileCopyTask::currentFile, this, [=](const QString& filename) {
            // 截断过长的文件名
            QString displayName = filename;
            if (displayName.length() > 70)
                displayName = displayName.left(40) + "..." + displayName.right(10);

            label->setText(QString("正在粘贴 %1, 请稍候...").arg(filename));
            /*dialog->setFixedHeight(170);
            dialog->adjustSize();
            if (dialog->width() > 800)
                dialog->resize(800, dialog->width());*/
        });
    }
    else if (totalSize > 1 * 1024 * 1024 * 1024)
    {
        dialog = new QDialog(p);
        dialog->setWindowTitle("粘贴中...");
        dialog->setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
        dialog->setWindowModality(Qt::NonModal);
        dialog->setFixedSize(400, 300);

        QVBoxLayout* layout = new QVBoxLayout(dialog);
        label = new QLabel("正在粘贴, 请稍候...");
        label->setWordWrap(true);
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget(label);

        dialog->show();

        connect(worker, &FileCopyTask::currentFile, this, [=](const QString& filename) {
            QString displayName = filename;
            if (displayName.length() > 70)
                displayName = displayName.left(40) + "..." + displayName.right(10);

            label->setText(QString("正在粘贴 %1, 请稍候...").arg(filename));
        });
    }

    connect(thread, &QThread::started, worker, &FileCopyTask::doCopy);
    connect(worker, &FileCopyTask::finished, this, [=](bool success, const QString &err) {
        if (dialog) dialog->close();
        if (!success) QMessageBox::warning(this, "粘贴失败 : ", err);
        else
        {
            if (clipIsCut)
            {
                del(srcs, false);
                clipIsCut = false;
            }
            //updateRightView();

            if (contextMenuView == leftView)
            {
                leftView->viewport()->update();
                rightView->viewport()->update();
            }
            else if (contextMenuView == fileView)
                fileView->viewport()->update();
        }
        thread->quit();
        thread->wait();
        worker->deleteLater();
        thread->deleteLater();
        if (dialog) dialog->deleteLater();
    });
    thread->start();

    if (totalSize <= 1 * 1024 * 1024 * 1024)
    {
        connect(thread, &QThread::started, worker, &FileCopyTask::doCopy);
        connect(worker, &FileCopyTask::finished, this, [=](bool success, const QString &err) {
            if (!success) QMessageBox::warning(this, "粘贴失败 : ", err);
            else
            {
                if (clipIsCut)
                {
                    del(srcs, false);
                    clipIsCut = false;
                }
                //updateRightView();

                if (contextMenuView == leftView)
                {
                    leftView->viewport()->update();
                    rightView->viewport()->update();
                }
                else if (contextMenuView == fileView)
                    fileView->viewport()->update();
            }
            thread->quit();
            thread->wait();
            worker->deleteLater();
            thread->deleteLater();
        });
        thread->start();
    }

    // 粘贴后清理痕迹
    clipPaths.clear();
    rightClickMenu->isCopy = false;
}

// 删除
void MainWindow::fileDeleteError(const QString& path, const QString& errorMsg, QFileInfo info)
{
    QWidget* p = (contextMenuView == fileView && favoritesDialogPtr)
                 ? static_cast<QWidget*>(favoritesDialogPtr)
                 : this;

    // Construction details
    QString details = QString("名称 : %1\n后缀 : %2\n修改时间 : %3\n路径 : %4")
        .arg(info.fileName())
        .arg(info.suffix())
        .arg(info.lastModified().toString("yyyy-MM-dd hh:mm:ss"))
        .arg(info.absoluteFilePath());

    QDialog dlg(p);
    dlg.setWindowTitle("删除失败");
    QVBoxLayout* layout = new QVBoxLayout(&dlg);

    QLabel* label = new QLabel(QString("%1\n\n%2").arg(errorMsg, details));
    layout->addWidget(label);

    QCheckBox* applyAllBox = new QCheckBox("自动对后续同类型的文件执行此操作");
    layout->addWidget(applyAllBox);

    QHBoxLayout* btnLayout = new QHBoxLayout;
    QPushButton* retryBtn = new QPushButton("重试");
    QPushButton* skipBtn = new QPushButton("跳过");
    QPushButton* cancelBtn = new QPushButton("取消");
    btnLayout->addWidget(retryBtn);
    btnLayout->addWidget(skipBtn);
    btnLayout->addWidget(cancelBtn);
    layout->addLayout(btnLayout);

    int userAction = -1;
    QObject::connect(retryBtn, &QPushButton::clicked, [&]() { userAction = 0; dlg.accept(); });
    QObject::connect(skipBtn, &QPushButton::clicked, [&]() { userAction = 1; dlg.accept(); });
    QObject::connect(cancelBtn, &QPushButton::clicked, [&]() { userAction = 2; dlg.accept(); });

    dlg.exec();

    // 发出用户选择返回信号
    emit fileDeleteTask->userDeleteChoice(userAction, applyAllBox->isChecked());
}

bool MainWindow::deleteWithWindowsDialog(const QStringList& paths)
{
    std::wstring allSrc;
    for (const QString& path : paths)
    {
        allSrc += path.toStdWString();
        allSrc.push_back(L'\0');
    }
    allSrc.push_back(L'\0');
    std::vector<wchar_t> from(allSrc.begin(), allSrc.end());

    SHFILEOPSTRUCTW fileOp = {};
    fileOp.wFunc = FO_DELETE;
    fileOp.pFrom = from.data();
    fileOp.pTo = nullptr;
    // 不带FOF_ALLOWUNDO，彻底删除，弹出原生冲突/错误对话框
    fileOp.fFlags = 0;
    int res = SHFileOperationW(&fileOp);
    return (res == 0 && !fileOp.fAnyOperationsAborted);
}

bool MainWindow::moveToRecycleBin(const QString& path)
{
#ifdef Q_OS_WIN
    std::wstring wpath = path.toStdWString();
    std::vector<wchar_t> from(wpath.begin(), wpath.end());
    from.push_back(L'\0');
    from.push_back(L'\0');

    SHFILEOPSTRUCTW fileOp;
    fileOp.wFunc = FO_DELETE;
    fileOp.pFrom = from.data();
    fileOp.fFlags = FOF_ALLOWUNDO;
    //fileOp.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_SILENT;

    int res = SHFileOperationW(&fileOp);
    return (res == 0 && !fileOp.fAnyOperationsAborted);
#else
    Q_UNUSED(path);
    return false;
#endif
}

void FileDeleteTask::doDelete(bool toRecycleBin)
{
    qint64 deleted = 0;
    bool allOk = true;
    bool skipAll = false;
    bool retryAll = false;
    bool cancelAll = false;

    for (const QString& p : m_paths)
    {
        QFileInfo fi(p);
        emit currentFile(fi.fileName());

        bool ok = false;
        int userAction = 0;    // 0=重试, 1=跳过, 2=取消
        static bool applyToAll = false;
        static int lastAction = 0;

        while (true)
        {
            if (cancelAll) break;

            if (toRecycleBin)
            {
                #ifdef Q_OS_WIN
                ok = MainWindow::moveToRecycleBin(p);
                #else
                ok = MainWindow::moveToTrash(p);
                #endif
            }
            else
            {
                if (fi.isDir())
                {
                    QDir dir(p);
                    ok = dir.removeRecursively();
                }
                else
                    ok = QFile::remove(p);
            }

            // 成功，继续下一个文件
            if (ok) break;

            // 失败，处理冲突
            if (applyToAll)
                userAction = lastAction;
            else
            {
                // 向主线程发送信号以弹出对话框
                // 阻塞等待用户选择
                QEventLoop loop;
                int result = 0;
                bool applyAll = false;
                emit fileDeleteError(p, "删除文件/文件夹失败", fi);

                connect(this, &FileDeleteTask::userDeleteChoice, &loop, [&](int action, bool applyToAllFlag) {
                    result = action;
                    applyAll = applyToAllFlag;
                    loop.quit();
                });
                loop.exec();

                userAction = result;
                if (applyAll)
                {
                    applyToAll = true;
                    lastAction = userAction;
                }
            }

            // 0 = 重试, 1 = 跳过, 2 = 取消
            if (userAction == 0) continue;
            if (userAction == 1) break;
            if (userAction == 2)
            {
                cancelAll = true;
                allOk = false;
                break;
            }
        }

        if (cancelAll) break;

        // 进度统计信息
        if (ok && fi.isFile()) deleted += fi.size();
        emit progress(deleted, m_total);
        QCoreApplication::processEvents();
    }
    emit finished(allOk, allOk ? QString() : QString("删除一些文件/文件夹失败"));
}

void MainWindow::toRecyleBin()
{
    QWidget* p = (contextMenuView == fileView && favoritesDialogPtr)
                 ? static_cast<QWidget*>(favoritesDialogPtr)
                 : this;

    QModelIndexList selected = contextMenuView->selectionModel()->selectedIndexes();
    QModelIndexList files;
    for (const QModelIndex& idx : selected)
    {
        if (idx.column() == 0) files.append(idx);
    }
    if (files.isEmpty()) return;

    QStringList paths;
    qint64 totalSize = 0;
    for (const QModelIndex& idx : files)
    {
        QString path = fileModel->filePath(idx);
        paths << path;
        QFileInfo fi(path);

        if (fi.isDir())
        {
            QDirIterator it(path, QDir::Files | QDir::NoDotAndDotDot | QDir::Dirs, QDirIterator::Subdirectories);
            while (it.hasNext())
            {
                QFileInfo sfi(it.next());
                if (sfi.isFile()) totalSize += sfi.size();
            }
        }
        else
            totalSize += fi.size();
    }

    // 小于1GB的文件将被同步删除
    if (totalSize <= 1 * 1024 * 1024 * 1024)
    {
        bool allOk = true;
        for (const QString &p : paths)
        {
#ifdef Q_OS_WIN
            if (!MainWindow::moveToRecycleBin(p))
                allOk = false;
#else
            if (!MainWindow::moveToTrash(p))
                allOk = false;
#endif
        }
        if (!allOk)
            QMessageBox::warning(p, tr("操作失败"), tr("有些选项无法移至回收站"));
        //else
            //updateRightView();
        return;
    }

    // 异步删除大于1GB的文件
    QThread* thread = new QThread;
    FileDeleteTask* worker = new FileDeleteTask(paths, totalSize);
    worker->moveToThread(thread);
    connect(worker, &FileDeleteTask::fileDeleteError, this, &MainWindow::fileDeleteError);

    QDialog* dialog = nullptr;
    dialog->layout()->setSizeConstraint(QLayout::SetFixedSize);
    QProgressBar* progressBar = nullptr;
    QLabel* label = nullptr;

    if (totalSize > 4.7 * 1024 * 1024 * 1024)
    {
        dialog = new QDialog(p);
        dialog->setWindowTitle("正在移至回收站...");
        dialog->setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
        dialog->setWindowModality(Qt::NonModal);
        dialog->setFixedSize(400, 300);

        QVBoxLayout* layout = new QVBoxLayout(dialog);
        label = new QLabel("正在移至回收站...");
        label->setWordWrap(true);    // 允许自动换行
        label->setAlignment(Qt::AlignCenter);    // 居中对齐
        progressBar = new QProgressBar(dialog);
        progressBar->setRange(0, 100);
        layout->addWidget(label);
        layout->addWidget(progressBar);

        dialog->show();

        connect(worker, &FileDeleteTask::progress, this, [=](qint64 deleted, qint64 total) {
            int percent = total > 0 ? int(100.0 * deleted / total) : 0;
            progressBar->setValue(percent);
            //dialog->adjustSize();
        });
    }
    else
    {
        dialog = new QDialog(p);
        dialog->setWindowTitle("正在移至回收站...");
        dialog->setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
        dialog->setWindowModality(Qt::NonModal);
        dialog->resize(400, 90);

        QVBoxLayout* layout = new QVBoxLayout(dialog);
        label = new QLabel("正在移至回收站...");
        label->setWordWrap(true);
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget(label);

        dialog->show();
    }

    // 实时显示当前文件名
    connect(worker, &FileDeleteTask::currentFile, this, [=](const QString& filename) {
        // 截断过长的文件名
        QString displayName = filename;
        if (displayName.length() > 70)
            displayName = displayName.left(40) + "..." + displayName.right(10);

        label->setText(QString("正在移至回收站 : %1").arg(filename));
        /*dialog->setFixedHeight(170);
        dialog->adjustSize();
        if (dialog->width() > 800)
            dialog->resize(800, dialog->width());*/
    });

    connect(thread, &QThread::started, [=]() { worker->doDelete(true); });
    connect(worker, &FileDeleteTask::finished, this, [=](bool success, const QString &err) {
        if (dialog) dialog->close();
        if (!success) QMessageBox::warning(p, "操作失败", err);
        else updateRightView();
        thread->quit();
        thread->wait();
        worker->deleteLater();
        thread->deleteLater();
        if (dialog) dialog->deleteLater();
    });
    thread->start();
}

QString MainWindow::getDeleteErrorReason(const QString& path)
{
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly))
    {
        QString errStr = f.errorString();
#ifdef Q_OS_WIN
        if (errStr.contains("被另一个进程使用", Qt::CaseInsensitive) ||
            errStr.contains("共享违规", Qt::CaseInsensitive))
            return "该文件已被占用 (某个程序正在使用它)";
#endif
        if (errStr.contains("没有权限", Qt::CaseInsensitive))
            return "没有权限";
        if (errStr.contains("没有这样的文件或目录", Qt::CaseInsensitive))
            return "文件或文件夹不存在";

        int err = errno;
        if (err == 32 || err == 33)
            return "该文件已被占用 (某个程序正在使用它)";
        else if (err == 13 || err == 5)
            return "没有权限";
        else if (err == 2)
            return "文件或文件夹不存在";
        else if (!errStr.isEmpty())
            return QString("无法删除，系统错误信息 : %1").arg(errStr);
        else
            return QString("无法删除，系统错误代码%1 : %2").arg(err).arg(strerror(err));
    }
    f.close();
    return QString();
}

void MainWindow::removeFromFavorites(const QString& path)
{
    bool changed = false;
    for (auto it = favorites.begin(); it != favorites.end(); ++it)
    {
        QStringList& favList = it.value();
        int before = favList.size();
        favList.removeAll(path);
        if (favList.size() != before) changed = true;
    }
    if (changed) saveFavoritesToFile();
}

void MainWindow::del(const QStringList& paths, bool needConfirm)
{
    del(paths, needConfirm, false);
}

void MainWindow::del(const QStringList& paths, bool needConfirm, bool useSystemDelete)
{
    QWidget* p = (contextMenuView == fileView && favoritesDialogPtr)
                 ? static_cast<QWidget*>(favoritesDialogPtr)
                 : this;

    // 统一收集待删除文件/文件夹的路径
    QStringList filesToDelete;
    if (paths.isEmpty())
    {
        // 删除选定项
        QModelIndexList selected = contextMenuView->selectionModel()->selectedIndexes();
        QSet<QString> uniquePaths;
        for (const QModelIndex& idx : selected)
        {
            if (idx.column() == 0)
            {
                QString path;
                if (contextMenuView == fileView && fileView)
                {
                    // 收藏夹文件视图: 根据当前 model 获取路径
                    path = fileModel->filePath(idx);
                }
                else if (contextMenuView == leftView || contextMenuView == rightView)
                {
                    // 主程序视图
                    path = fileModel->filePath(idx);
                }

                if (!path.isEmpty())
                    uniquePaths.insert(path);
            }
        }
        filesToDelete = uniquePaths.values();
    }
    else
        filesToDelete = paths;
    if (filesToDelete.isEmpty()) return;

    // 统计信息
    QStringList names;
    qint64 totalSize = 0;
    for (const QString& p : filesToDelete)
    {
        QFileInfo fi(p);
        names << fi.fileName();
        if (fi.isDir())
        {
            QDirIterator it(p, QDir::Files | QDir::NoDotAndDotDot | QDir::Dirs, QDirIterator::Subdirectories);
            while (it.hasNext())
            {
                QFileInfo sfi(it.next());
                if (sfi.isFile()) totalSize += sfi.size();
            }
        }
        else
            totalSize += fi.size();
    }

    // 非模态确认弹窗
    if (needConfirm)
    {
        QString info = names.join(", ");
        QMessageBox* box = new QMessageBox(QMessageBox::Question,
                                           "Delete",
                                           QString("确认删除 : \n%1 吗? ").arg(info),
                                           QMessageBox::Yes | QMessageBox::No,
                                           p);
        connect(box, &QMessageBox::finished, this, [=](int result){
            if (result == QMessageBox::Yes)
            {
                // 用户确认后，执行异步删除
                this->startAsyncDelete(filesToDelete, totalSize);
            }
            box->deleteLater();
        });
        box->show();
        return; // 等待用户操作
    }

    // 不需要确认时，直接删除
    startAsyncDelete(filesToDelete, totalSize);
}

// 异步删除的实际实现
void MainWindow::startAsyncDelete(const QStringList& filesToDelete, qint64 totalSize)
{
    QWidget* p = (contextMenuView == fileView && favoritesDialogPtr)
                 ? static_cast<QWidget*>(favoritesDialogPtr)
                 : this;

    QThread* thread = new QThread;
    FileDeleteTask* worker = new FileDeleteTask(filesToDelete, totalSize);
    worker->moveToThread(thread);

    QDialog* dialog = nullptr;
    QProgressBar* progressBar = nullptr;
    QLabel* label = nullptr;

    if (totalSize > 4.7 * 1024 * 1024 * 1024)
    {
        dialog = new QDialog(p);
        dialog->setWindowTitle("删除中...");
        dialog->setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
        dialog->setWindowModality(Qt::NonModal);
        dialog->setFixedSize(400, 300);

        QVBoxLayout* layout = new QVBoxLayout(dialog);
        label = new QLabel("正在删除...");
        label->setWordWrap(true);    // 允许自动换行
        label->setAlignment(Qt::AlignCenter);    // 居中对齐
        progressBar = new QProgressBar(dialog);
        progressBar->setRange(0, 100);
        layout->addWidget(label);
        layout->addWidget(progressBar);

        dialog->show();

        connect(worker, &FileDeleteTask::progress, this, [=](qint64 deleted, qint64 total) {
            int percent = total > 0 ? int(100.0 * deleted / total) : 0;
            progressBar->setValue(percent);
            //dialog->adjustSize();
        });
    }
    else
    {
        dialog = new QDialog(p);
        dialog->setWindowTitle("删除中..");
        dialog->setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
        dialog->setWindowModality(Qt::NonModal);
        dialog->setFixedSize(400, 300);

        QVBoxLayout* layout = new QVBoxLayout(dialog);
        label = new QLabel("正在删除...");
        label->setWordWrap(true);
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget(label);

        dialog->show();
    }

    // 实时显示已删除的文件名
    connect(worker, &FileDeleteTask::currentFile, this, [=](const QString& filename) {
        // 截断过长的文件名
        QString displayName = filename;
        if (displayName.length() > 70)
            displayName = displayName.left(40) + "..." + displayName.right(10);

        label->setText(QString("正在删除 : %1").arg(filename));
        /*dialog->setFixedHeight(170);
        dialog->adjustSize();
        if (dialog->width() > 800)
            dialog->resize(800, dialog->width());*/
    });

    connect(thread, &QThread::started, [=]() { worker->doDelete(false); });
    connect(worker, &FileDeleteTask::finished, this, [=](bool success, const QString &err) {
        if (dialog) dialog->close();
        if (!success) QMessageBox::warning(p, "删除失败", err);
        //else updateRightView();

        thread->quit();
        thread->wait();
        worker->deleteLater();
        thread->deleteLater();
        if (dialog) dialog->deleteLater();
    });
    thread->start();
}

// 重命名
void MainWindow::rename()
{
    clearCutStateIfNeeded();

    //QModelIndex idx = contextMenuView->currentIndex();
    //if (!idx.isValid()) return ;

    QString oldPath = fileModel->filePath(contextMenuIndex);
    QFileInfo fi(oldPath);

    QWidget* p = (contextMenuView == fileView && favoritesDialogPtr)
                 ? static_cast<QWidget*>(favoritesDialogPtr)
                 : this;

    QInputDialog dlg(p);
    dlg.setWindowTitle("重命名");
    dlg.setLabelText("输入新名称 : ");
    dlg.setTextValue(fi.fileName());
    dlg.setOkButtonText("确定");
    dlg.setCancelButtonText("取消");

    dlg.resize(400, 300);
    font.setFamilies(QStringList() << "Microsoft YaHei" << "Malgun Gothic" << "Segoe UI");
    font.setPointSize(10);
    dlg.setFont(font);

    if (dlg.exec() == QDialog::Accepted)
    {
        QString newName = dlg.textValue().trimmed();
        if (!newName.isEmpty() && newName != fi.fileName())
        {
            QString newPath = fi.absolutePath() + QDir::separator() + newName;
            if (!QFile::rename(oldPath, newPath)) QMessageBox::warning(this, "重命名失败", "无法重命名");
            //else updateRightView();
        }
    }
}

// 属性
void MainWindow::properties()
{
    clearCutStateIfNeeded();

    QWidget* p = (contextMenuView == fileView && favoritesDialogPtr)
                 ? static_cast<QWidget*>(favoritesDialogPtr)
                 : this;

    //QModelIndex idx = leftView->currentIndex();
    //if (!idx.isValid()) return;

    QString path = fileModel->filePath(contextMenuIndex);
    std::wstring wPath = path.toStdWString();

    SHELLEXECUTEINFOW sei;
    ZeroMemory(&sei, sizeof(sei));
    sei.cbSize = sizeof(sei);
    sei.fMask = SEE_MASK_INVOKEIDLIST | SEE_MASK_FLAG_DDEWAIT;
    sei.hwnd = reinterpret_cast<HWND>(p->winId());
    sei.lpVerb = L"properties";
    sei.lpFile = wPath.c_str();
    sei.nShow = SW_SHOW;

    HRESULT hr = CoInitialize(NULL);
    if (SUCCEEDED(hr))
    {
        BOOL result = ShellExecuteExW(&sei);
        if (!result)
        {
            DWORD err = GetLastError();
            qDebug() << "ShellExecuteExW 失败并出现错误 : " << err;
        }
        CoUninitialize();
    }
    else
        qDebug() << "CoInitialize 失败并显示 HRESULT : " << hr;
}

// 文件预览
void MainWindow::previewFile(const QModelIndex& index)
{
    if (isPreview)
    {
        QString filePath = fileModel->filePath(index);
        QFileInfo fileInfo(filePath);

        // 如果不是文件，直接返回
        if (!fileInfo.isFile()) return;

        QString suffix = fileInfo.suffix().toLower();

        // 如果不支持预览类型，则直接返回
        if (suffix != "txt" &&
            suffix != "png" && suffix != "jpg" &&
            suffix != "jpeg" && suffix != "bmp")
        {
            return;
        }

        // 如果预览框已打开，请先将其关闭
        if (currentPreviewDialog)
        {
            currentPreviewDialog->close();
            currentPreviewDialog->deleteLater();
            currentPreviewDialog = nullptr;
        }

        // 创建新的预览对话框
        currentPreviewDialog = new QDialog(this);
        currentPreviewDialog->setAttribute(Qt::WA_DeleteOnClose);    // 关闭时自动销毁
        currentPreviewDialog->setWindowTitle("预览 : " + fileInfo.fileName());
        QVBoxLayout* layout = new QVBoxLayout(currentPreviewDialog);

        // 当对话框被销毁时，重置 currentPreviewDialog 指针
        connect(currentPreviewDialog, &QObject::destroyed, this, [this](){
            currentPreviewDialog = nullptr;
        });

        // 根据文件类型创建不同的控件
        if (suffix == "txt")
        {
            QTextEdit* textEdit = new QTextEdit(currentPreviewDialog);
            textEdit->setReadOnly(true);
            // 设置更大的字体
            QFont font = textEdit->font();
            font.setPointSize(11);
            textEdit->setFont(font);

            QFile file(filePath);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                textEdit->setText(file.readAll());
                file.close();
            }
            else
                textEdit->setText("无法打开文件！");
            layout->addWidget(textEdit);
        }
        else if (suffix == "png" || suffix == "jpg" ||
                 suffix == "jpeg" || suffix == "bmp")
        {
            // 使用自定义 ScalableImageLabel 显示图像
            ScalableImageLabel* imgLabel = new ScalableImageLabel(currentPreviewDialog);
            QPixmap pixmap(filePath);
            if (pixmap.isNull())
                imgLabel->setText("无法打开图像！");
            else
                imgLabel->setPixmap(pixmap);
            layout->addWidget(imgLabel);
        }

        currentPreviewDialog->setLayout(layout);
        currentPreviewDialog->resize(800, 600);
        currentPreviewDialog->show();
    }
}
