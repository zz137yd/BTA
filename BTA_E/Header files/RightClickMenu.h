#ifndef RIGHTCLICKMENU_H
#define RIGHTCLICKMENU_H

#include <QObject>
#include <QDebug>

#include <QMenu>
#include <QWidget>
#include <QAction>

#include <QRegularExpression>

// Embed Windows Files
#ifdef Q_OS_WIN
#include <windows.h>
#include <shobjidl.h>
#include <shlguid.h>
#include <shellapi.h>
#endif

// RightClickMenu.cpp
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QDir>
#include <QRegularExpression>

#include <QContextMenuEvent>
#include <QProcess>
#include <QDateTime>

class RightClickMenu : public QMenu
{
    Q_OBJECT
public:
    explicit RightClickMenu(QWidget* parent = nullptr);
    ~RightClickMenu();

    void itemsBlock(const QString& path);       // If necessary, disable some right-click menu items
    void whiteBlock(const QString& path);       // Masking of blank areas
    void cutChecked(bool checked);              // Check if it is in shear state

    bool isCopy;                                // Whether it has been copied
    bool isWhiteSpace;                          // Whether it is in the blank area

    // Get private variables
    QAction* getRefresh() const { return refresh; }
    QAction* getCollections() const { return collections; }
    QAction* getNewDirectory() const { return newDirectory; }
    QAction* getCut() const { return cut; }
    QAction* getCopy() const { return copy; }
    QAction* getPaste() const { return paste; }
    QAction* getRecyleBin() const { return recyleBin; }
    QAction* getDel() const { return del; }
    QAction* getRename() const { return rename; }
    QAction* getProperties() const { return properties; }

signals:
    void refreshRequested();
    void openFileRequested();
    void administratorRequested();

    void openWithRequested(const QString& application);
    void compressionRequested(const QString& format);

    void collectRequested();
    void createNewRequested(const QString& type);
    void createNewDirectoryRequested();

    void copyFilePathRequested();
    void createShortcutRequested();

    void cutRequested();
    void copyRequested();
    void pasteRequested();
    void recyleBinRequested();
    void deleteRequested();
    void renameRequested();
    void propertiesRequested();

public slots:
    void onRefresh();
    void openFile();
    void runInAdministrator();

    void openWith(QAction* action);
    void compressionMethods(QAction* action);

    void addToFavorites();
    void createNew(QAction* action);
    void onCreateDirectory();

    void onCopyFilePath();
    void onCreateShortCut();
    void onCut();
    void onCopy();
    void onPaste();
    void onRecyleBin();
    void onDelete();
    void onRename();
    void onProperties();

private:
    bool isDisk;
    bool isWhiteDisk;

    QAction* refresh;
    QAction* fileOpen;
    QAction* administratorRun;

    QMenu* openWithMenu;
    QAction* notePad;
    QAction* chooseInWindows;

    QMenu* compressionMenu;
    QAction* zip;
    QAction* sevenZ;
    QAction* targz;

    QAction* collections;
    QMenu* newCreations;
    QAction* newDirectory;
    QAction* newImage;
    QAction* newTxt;
    QAction* newDocx;
    QAction* newPdf;
    QAction* newPptx;
    QAction* newXlsx;

    QAction* pathCopy;
    QAction* shortCut;
    QAction* cut;
    QAction* copy;
    QAction* paste;
    QAction* recyleBin;
    QAction* del;
    QAction* rename;
    QAction* properties;
};

#endif // RIGHTCLICKMENU_H
