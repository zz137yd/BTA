#include "RightClickMenu.h"

RightClickMenu::RightClickMenu(QWidget* parent)
    : QMenu(parent)
{
    isDisk = true;
    isWhiteDisk = true;

    refresh = addAction("Refresh");
    fileOpen = addAction("Open");
    administratorRun = addAction("Run in Administrator");

    addSeparator();

    openWithMenu = new QMenu("Open with", this);
    notePad = openWithMenu->addAction("Notepad");
    chooseInWindows = openWithMenu->addAction("Select in windows");

    addMenu(openWithMenu);

    compressionMenu = new QMenu("Compress to", this);
    zip = compressionMenu->addAction("ZIP");
    sevenZ = compressionMenu->addAction("7z");
    targz = compressionMenu->addAction("tar");

    addMenu(compressionMenu);
    addSeparator();

    collections = addAction("To favorites");
    newCreations = new QMenu("New", this);
    newDirectory = new QAction("Folder", this);
    newCreations->addAction(newDirectory);
    newImage = newCreations->addAction("Image(.png)");
    newTxt = newCreations->addAction("Text Document (.txt)");
    newDocx = newCreations->addAction("Word (.docx)");
    newPdf = newCreations->addAction("PDF (.pdf)");
    newPptx = newCreations->addAction("PPT (.pptx)");
    newXlsx = newCreations->addAction("Excel (.xlsx)");
    addMenu(newCreations);

    addSeparator();

    pathCopy = addAction("Copy as a path");
    shortCut = addAction("Create shortcut");

    addSeparator();

    cut = addAction("Cut");
    cut->setCheckable(true);
    cut->setChecked(false);
    copy = addAction("Copy");
    isCopy = false;
    paste = addAction("Paste");
    paste->setVisible(false);

    addSeparator();

    recyleBin = addAction("Delete");
    del = addAction("Permanently Delete");
    rename = addAction("Rename");

    addSeparator();

    properties = addAction("Properties");

    // Shortcut key
    refresh->setShortcut(QKeySequence(Qt::Key_F5));
    collections->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_T));
    cut->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_X));
    copy->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_C));
    recyleBin->setShortcut(QKeySequence(Qt::Key_Delete));
    del->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Delete));
    rename->setShortcut(QKeySequence(Qt::Key_F2));

    // Bind
    connect(refresh, &QAction::triggered, this, &RightClickMenu::onRefresh);
    connect(fileOpen, &QAction::triggered, this, &RightClickMenu::openFile);
    connect(administratorRun, &QAction::triggered, this, &RightClickMenu::runInAdministrator);
    connect(openWithMenu, &QMenu::triggered, this, &RightClickMenu::openWith);
    connect(compressionMenu, &QMenu::triggered, this, &RightClickMenu::compressionMethods);
    connect(collections, &QAction::triggered, this, &RightClickMenu::addToFavorites);
    connect(newCreations, &QMenu::triggered, this, &RightClickMenu::createNew);
    connect(newDirectory, &QAction::triggered, this, &RightClickMenu::onCreateDirectory);
    connect(pathCopy, &QAction::triggered, this, &RightClickMenu::onCopyFilePath);
    connect(shortCut, &QAction::triggered, this, &RightClickMenu::onCreateShortCut);
    connect(cut, &QAction::triggered, this, &RightClickMenu::onCut);
    connect(copy, &QAction::triggered, this, &RightClickMenu::onCopy);
    connect(paste, &QAction::triggered, this, &RightClickMenu::onPaste);
    connect(recyleBin, &QAction::triggered, this, &RightClickMenu::onRecyleBin);
    connect(del, &QAction::triggered, this, &RightClickMenu::onDelete);
    connect(rename, &QAction::triggered, this, &RightClickMenu::onRename);
    connect(properties, &QAction::triggered,this, &RightClickMenu::onProperties);
}

RightClickMenu::~RightClickMenu() {}


void RightClickMenu::itemsBlock(const QString& path)
{
    isDisk = (path == "");

    if (isDisk)
    {
        refresh->setVisible(!isDisk);
        fileOpen->setVisible(isDisk);
        administratorRun->setVisible(isDisk);
        openWithMenu->menuAction()->setVisible(!isDisk);
        compressionMenu->menuAction()->setVisible(!isDisk);
        collections->setVisible(!isDisk);
        newCreations->menuAction()->setVisible(!isDisk);
        pathCopy->setVisible(!isDisk);
        shortCut->setVisible(!isDisk);
        cut->setVisible(!isDisk);
        copy->setVisible(!isDisk);
        paste->setVisible(!isDisk);
        recyleBin->setVisible(!isDisk);
        del->setVisible(!isDisk);
        rename->setVisible(!isDisk);
        properties->setVisible(isDisk);
    }
    else
    {
        refresh->setVisible(isDisk);
        fileOpen->setVisible(!isDisk);
        administratorRun->setVisible(!isDisk);
        openWithMenu->menuAction()->setVisible(!isDisk);
        compressionMenu->menuAction()->setVisible(!isDisk);
        collections->setVisible(!isDisk);
        newCreations->menuAction()->setVisible(!isDisk);
        pathCopy->setVisible(!isDisk);
        shortCut->setVisible(!isDisk);
        cut->setVisible(!isDisk);
        copy->setVisible(!isDisk);
        paste->setVisible(isDisk);
        recyleBin->setVisible(!isDisk);
        del->setVisible(!isDisk);
        rename->setVisible(!isDisk);
        properties->setVisible(!isDisk);
    }

    if (isCopy & !isDisk) paste->setVisible(true);
}

void RightClickMenu::whiteBlock(const QString& path)
{
    isWhiteDisk = (path == "");

    if (isWhiteDisk)
    {
        refresh->setVisible(isWhiteDisk);
        fileOpen->setVisible(!isWhiteDisk);
        administratorRun->setVisible(!isWhiteDisk);
        openWithMenu->menuAction()->setVisible(!isWhiteDisk);
        compressionMenu->menuAction()->setVisible(!isWhiteDisk);
        collections->setVisible(!isWhiteDisk);
        newCreations->menuAction()->setVisible(!isWhiteDisk);
        pathCopy->setVisible(!isWhiteDisk);
        shortCut->setVisible(!isWhiteDisk);
        cut->setVisible(!isWhiteDisk);
        copy->setVisible(!isWhiteDisk);
        paste->setVisible(!isWhiteDisk);
        recyleBin->setVisible(!isWhiteDisk);
        del->setVisible(!isWhiteDisk);
        rename->setVisible(!isWhiteDisk);
        properties->setVisible(!isWhiteDisk);
    }
    else
    {
        refresh->setVisible(!isWhiteDisk);
        fileOpen->setVisible(isWhiteDisk);
        administratorRun->setVisible(isWhiteDisk);
        openWithMenu->menuAction()->setVisible(isWhiteDisk);
        compressionMenu->menuAction()->setVisible(isWhiteDisk);
        collections->setVisible(isWhiteDisk);
        newCreations->menuAction()->setVisible(!isWhiteDisk);
        pathCopy->setVisible(!isWhiteDisk);
        shortCut->setVisible(!isWhiteDisk);
        cut->setVisible(isWhiteDisk);
        copy->setVisible(isWhiteDisk);
        paste->setVisible(isWhiteDisk);
        recyleBin->setVisible(isWhiteDisk);
        del->setVisible(isWhiteDisk);
        rename->setVisible(isWhiteDisk);
        properties->setVisible(!isWhiteDisk);
    }

    if (isCopy & !isWhiteDisk) paste->setVisible(true);
}

void RightClickMenu::cutChecked(bool checked)
{
    if (cut) cut->setChecked(checked);
}

void RightClickMenu::onRefresh()
{
    emit refreshRequested();
}

void RightClickMenu::openWith(QAction* action)
{
    if (action == notePad) emit openWithRequested("notepad.exe");
    else if (action == chooseInWindows) emit openWithRequested("windows");
}

void RightClickMenu::compressionMethods(QAction* action)
{
    if (action == zip) emit compressionRequested("zip");
    else if (action == sevenZ) emit compressionRequested("7z");
    else if (action == targz) emit compressionRequested("tar");
}

void RightClickMenu::createNew(QAction* action)
{
    if (action == newDirectory) emit createNewRequested("directory");
    else if (action == newImage) emit createNewRequested("image");
    else if (action == newTxt) emit createNewRequested("txt");
    else if (action == newDocx) emit createNewRequested("docx");
    else if (action == newPdf) emit createNewRequested("pdf");
    else if (action == newPptx) emit createNewRequested("pptx");
    else if (action == newXlsx) emit createNewRequested("xlsx");
}

void RightClickMenu::onCreateDirectory()     { emit createNewDirectoryRequested();  }
void RightClickMenu::openFile()              { emit openFileRequested();            }
void RightClickMenu::runInAdministrator()    { emit administratorRequested();       }
void RightClickMenu::addToFavorites()        { emit collectRequested();             }
void RightClickMenu::onCopyFilePath()        { emit copyFilePathRequested();        }
void RightClickMenu::onCreateShortCut()      { emit createShortcutRequested();      }

void RightClickMenu::onCut()
{
    emit cutRequested();
    cutChecked(true);
}

void RightClickMenu::onCopy()          { emit copyRequested();        }
void RightClickMenu::onPaste()         { emit pasteRequested();       }
void RightClickMenu::onRecyleBin()     { emit recyleBinRequested();   }
void RightClickMenu::onDelete()        { emit deleteRequested();      }
void RightClickMenu::onRename()        { emit renameRequested();      }
void RightClickMenu::onProperties()    { emit propertiesRequested();  }
