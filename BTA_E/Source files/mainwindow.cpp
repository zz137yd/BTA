#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QThreadPool::globalInstance()->setMaxThreadCount(2);

    this->resize(1600, 900);
    this->setWindowTitle("BTA   ----    Before important operation, remember to backup！");

    // Centerpiece
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // File Display
    fileModel = new MyFileSystemModelEx(this);
    fileModel->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden);
    fileModel->setRootPath(rootPath);

    // leftView is set to open folders and files only when you double-click the left button
    leftView = new MyLeftView(this);
    leftView->setModel(fileModel);
    leftView->setRootIndex(fileModel->index(rootPath));                  // The default displayed path
    leftView->setExpandsOnDoubleClick(false);                            // Disable double click to expand
    leftView->setContextMenuPolicy(Qt::CustomContextMenu);               // Enable custom right-click menu
    // Set the column width strategy to leave space on the right side of the last column
    leftView->header()->setStretchLastSection(false);
    leftView->setSelectionMode(QAbstractItemView::ExtendedSelection);    // Multiple selections possible
    leftView->setItemDelegate(new SpacingDelegate(17, this));

    // Drag event filtering
    leftView->setDragEnabled(true);
    leftView->setAcceptDrops(true);
    //leftView->viewport()->setAcceptDrops(true);
    //leftView->setDragDropMode(QAbstractItemView::DropOnly);
    leftView->setDropIndicatorShown(true);
    leftView->setDragDropMode(QAbstractItemView::DragDrop);
    //leftView->viewport()->installEventFilter(this);

    // RightView is only used to display the parent directory of leftview
    // The directory can be expanded
    rightView = new QTreeView(this);
    rightView->setModel(fileModel);
    rightView->setRootIndex(fileModel->index(rootPath));
    rightView->setContextMenuPolicy(Qt::CustomContextMenu);
    rightView->setItemDelegate(new SpacingDelegate(17, this));

    // Set the displayed font
    //font.setFamily("Segoe UI");
    //font.setFamilies({"Microsoft YaHei", "Malgun Gothic"});
    font.setFamilies(QStringList() << "Microsoft YaHei" << "Malgun Gothic" << "Segoe UI");
    leftView->setFont(font);
    rightView->setFont(font);

    // Set the column width strategy to leave space on the right side of the last column
    leftView->header()->setStretchLastSection(false);
    rightView->header()->setStretchLastSection(false);

    // Adjust file display
    viewAdjust();

    // Allow users to resize column widths
    leftView->header()->setSectionsMovable(true);
    rightView->header()->setSectionsMovable(true);

    // Draw progress bar
    sizeDelegate = new SizeProgressDelegate(fileModel, this);
    sizeDelegate->loadCacheFromFile("");
    connect(sizeDelegate, &SizeProgressDelegate::requestUpdate, [this]() {
        leftView->viewport()->update();
        rightView->viewport()->update();
    });
    leftView->setItemDelegateForColumn(1, sizeDelegate);
    rightView->setItemDelegateForColumn(1, sizeDelegate);

    // Save interface configuration
    savedHeaderState = leftView->header()->saveState();

    // Double-click to open the binding, click to preview
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

    // Forward and Back buttons
    backButton = new QPushButton(this);
    forwardButton = new QPushButton(this);

    // Set to selectable state
    backButton->setCheckable(true);
    forwardButton->setCheckable(true);

    backButton->setFixedHeight(buttonHeight);
    backButton->setFixedWidth(buttonHeight);
    forwardButton->setFixedHeight(buttonHeight);
    forwardButton->setFixedWidth(buttonHeight);

    backButton->setIcon(QIcon(":/left-1.png"));
    forwardButton->setIcon(QIcon(":/right-1.png"));

    // Button style
    QString style = "QPushButton { background-color: transparent; border: none; }";
    backButton->setStyleSheet(style);
    forwardButton->setStyleSheet(style);

    buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(backButton);
    buttonLayout->addWidget(forwardButton);

    connect(backButton, &QPushButton::clicked, this, &MainWindow::backClicked);
    connect(forwardButton, &QPushButton::clicked, this, &MainWindow::forwardClicked);


    // Refresh button
    refreshButton = new QPushButton(this);
    refreshButton->setFixedHeight(buttonHeight);
    refreshButton->setFixedWidth(74);
    refreshButton->setIcon(QIcon(":/refresh.png"));
    refreshButton->setStyleSheet("background-color: transparent; border: none;");

    buttonLayout->addWidget(refreshButton);

    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::refreshView);


    // Path buttons
    for (int i = 0; i < 7; ++i)
    {
        QToolButton* button = new QToolButton(this);

        // Button size changes with path length
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
    // Add a stretchable whitespace
    buttonLayout->addStretch();


    // Button to copy the path
    copyPathButton = new QPushButton(this);
    copyPathButton->setFixedHeight(buttonHeight);
    copyPathButton->setFixedWidth(buttonWidth);
    copyPathButton->setText("Copy path");

    // Set font
    font = copyPathButton->font();
    font.setPointSize(fontSize);
    copyPathButton->setFont(font);

    buttonQss(copyPathButton);

    buttonLayout->addWidget(copyPathButton);
    connect(copyPathButton, &QPushButton::clicked, this, &MainWindow::copyPath);


    // Whether preview
    previewButton = new QPushButton(this);
    previewButton->setFixedHeight(buttonHeight);
    previewButton->setFixedWidth(227);
    previewButton->setText("Preview Disabled");

    font = previewButton->font();
    font.setPointSize(fontSize);
    previewButton->setFont(font);

    // Preview is turned off by default
    isPreview = false;

    buttonQss(previewButton);

    buttonLayout->addWidget(previewButton);
    connect(previewButton, &QPushButton::clicked, this, &MainWindow::preview);


    // Dark mode
    darkButton = new QPushButton();
    darkButton->setFixedHeight(buttonHeight);
    darkButton->setFixedWidth(buttonWidth);
    darkButton->setText("Dark Mode");

    font = darkButton->font();
    font.setPointSize(fontSize);
    darkButton->setFont(font);

    buttonQss(darkButton);

    buttonLayout->addWidget(darkButton);
    connect(darkButton, &QPushButton::clicked, this, &MainWindow::changeDark);


    // Set the favorites to contain only the path
    fButton = new QPushButton(this);
    fButton->setFixedHeight(buttonHeight);
    fButton->setFixedWidth(buttonWidth);
    fButton->setText("Only Path");

    font = fButton->font();
    font.setPointSize(fontSize);
    fButton->setFont(font);

    buttonQss(fButton);

    // Favorites drop-down box
    collectComboBox = new QComboBox(this);
    font = collectComboBox->font();
    font.setPointSize(fontSize);
    collectComboBox->setFont(font);

    collectComboBox->addItem("Work");
    collectComboBox->addItem("Personal");
    collectComboBox->addItem("Key");
    collectComboBox->addItem("Leisure");

    collectComboBox->setFixedHeight(buttonHeight);
    collectComboBox->setFixedWidth(buttonWidth + 1);
    comboboxQss(collectComboBox);

    buttonLayout->addWidget(fButton);
    buttonLayout->addWidget(collectComboBox);

    connect(fButton, &QPushButton::clicked, this, &MainWindow::onlyPath);
    connect(collectComboBox, QOverload<const QString&>::of(&QComboBox::activated), this, &MainWindow::favoritesDialog);


    // File sort
    sortComboBox = new QComboBox(this);
    font = sortComboBox->font();
    font.setPointSize(fontSize);
    sortComboBox->setFont(font);

    sortComboBox->addItem("Name");
    sortComboBox->addItem("Size");
    sortComboBox->addItem("Creation time");
    sortComboBox->addItem("Modification time");
    sortComboBox->addItem("Type");

    sortComboBox->insertSeparator(sortComboBox->count());
    sortComboBox->addItem("Descending");
    sortComboBox->addItem("Ascending");
    sortComboBox->addItem("Default");

    sortComboBox->setFixedHeight(buttonHeight);
    sortComboBox->setFixedWidth(200);
    comboboxQss(sortComboBox);

    // Initialize the mapping table
    sortMap["Name"] = 0;
    sortMap["Size"] = 1;
    sortMap["Creation time"] = fileModel->columnCount() - 2;
    sortMap["Modification time"] = fileModel->columnCount() - 1;
    sortMap["Type"] = 2;

    currentSort = "Name";
    currentOrder = Qt::AscendingOrder;

    buttonLayout->addWidget(sortComboBox);
    connect(sortComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::fileSort);


    // Search
    searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("Search in folder");
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


    // Right-click menu
    leftView->setContextMenuPolicy(Qt::CustomContextMenu);
    rightClickMenu = new RightClickMenu(this);
    categories = QStringList({"Work", "Personal", "Key", "Leisure"});

    // shortcut key
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

    // Display menu when right click triggers
    connect(leftView, &MyLeftView::filesDropped, this, &MainWindow::handleExternalDropped);
    connect(leftView, &QWidget::customContextMenuRequested, this, &MainWindow::showContextMenu);

    // Bind
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

    // File perview
    connect(leftView, &QTreeView::clicked, this, &MainWindow::previewFile);


    // Overall layout
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

// All interface style adjustments
void MainWindow::interfaceStyle()
{
    // Set the column width strategy to leave space on the right side of the last column
    leftView->header()->setStretchLastSection(false);
    rightView->header()->setStretchLastSection(false);

    // leftView is set to open folders and files only when you double-click the left button
    leftView = new MyLeftView(this);
    leftView->setModel(fileModel);
    leftView->setRootIndex(fileModel->index(rootPath));
    leftView->setExpandsOnDoubleClick(false);                            // Disable double click to expand
    leftView->setContextMenuPolicy(Qt::CustomContextMenu);               // Enable custom right-click menu
    leftView->setSelectionMode(QAbstractItemView::ExtendedSelection);    // Multiple selections possible
    leftView->setItemDelegate(new SpacingDelegate(17, this));

    // RightView is only used to display the parent directory of leftview
    // The directory can be expanded
    rightView = new QTreeView(this);
    rightView->setModel(fileModel);
    rightView->setRootIndex(fileModel->index(rootPath));
    rightView->setContextMenuPolicy(Qt::CustomContextMenu);
    rightView->setItemDelegate(new SpacingDelegate(17, this));

    // Set the displayed font
    //font.setFamily("Segoe UI");
    //font.setFamilies({"Microsoft YaHei", "Malgun Gothic"});
    font.setFamilies(QStringList() << "Microsoft YaHei" << "Malgun Gothic" << "Segoe UI");
    leftView->setFont(font);
    rightView->setFont(font);

    // Allow users to resize column widths
    leftView->header()->setSectionsMovable(true);
    rightView->header()->setSectionsMovable(true);

    // Adjust file display
    viewAdjust();

    // Draw progress bar
    sizeDelegate = new SizeProgressDelegate(fileModel, this);
    sizeDelegate->loadCacheFromFile("");
    connect(sizeDelegate, &SizeProgressDelegate::requestUpdate, [this]() {
        leftView->viewport()->update();
        rightView->viewport()->update();
    });
    leftView->setItemDelegateForColumn(1, sizeDelegate);
    rightView->setItemDelegateForColumn(1, sizeDelegate);
}

// Adjust each column in file display area
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

// Drag event filtering
bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == leftView->viewport())
    {
        if (event->type() == QEvent::DragEnter)
        {
            QDragEnterEvent* dragEvent = static_cast<QDragEnterEvent*>(event);

            // Only allowed if leftPath is not empty and the file is dragged
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

// Drag and paste
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

// Update the stack storing the path
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

        // Continue to add paths
        if (ss > ps)
        {
            for (int i = ps; i < ss; ++i)
            {
                stack.push("/" + seg[i]);
            }
        }
        // Handling Fallback
        // If go back to the root directory, there is "" in the stack, and its length is 1
        // After clicking into a disk, the seg length is 1, and the same situation needs to be handled
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

// Update path button
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

        // Calculate the current path length and adjust the button width in real time
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

// Update right area
void MainWindow::updateRightView()
{
    QModelIndex leftRoot = leftView->rootIndex();
    leftPath = fileModel->filePath(leftRoot);
    QDir dir(leftPath);

    if (dir.cdUp()) rightPath = dir.absolutePath();
    else rightPath = rootPath;

    rightView->setRootIndex(fileModel->index(rightPath));
}

// Double click
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
            // Get file association program
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

// Back button
void MainWindow::backClicked()
{
    // Return to the main program interface from the search interface
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

// Forward button
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

// Refresh
void MainWindow::refreshView()
{
    if (!fileModel) return;

    if (contextMenuView == leftView || contextMenuView == rightView)
    {
        QString leftRootPath = fileModel->filePath(leftView->rootIndex());
        QString rightRootPath = fileModel->filePath(rightView->rootIndex());

        // Clear the cache (clears only the current directory and its subdirectories)
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

        // Refresh main program view
        leftView->setRootIndex(fileModel->index(leftRootPath));
        leftView->viewport()->update();

        updateRightView();
        rightView->viewport()->update();
    }
    else if (contextMenuView == fileView && fileView)
    {
        QString fileRootPath = fileModel->filePath(fileView->rootIndex());

        // Clear the cache (clears only the current directory and its subdirectories)
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

        // Rresh favorites file view
        fileView->setRootIndex(fileModel->index(fileRootPath));
        fileView->viewport()->update();
    }
}

// Click path button
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

// Copy current path
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
        QToolTip::showText(QCursor::pos(), "Have copyed", this);
    }
}

// Whether to enable preview
void MainWindow::preview()
{
    if (isPreview)
    {
        isPreview = false;
        previewButton->setText("Preview Disabled");
    }
    else
    {
        isPreview = true;
        previewButton->setText("Preview Enabled");
    }
}

// Dark mode
void MainWindow::changeDark()
{
    isDarkMode = !isDarkMode;

    if(isDarkMode)
    {
        QString eyeStyle = R"(
            QWidget {
                background-color: #2e3b3f;    /* Dark green gray */
                color: #d0e0dc;               /* Soft light green with white text */
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
        darkButton->setText("Normal Mode");

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
        darkButton->setText("Dark Mode");

        buttonQss(copyPathButton);
        buttonQss(previewButton);
        buttonQss(darkButton);
        buttonQss(fButton);

        comboboxQss(collectComboBox);
        comboboxQss(sortComboBox);
    }
}

// Open Favorites
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
            QString("Path copied : %1").arg(paths.first()) :
            QString("%1 paths copied").arg(paths.size());

        QToolTip::showText(QCursor::pos(), message, favoritesList, QRect(), 2000);
    }
}

void MainWindow::moveFavorite(const QList<QListWidgetItem*>& items)
{
    if (items.isEmpty()) return;

    QInputDialog dlg(favoritesDialogPtr);
    dlg.setWindowTitle("Move Collection");
    dlg.setLabelText("Please select the target favorites : ");
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
        "Delete Collection",
        QString("Are you sure you want to delete the following from “%1”?\n%2")
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
    QAction* moveAct = menu.addAction("Move to...");
    QAction* delAct  = menu.addAction("Delete");
    QAction* copyPathAct = menu.addAction("Copy path");
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

    // Determine whether the current path is included in initialPath
    QString normInit = QDir::cleanPath(initialPath);
    QString normCur = QDir::cleanPath(fPath);

    // Split into directory hierarchies
    QStringList initParts = normInit.split('/', Qt::SkipEmptyParts);
    QStringList curParts = normCur.split('/', Qt::SkipEmptyParts);

    // If the current path is not a subdirectory of initialPath
    // or the level is shallower
    // return to the favorites list
    bool flag = (!normCur.startsWith(normInit) || curParts.size() < initParts.size());

    if (fPath == initialPath || flag)
    {
        fPath = "";
        fLabel->setText(QString("Favorite list : %1").arg(fCategory));
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
            // Open the folder with Explorer
            QProcess::startDetached("explorer.exe", QStringList() << QDir::toNativeSeparators(path));
        }
        else if (info.isFile())
        {
            // The "Open With" dialog box pops up
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
                // Get file association program
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
                    QMessageBox::warning(this, "Error", "Unable to open the file with administrator privileges");
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

    // If keyword is empty, restore the original list/view
    if (key.isEmpty())
    {
        isFSearch = false;

        // If currently in favorites list, reload all favorites
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
        // If in file view, reuse the main program search to clear
        else
        {
            fileView->setModel(fileModel);
            QModelIndex idx = fileModel->index(fPath);
            if (idx.isValid()) fileView->setRootIndex(idx);
            //restoreBrowserView();
        }
        return;
    }

    // If still running, cancel
    if (fSearchWatcher.isRunning())
    {
        fSearchWatcher.cancel();
        fSearchWatcher.waitForFinished();
    }

    // Distinguish between favorites list and file view
    if (fStack->currentWidget() == favoritesList)
    {
        // Get all paths of the current category
        QStringList all = favorites.value(fCategory);

        // Asynchronous filtering, keep the chapter names containing keywords
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
        // In favorites list mode, refresh favoritesList
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
        // In file view mode, refresh fileView
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

        // Adaptive length
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

// Main favorites function
void MainWindow::favoritesDialog(const QString& category)
{
    loadFavoritesFromFile();
    fPathStack.clear();

    fDialog = new QDialog(nullptr);
    fDialog ->setWindowTitle(QString("Collections - %1").arg(fCategory));
    fDialog ->resize(1600, 900);
    fDialog ->setMinimumSize(1024, 768);

    // Ability to drag and drop files between the favorites dialog and the main program
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

    // A label to display the current path
    fLabel = new QLabel(QString("Favorites list : %1").arg(fCategory));
    fdFont = fLabel->font();
    fdFont.setPointSize(11);
    fdFont.setFamilies(QStringList() << "Microsoft YaHei" << "Malgun Gothic" << "Segoe UI");
    fLabel->setFont(fdFont);

    // Up and down buttons, hidden by default
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

    // Use the stack widget to manage two views: favorites list and file view
    fStack = new QStackedWidget;

    // Favorites list (QListWidget)
    favoritesList = new QListWidget;
    favoritesList->setFont(fdFont);
    favoritesList->setMouseTracking(true);    // Enable Mouse Tracking

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

    // Right-click menu
    favoritesList->viewport()->installEventFilter(clickFilter);
    favoritesList->setContextMenuPolicy(Qt::CustomContextMenu);

    // File view (QTreeView)
    fileView = new MyFileView;
    fileView->setModel(fileModel);
    fileView->setFont(fdFont);
    fStack->addWidget(fileView);
    fileView->setContextMenuPolicy(Qt::CustomContextMenu);

    // Adjust the display style
    favoritesView();

    // Able to drag and drop
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

    // Progress bar
    fSizeDelegate = new SizeProgressDelegate(fileModel, fDialog);
    fSizeDelegate->loadCacheFromFile("");
    connect(fSizeDelegate, &SizeProgressDelegate::requestUpdate, [this]() {
        fileView->viewport()->update();
    });
    fileView->setItemDelegateForColumn(1,fSizeDelegate);

    fileView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    fileView->header()->setStretchLastSection(false);
    fileView->header()->setSectionsMovable(true);

    // Path button
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

    // Search
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

    // Bind
    connect(favoritesList, &QListWidget::itemDoubleClicked, this, &MainWindow::favoriteItems);
    connect(favoritesList, &QListWidget::customContextMenuRequested,
            this, &MainWindow::favoritesContextMenu);
    connect(favoritesList, &QListWidget::itemEntered, this, [](QListWidgetItem* item) {
        if (item)
        {
            QString path = item->data(Qt::UserRole).toString();
            item->setToolTip(path);    // Prompt for full path
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

    // Short-cut key
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

// File sort
void MainWindow::fileSort()
{
    QString selectedText = sortComboBox->currentText();

    if (selectedText == "Default")
    {
        currentSort = "Name";
        currentOrder = Qt::AscendingOrder;
    }

    if (sortMap.contains(selectedText)) currentSort = selectedText;

    if (selectedText == "Descending") currentOrder = Qt::DescendingOrder;

    if (selectedText == "Ascending") currentOrder = Qt::AscendingOrder;

    fileModel->sort(sortMap[currentSort], currentOrder);
    return;
}

// Search
void MainWindow::search()
{
    // Search only if leftPath is not empty
    // Skipped disk search
    if (leftPath.isEmpty()) return;

    // Read keywords
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

    // If the previous search is still running, cancel it
    if (searchWatcher.isRunning())
    {
        searchWatcher.cancel();
        searchWatcher.waitForFinished();
    }

    // Asynchronously traverse the current directory and its subdirectories
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
    searchModel->setHorizontalHeaderLabels({("Results")});

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

    // Adaptive length
    if (leftView->header())
        leftView->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);

    isSearch = true;
}

// Calling custom right-click menu
void MainWindow::showContextMenu(const QPoint& pos)
{
    // Identify which view it is
    QAbstractItemView* senderView = qobject_cast<QAbstractItemView*>(sender());
    if (!senderView) return;
    QModelIndex index = senderView->indexAt(pos);

    contextMenuView = senderView;
    contextMenuIndex = index;

    // Determine whether it is search mode
    if (isSearch || (senderView == fileView && isFSearch))
    {
        if (!index.isValid()) return;
        QString path;

        if (isSearch)
            path = index.data(Qt::ToolTipRole).toString();
        else if (isFSearch)
            path = index.data(Qt::UserRole).toString();

        QMenu menu;
        QAction* copyPathAction = menu.addAction("Copy path");
        QAction* sel = menu.exec(senderView->viewport()->mapToGlobal(pos));
        if (sel == copyPathAction)
        {
            QClipboard* clipboard = QApplication::clipboard();
            clipboard->setText(path);
            QToolTip::showText(QCursor::pos(), QString("Copied : %1").arg(path));
        }
        return;
    }

    // Set currentContextIndex according to senderView
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

// Get the file association application path
QString MainWindow::getAssociatedSW(const QString& path)
{
    QFileInfo fi(path);
    QString ext = fi.suffix();

    // Get associated programs from the registry
    QSettings regExt(QString("HKEY_CLASSES_ROOT\\.%1").arg(ext), QSettings::NativeFormat);
    QString typeName = regExt.value("Default").toString();

    QSettings regCmd(QString("HKEY_CLASSES_ROOT\\%1\\shell\\open\\command").arg(typeName), QSettings::NativeFormat);
    QString cmd = regCmd.value("Default").toString();

    // Parsing command templates
    cmd.replace("\"%1\"", "\"%2\"").replace("%1", "%2");
    cmd = cmd.arg(fi.absoluteFilePath());

    // Extractor program path
    QRegularExpression exeRegex("^\"?(.+?\\.exe)");
    QRegularExpressionMatch match = exeRegex.match(cmd);
    return match.hasMatch() ? match.captured(1) : "";
}

// Run the program with elevated privileges
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
            qDebug() << "User canceled the UAC prompt";
        return false;
    }
    return true;
}

// Copy the entire directory for zipping and pasting
void MainWindow::copyDirectory(const QString& srcPath, const QString& dstPath)
{
    QDir srcDir(srcPath);
    if (!srcDir.exists()) return;

    QDir dstDir(dstPath);
    if (!dstDir.exists())
        QDir().mkpath(dstPath);

    // List all files and subdirectories
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

// Clear Clipping Status
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

// Refresh
void MainWindow::refreshInterface()
{
    refreshView();
}

// Open
void MainWindow::openFile()
{
    if (!contextMenuView || !contextMenuIndex.isValid()) return;

    openAsAdmin = false;

    if (contextMenuView == leftView)
        leftView_doubleClicked(contextMenuIndex);
    else if (contextMenuView == fileView)
        fileView_doubleClicked(contextMenuIndex);
}

// Run in Administrator
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

// Open with
void MainWindow::openWith(const QString& application)
{
    // If there is a state like "cut", clean it up first
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
            "Select a program to open the file with",
            startDir,
            "All files (*)"
        );

        if (selectedApp.isEmpty()) return;
        appToRun = selectedApp;
    }
    else appToRun = application;

    // Start with QProcess and pass in the current file path
    QProcess::startDetached(appToRun, QStringList() << path);
}

// Compress
// If has 7z environment locally, use 7z to complete all the compression
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
    dlg.setWindowTitle("Reanme");
    dlg.setLabelText("Enter New Name：");
    dlg.setTextValue(fi.fileName());
    dlg.setOkButtonText("OK");
    dlg.setCancelButtonText("Cancel");

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
        QMessageBox::warning(p, "Error", "Please select a file or folder before compressing.");
        return;
    }

    QString srcPath = fileModel->filePath(contextMenuIndex);
    if (srcPath.isEmpty())
    {
        QMessageBox::warning(p, "Error", "Unable to get the path of the selected item.");
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
            QMessageBox::warning(p, "Missing environment", "7z environment not detected.");
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
        QMessageBox::warning(p, "Unsupported format", QString("Unsupported format : %1").arg(format));
        return;
    }

    if (m_progressDialog)
    {
        m_progressDialog->close();
        m_progressDialog->deleteLater();
        m_progressDialog = nullptr;
    }

    m_progressDialog = new QProgressDialog(
        QString("Compressing %1 …").arg(QFileInfo(dstPath).fileName()),
        "Cancel", 0, 100, p);

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

    // 7z/zip/tar branch, using 7z
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

    // PowerShell and system tar branching with pseudo-progress
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
        m_fakeProgressTimer->start(50);    // The progress bar will be full in about 5 seconds
    }

    // End processing
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
                QMessageBox::information(p, "Finished", QString("Compressed file : \n%1").arg(dstPath));
                renameCompressions(dstPath);
            }
            else
                QMessageBox::warning(p, "Failed", QString("Compression failed (Exit Code %1)").arg(exitCode));

            m_compressProc->deleteLater();
            m_compressProc = nullptr;
        });

    m_progressDialog->setValue(0);
    m_compressProc->start(program, args);
}

// Add to favorites
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

    // Get the path of the currently selected file/folder
    QModelIndex index = leftView->currentIndex();
    if (!index.isValid()) return;

    QString path = fileModel->filePath(index);

    // Pop up a dialog box to let the user select the collection category to be removed
    QInputDialog inputDialog(fDialog);
    inputDialog.setWindowTitle("Remove collection");
    inputDialog.setLabelText("Please select the category to remove from : ");
    inputDialog.setComboBoxItems(categories);
    inputDialog.setComboBoxEditable(false);
    // Use the same font settings
    inputDialog.setFont(font);
    inputDialog.resize(400, 300);

    if (inputDialog.exec() != QDialog::Accepted)
        return;

    QString category = inputDialog.textValue();
    if (category.isEmpty())
        return;

    // Remove from favorites and save to file
    // removeAll will return the number of elements removed
    int removed = favorites[category].removeAll(path);
    if (removed > 0)
    {
        saveFavoritesToFile();
        QMessageBox::information(
            fDialog,
            "Remove collection",
            QString("Successfully removed path \"%1\" from the “%2” collection.").arg(path, category)
        );
    }
    else
    {
        QMessageBox::information(
            fDialog,
            "Remove collection",
            QString("Path \"%1\" is not in “%2” collection.").arg(path, category)
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
        QMessageBox::information(p, "Add to favorites", QString("Successfully added path \"%1\" to the “%2” collection.").arg(path, category));
    }
    else
        QMessageBox::information(p, "Add to favorites", QString("Path \"%1\" already exists in “%2” collection.").arg(path, category));
}

void MainWindow::addToFavorites()
{
    loadFavoritesFromFile();

    // Get all selected items (only take column==0 to prevent duplication)
    QModelIndexList selected = contextMenuView->selectionModel()->selectedIndexes();
    QSet<QString> uniquePaths;
    for (const QModelIndex& idx : selected)
    {
        if (idx.column() == 0)
            uniquePaths.insert(fileModel->filePath(idx));
    }
    if (uniquePaths.isEmpty()) return;

    // A dialog box pops up to select a favorite category
    QWidget* p = (contextMenuView == fileView && favoritesDialogPtr)
                 ? static_cast<QWidget*>(favoritesDialogPtr)
                 : this;

    QInputDialog inputDialog(p);
    inputDialog.setWindowTitle("Add to favorites");
    inputDialog.setLabelText("Please select collection category : ");
    inputDialog.setComboBoxItems(categories);
    inputDialog.setComboBoxEditable(false);

    // Set a larger font
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
        // Add all selected files/folders in batches
        for (const QString& path : uniquePaths)
        {
            collect(category, path);
        }
    }
}

// Create new
void MainWindow::createNew(const QString& type)
{
    // Get the current directory
    QModelIndex currIdx = contextMenuView->rootIndex();
    QString dirPath = fileModel->filePath(currIdx);
    if (dirPath.isEmpty() || !QDir(dirPath).exists()) return;

    // Determine the suffix, default name, and whether to use COM based on the type
    QString suffix, defaultName;
    bool useCom = false;

    if      (type == "directory") { defaultName = "New folder"; }
    else if (type == "image")     { suffix = ".png";  defaultName = "New image"; }
    else if (type == "txt")       { suffix = ".txt";  defaultName = "New text document"; }
    else if (type == "docx")      { suffix = ".docx"; defaultName = "New Word document"; useCom = true; }
    else if (type == "pdf")       { suffix = ".pdf";  defaultName = "New PDF document"; }
    else if (type == "pptx")      { suffix = ".pptx"; defaultName = "New PPT document"; useCom = true; }
    else if (type == "xlsx")      { suffix = ".xlsx"; defaultName = "New Excel table"; useCom = true; }
    else                          { return; }

    // Pop up input box to get the user's file
    QWidget* p = (contextMenuView == fileView && favoritesDialogPtr)
                 ? static_cast<QWidget*>(favoritesDialogPtr)
                 : this;

    QInputDialog dlg(p);
    dlg.setWindowTitle("New");
    dlg.setLabelText("Enter name : ");
    dlg.setTextValue(defaultName);
    dlg.setOption(QInputDialog::NoButtons, false);
    dlg.setOkButtonText("OK");
    dlg.setCancelButtonText("Cancel");

    dlg.setFixedSize(400, 300);
    font.setFamilies(QStringList() << "Microsoft YaHei" << "Malgun Gothic" << "Segoe UI");
    font.setPointSize(10);
    dlg.setFont(font);

    // Pop up dialog box
    if (dlg.exec() != QDialog::Accepted) return;

    // Get input and remove leading and trailing whitespace
    QString userInput = dlg.textValue().trimmed();
    if (userInput.isEmpty()) return;

    // Remove unnecessary suffixes and prepare finalName/fullPath
    QFileInfo inputInfo(userInput);
    QString baseName  = inputInfo.completeBaseName();    // Support multiple extensions
    QString finalName = (type == "directory")
                        ? baseName
                        : baseName + suffix;

    QString fullPath;
    int counter = 1;

    // Guaranteed no duplication of names
    do
    {
        fullPath = QDir(dirPath).filePath(finalName);
        if (!QFileInfo::exists(fullPath)) break;
        finalName = QString("%1(%2)%3")
                        .arg(baseName)
                        .arg(counter++)
                        .arg(suffix);
    } while (true);

    // Unified asynchronous creation for all types
    auto createFunc = [=]() -> bool {
        // Directory: Synchronous creation
        if (type == "directory")
            return QDir(dirPath).mkdir(finalName);

        // Ordinary files: Synchronous creation
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

        // Office files: Create asynchronously to avoid blocking
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
            QMessageBox::warning(p, "Error", QString("Unable to create : \n%1").arg(fullPath));
        }
        else
        {
            qDebug() << "Create Successfully : " << fullPath;
            //refreshView();
        }
    });
    watcher->setFuture(QtConcurrent::run(createFunc));
}

// Create new folder
void MainWindow::createDirectory()
{
    QString type = "directory";
    createNew(type);
}

// Copy file path
void MainWindow::copyFilePath()
{
    //QModelIndex idx = leftView->currentIndex();
    //if (!idx.isValid()) return;
    QString path = fileModel->filePath(contextMenuIndex);
    QApplication::clipboard()->setText(path);

    QWidget* p = (contextMenuView == fileView && favoritesDialogPtr)
                 ? static_cast<QWidget*>(favoritesDialogPtr)
                 : this;

    QToolTip::showText(QCursor::pos(), QString("Copied : %1").arg(path), p);
}

// Create shortcuts across platforms
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

// Create shortcut
void MainWindow::createShortCut()
{
    QWidget* p = (contextMenuView == fileView && favoritesDialogPtr)
                 ? static_cast<QWidget*>(favoritesDialogPtr)
                 : this;

    clearCutStateIfNeeded();
    if (!contextMenuIndex.isValid())
    {
        QMessageBox::warning(p, "Failed", "No files or folders selected");
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
        QMessageBox::warning(p, "Failed", "Unable to create shortcut");
#elif defined(Q_OS_LINUX)
    linkPath = QDir(dir).filePath(name + ".desktop");
    if (!createLinuxDesktopFile(targetPath, linkPath))
        QMessageBox::warning(p, "Failed", "Unable to create desktop shortcut");
#elif defined(Q_OS_MAC)
    linkPath = QDir(dir).filePath(name + ".alias");
    if (!createMacAlias(targetPath, linkPath))
        QMessageBox::warning(p, "Failed", "Unable to create alias");
#endif

    // Refresh view
    updateRightView();
}

// Cut
bool MainWindow::isFileAvailableForCut(const QString& path, QString& errorDetail)
{
    QFileInfo fi(path);
    if (!fi.exists())
    {
        errorDetail = "File does not exist";
        return false;
    }
    if (!fi.isWritable())
    {
        errorDetail = "No write permission";
        return false;
    }
    if (fi.isFile())
    {
        QFile file(path);
        // First determine whether it can be opened in write mode
        if (!file.open(QIODevice::ReadWrite))
        {
            // Determine whether it is a permission issue or is occupied
            // Try opening it read-only first
            if (file.open(QIODevice::ReadOnly))
            {
                errorDetail = "The file is occupied by another program";
                file.close();
            }
            else
                errorDetail = "The file is occupied or has no permission (the specific reason is unknown)";
            return false;
        }
        file.close();
    }
    if (fi.isSymLink() && !QFileInfo(fi.symLinkTarget()).exists())
    {
        errorDetail = "Symbolic link target unreachable";
        return false;
    }
    // Check for network/hardware issues (such as unreachable paths, unavailable drive letters)
#ifdef Q_OS_WIN
    if (fi.absoluteFilePath().startsWith("\\\\") && !fi.isReadable())
    {
        errorDetail = "The network path is not accessible";
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

        // Determines whether the favorites file view is opening the folder
        if (favoritesDialogPtr && fileView->isVisible() && QFileInfo(path).isDir() && mainPath == path)
        {
            QMessageBox::warning(p, "Unabel to cut", "The folder is being used by Favorites and cannot be cut.");
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
            msg += QString("\nName : %1\n\nType : %2\n\nModified time : %3\n\nPath : %4\n\nError : %5\n")
                .arg(fi.fileName())
                .arg(fi.isDir() ? "Folder" : "File")
                .arg(fi.lastModified().toString("yyyy-MM-dd hh:mm:ss"))
                .arg(fi.absoluteFilePath())
                .arg(errorDetails[i]);
        }
        QMessageBox::warning(p, "Unable to cut", msg);
        return;
    }

    // All can be cut, execute cutting
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

// Copy
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

// Paste
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
                    // Only ensure that the directory exists, do not delete it as a whole
                    QDir().mkpath(dstFile);
                }
                else
                {
                    // Overwrite only if the target file exists and the source and target file paths are different
                    if (QFileInfo::exists(dstFile) && srcFile != dstFile && m_toOverwriteFiles.contains(dstFile))
                        QFile::remove(dstFile);
                    QFile srcF(srcFile);
                    QFile dstF(dstFile);
                    if (!srcF.open(QIODevice::ReadOnly) || !dstF.open(QIODevice::WriteOnly))
                    {
                        emit finished(false, QString("Unable to open file : %1").arg(srcFile));
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
            // Overwrite only if the target file exists and the source and target file paths are different
            if (QFileInfo::exists(m_dsts[i]) && m_srcs[i] != m_dsts[i] && m_toOverwriteFiles.contains(m_dsts[i]))
                QFile::remove(m_dsts[i]);
            QFile srcF(m_srcs[i]);
            QFile dstF(m_dsts[i]);
            if (!srcF.open(QIODevice::ReadOnly) || !dstF.open(QIODevice::WriteOnly))
            {
                emit finished(false, QString("Unable to open file : %1").arg(m_srcs[i]));
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

    // Target Directory
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

    // Whether to cover
    if (!fileConflicts.isEmpty())
    {
        QString info = fileConflicts.join("\n");
        auto btn = QMessageBox::question(p, "File overwrite confirmation",
            tr("There is file with same name : \n%1\n overwrite? ").arg(info),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (btn != QMessageBox::Yes)
            return;
    }
    if (!dirConflicts.isEmpty())
    {
        QString info = dirConflicts.join("\n");
        auto btn = QMessageBox::question(p, "Folder overwrite confirmation",
            tr("There is folder with same name : \n%1\n overwrite？").arg(info),
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

    // Start asynchronous replication, passing the files and folders that need to be overwritten
    QThread* thread = new QThread;
    FileCopyTask* worker = new FileCopyTask(srcs, dsts, totalSize, toOverwriteFiles, toOverwriteDirs);
    worker->moveToThread(thread);

    QDialog* dialog = nullptr;
    QProgressBar* progressBar = nullptr;
    QLabel* label = nullptr;

    if (totalSize > 4.7 * 1024 * 1024 * 1024)
    {
        dialog = new QDialog(p);
        dialog->setWindowTitle("Pasting...");
        dialog->setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
        dialog->setWindowModality(Qt::NonModal);
        dialog->setFixedSize(400, 300);

        QVBoxLayout* layout = new QVBoxLayout(dialog);
        label = new QLabel("Pasting, please wait...");
        label->setWordWrap(true);    // Allow line breaks
        label->setAlignment(Qt::AlignCenter); // Center Align
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
            // Truncate long file names
            QString displayName = filename;
            if (displayName.length() > 70)
                displayName = displayName.left(40) + "..." + displayName.right(10);

            label->setText(QString("Pasting %1, please wait...").arg(filename));
            /*dialog->setFixedHeight(170);
            dialog->adjustSize();
            if (dialog->width() > 800)
                dialog->resize(800, dialog->width());*/
        });
    }
    else if (totalSize > 1 * 1024 * 1024 * 1024)
    {
        dialog = new QDialog(p);
        dialog->setWindowTitle("Pasting...");
        dialog->setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
        dialog->setWindowModality(Qt::NonModal);
        dialog->setFixedSize(400, 300);

        QVBoxLayout* layout = new QVBoxLayout(dialog);
        label = new QLabel("Pasting, please wait...");
        label->setWordWrap(true);
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget(label);

        dialog->show();

        connect(worker, &FileCopyTask::currentFile, this, [=](const QString& filename) {
            QString displayName = filename;
            if (displayName.length() > 70)
                displayName = displayName.left(40) + "..." + displayName.right(10);

            label->setText(QString("Pasting %1, please wait...").arg(filename));
        });
    }

    connect(thread, &QThread::started, worker, &FileCopyTask::doCopy);
    connect(worker, &FileCopyTask::finished, this, [=](bool success, const QString &err) {
        if (dialog) dialog->close();
        if (!success) QMessageBox::warning(this, "Paste failed : ", err);
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
            if (!success) QMessageBox::warning(this, "Paste failed : ", err);
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

    // Clear traces after pasting
    clipPaths.clear();
    rightClickMenu->isCopy = false;
}

// Delete
void MainWindow::fileDeleteError(const QString& path, const QString& errorMsg, QFileInfo info)
{
    QWidget* p = (contextMenuView == fileView && favoritesDialogPtr)
                 ? static_cast<QWidget*>(favoritesDialogPtr)
                 : this;

    // Construction details
    QString details = QString("Name : %1\nSuffix : %2\nMomodified time : %3\nPath : %4")
        .arg(info.fileName())
        .arg(info.suffix())
        .arg(info.lastModified().toString("yyyy-MM-dd hh:mm:ss"))
        .arg(info.absoluteFilePath());

    QDialog dlg(p);
    dlg.setWindowTitle("Deletion failed");
    QVBoxLayout* layout = new QVBoxLayout(&dlg);

    QLabel* label = new QLabel(QString("%1\n\n%2").arg(errorMsg, details));
    layout->addWidget(label);

    QCheckBox* applyAllBox = new QCheckBox("Automatically perform this operation on subsequent files of the same type");
    layout->addWidget(applyAllBox);

    QHBoxLayout* btnLayout = new QHBoxLayout;
    QPushButton* retryBtn = new QPushButton("Retry");
    QPushButton* skipBtn = new QPushButton("Skip");
    QPushButton* cancelBtn = new QPushButton("Cancel");
    btnLayout->addWidget(retryBtn);
    btnLayout->addWidget(skipBtn);
    btnLayout->addWidget(cancelBtn);
    layout->addLayout(btnLayout);

    int userAction = -1;
    QObject::connect(retryBtn, &QPushButton::clicked, [&]() { userAction = 0; dlg.accept(); });
    QObject::connect(skipBtn, &QPushButton::clicked, [&]() { userAction = 1; dlg.accept(); });
    QObject::connect(cancelBtn, &QPushButton::clicked, [&]() { userAction = 2; dlg.accept(); });

    dlg.exec();

    // Signals return of user selection
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
    // Without FOF_ALLOWUNDO, completely delete, pop up native conflict/error dialog
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
        int userAction = 0;    // 0=retry, 1=skip, 2=cancel
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

            // Success, continue to the next file
            if (ok) break;

            // Failure, handling conflict
            if (applyToAll)
                userAction = lastAction;
            else
            {
                // Send a signal to the main thread to pop up a dialog box
                // Block waiting for the user to select
                QEventLoop loop;
                int result = 0;
                bool applyAll = false;
                emit fileDeleteError(p, "Failed to delete file/folder", fi);

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

            // 0 = retry, 1 = skip, 2 = cancel
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

        // Progress Statistics
        if (ok && fi.isFile()) deleted += fi.size();
        emit progress(deleted, m_total);
        QCoreApplication::processEvents();
    }
    emit finished(allOk, allOk ? QString() : QString("Failed to delete some files/folders"));
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

    // Files smaller than 1GB will be deleted synchronously
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
            QMessageBox::warning(p, tr("Operation failed"), tr("Some options cannot be moved to the Trash"));
        //else
            //updateRightView();
        return;
    }

    // Asynchronous deletion of files larger than 1GB
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
        dialog->setWindowTitle("Moving to Trash...");
        dialog->setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
        dialog->setWindowModality(Qt::NonModal);
        dialog->setFixedSize(400, 300);

        QVBoxLayout* layout = new QVBoxLayout(dialog);
        label = new QLabel("Moving to Trash...");
        label->setWordWrap(true);    // Allow line breaks
        label->setAlignment(Qt::AlignCenter);    // Center Align
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
        dialog->setWindowTitle("Moving to Trash...");
        dialog->setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
        dialog->setWindowModality(Qt::NonModal);
        dialog->setFixedSize(400, 300);

        QVBoxLayout* layout = new QVBoxLayout(dialog);
        label = new QLabel("Moving to Trash...");
        label->setWordWrap(true);
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget(label);

        dialog->show();
    }

    // Display current file name in real time
    connect(worker, &FileDeleteTask::currentFile, this, [=](const QString& filename) {
        // Truncate long file names
        QString displayName = filename;
        if (displayName.length() > 70)
            displayName = displayName.left(40) + "..." + displayName.right(10);

        label->setText(QString("Moving to Trash : %1").arg(filename));
        /*dialog->setFixedHeight(170);
        dialog->adjustSize();
        if (dialog->width() > 800)
            dialog->resize(800, dialog->width());*/
    });

    connect(thread, &QThread::started, [=]() { worker->doDelete(true); });
    connect(worker, &FileDeleteTask::finished, this, [=](bool success, const QString &err) {
        if (dialog) dialog->close();
        if (!success) QMessageBox::warning(p, "Operation failed", err);
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
        if (errStr.contains("used by another process", Qt::CaseInsensitive) ||
            errStr.contains("sharing violation", Qt::CaseInsensitive))
            return "The file is occupied (a program is using it)";
#endif
        if (errStr.contains("Permission denied", Qt::CaseInsensitive))
            return "No permission";
        if (errStr.contains("No such file or directory", Qt::CaseInsensitive))
            return "File or folder does not exist";

        int err = errno;
        if (err == 32 || err == 33)
            return "The file is occupied (a program is using it)";
        else if (err == 13 || err == 5)
            return "No permission";
        else if (err == 2)
            return "File or folder does not exist";
        else if (!errStr.isEmpty())
            return QString("Unable to delete, system error message : %1").arg(errStr);
        else
            return QString("Unable to delete, system error code%1 : %2").arg(err).arg(strerror(err));
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

    // Collect the paths of files/folders to be deleted uniformly
    QStringList filesToDelete;
    if (paths.isEmpty())
    {
        // Delete Selected Items
        QModelIndexList selected = contextMenuView->selectionModel()->selectedIndexes();
        QSet<QString> uniquePaths;
        for (const QModelIndex& idx : selected)
        {
            if (idx.column() == 0)
            {
                QString path;
                if (contextMenuView == fileView && fileView)
                {
                    // Favorites file view: Get the path based on the current model
                    path = fileModel->filePath(idx);
                }
                else if (contextMenuView == leftView || contextMenuView == rightView)
                {
                    // Main program view
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

    // Statistics
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

    // Non-modal confirmation popup
    if (needConfirm)
    {
        QString info = names.join(", ");
        QMessageBox* box = new QMessageBox(QMessageBox::Question,
                                           "Delete",
                                           QString("Confirm deletion of the following items : \n%1 ?").arg(info),
                                           QMessageBox::Yes | QMessageBox::No,
                                           p);
        connect(box, &QMessageBox::finished, this, [=](int result){
            if (result == QMessageBox::Yes)
            {
                // After the user confirms, execute asynchronous deletion
                this->startAsyncDelete(filesToDelete, totalSize);
            }
            box->deleteLater();
        });
        box->show();
        return;
    }

    // If confirmation is not required, delete directly
    startAsyncDelete(filesToDelete, totalSize);
}

// Actual implementation of asynchronous deletion
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
        dialog->setWindowTitle("Deleting...");
        dialog->setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
        dialog->setWindowModality(Qt::NonModal);
        dialog->setFixedSize(400, 300);

        QVBoxLayout* layout = new QVBoxLayout(dialog);
        label = new QLabel("Deleting...");
        label->setWordWrap(true);    // Allow word wrapping
        label->setAlignment(Qt::AlignCenter);    // Center Align
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
        dialog->setWindowTitle("Deleting...");
        dialog->setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
        dialog->setWindowModality(Qt::NonModal);
        dialog->setFixedSize(400, 300);

        QVBoxLayout* layout = new QVBoxLayout(dialog);
        label = new QLabel("Deleting...");
        label->setWordWrap(true);
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget(label);

        dialog->show();
    }

    // Display deleted file names in real time
    connect(worker, &FileDeleteTask::currentFile, this, [=](const QString& filename) {
        // Truncate long file names
        QString displayName = filename;
        if (displayName.length() > 70)
            displayName = displayName.left(40) + "..." + displayName.right(10);

        label->setText(QString("Deleting : %1").arg(filename));
        /*dialog->setFixedHeight(170);
        dialog->adjustSize();
        if (dialog->width() > 800)
            dialog->resize(800, dialog->width());*/
    });

    connect(thread, &QThread::started, [=]() { worker->doDelete(false); });
    connect(worker, &FileDeleteTask::finished, this, [=](bool success, const QString &err) {
        if (dialog) dialog->close();
        if (!success) QMessageBox::warning(p, "Deletion failed", err);
        //else updateRightView();

        thread->quit();
        thread->wait();
        worker->deleteLater();
        thread->deleteLater();
        if (dialog) dialog->deleteLater();
    });
    thread->start();
}

// Rename
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
    dlg.setWindowTitle("Rename");
    dlg.setLabelText("Enter new name : ");
    dlg.setTextValue(fi.fileName());
    dlg.setOkButtonText("OK");
    dlg.setCancelButtonText("Cancel");

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
            if (!QFile::rename(oldPath, newPath)) QMessageBox::warning(this, "Rename failed", "Unable to rename");
            //else updateRightView();
        }
    }
}

// Property
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
            qDebug() << "ShellExecuteExW failed with error : " << err;
        }
        CoUninitialize();
    }
    else
        qDebug() << "CoInitialize failed with HRESULT : " << hr;
}

// File preview
void MainWindow::previewFile(const QModelIndex& index)
{
    if (isPreview)
    {
        QString filePath = fileModel->filePath(index);
        QFileInfo fileInfo(filePath);

        // If it is not a file, return directly
        if (!fileInfo.isFile()) return;

        QString suffix = fileInfo.suffix().toLower();

        // If the preview type is not supported, return directly
        if (suffix != "txt" &&
            suffix != "png" && suffix != "jpg" &&
            suffix != "jpeg" && suffix != "bmp")
        {
            return;
        }

        // If there is a preview box open, close it first
        if (currentPreviewDialog)
        {
            currentPreviewDialog->close();
            currentPreviewDialog->deleteLater();
            currentPreviewDialog = nullptr;
        }

        // Create a new preview dialog
        currentPreviewDialog = new QDialog(this);
        currentPreviewDialog->setAttribute(Qt::WA_DeleteOnClose);    // Automatically destroyed when closed
        currentPreviewDialog->setWindowTitle("Preview : " + fileInfo.fileName());
        QVBoxLayout* layout = new QVBoxLayout(currentPreviewDialog);

        // When the dialog is destroyed, reset the currentPreviewDialog pointer
        connect(currentPreviewDialog, &QObject::destroyed, this, [this](){
            currentPreviewDialog = nullptr;
        });

        // Create different controls based on file type
        if (suffix == "txt")
        {
            QTextEdit* textEdit = new QTextEdit(currentPreviewDialog);
            textEdit->setReadOnly(true);
            // Set a larger font
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
                textEdit->setText("Unable to open file! ");
            layout->addWidget(textEdit);
        }
        else if (suffix == "png" || suffix == "jpg" ||
                 suffix == "jpeg" || suffix == "bmp")
        {
            // Use a custom ScalableImageLabel to display images
            ScalableImageLabel* imgLabel = new ScalableImageLabel(currentPreviewDialog);
            QPixmap pixmap(filePath);
            if (pixmap.isNull())
                imgLabel->setText("Unable to load image! ");
            else
                imgLabel->setPixmap(pixmap);
            layout->addWidget(imgLabel);
        }

        currentPreviewDialog->setLayout(layout);
        currentPreviewDialog->resize(800, 600);
        currentPreviewDialog->show();
    }
}
