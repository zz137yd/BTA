#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QThreadPool>
#include <QDebug>

// Layout
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>

// Style
#include <QFont>
#include <QFontMetrics>
#include <QIcon>


// ***  Above the file area  ***


// File Operations
#include <QFileSystemModel>
#include <QModelIndex>
#include <QHeaderView>
#include <QAbstractItemView>
#include <QStringList>
#include <QSettings>
#include <QDesktopServices>
#include <QUrl>

// Forward, Back, Refresh buttons
#include <QPushButton>
#include <QStack>

// Path buttons
#include <QToolButton>
#include <QVector>

// Button for copying path
#include <QPushButton>
#include <QClipboard>
#include <QToolTip>
#include <QCursor>

// Collect
#include <QStandardItemModel>
#include <QStackedWidget>
#include <QListWidget>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

// File Sort
#include <QComboBox>
#include <QMap>

// Search
#include <QLineEdit>
#include <QStandardItemModel>
#include <QFutureWatcher>


// ***  Software Features  ***


// Include utils
#include "Utils.h"

// Size Progress Bar
#include "OccupancyDisplay.h"

// Right-click Menu
#include "RightClickMenu.h"
#include <QAxObject>
#include <QFileInfo>
#include <QtConcurrent>
#include <QProcess>
#include <QProgressDialog>
#include <QShortcut>
#include <QFutureWatcher>
#include <QPdfWriter>
#include <QBrush>

// File Preview
#include <QDialog>
#include <QTextEdit>
#include <QFile>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MyFileSystemModelEx;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    // Whether it is in shear state
    bool isCut() const { return clipIsCut; }

    // Specifies whether the path is in current cut list
    bool isClipped(const QString& path) const { return clipPaths.contains(path); }

    // Auxiliary function for delete
    static bool moveToRecycleBin(const QString& path);

private slots:
    // Prefix function
    void updateStack(const QString& path, QStack<QString>& stack);
    void updateButtons(QVector<QToolButton*>& bs, QStack<QString>& stack);
    void updateRightView();

    // Double-click to open folders and files
    void leftView_doubleClicked(const QModelIndex& index);

    // Froward, Back buttons
    void backClicked();
    void forwardClicked();

    // Refresh button
    void refreshView();

    // Path button
    void pathClicked();

    // Copy Path
    void copyPath();

    // Whether to enable preview
    void preview();

    // Dark Mode
    void changeDark();

    // Collect
    void onlyPath();

    void favoriteItems(QListWidgetItem* item);
    void favoritesContextMenu(const QPoint& pos);

    void upClicked();
    void downClicked();

    void fileView_doubleClicked(const QModelIndex& index);
    void fPathClicked();

    void fSearch();
    void handleFSearchFinished();

    void favoritesDialog(const QString& category);

    // File Sort
    void fileSort();

    // Search
    void search();
    void handleSearchFinished();

    // Right-click Menu
    void refreshInterface();
    void showContextMenu(const QPoint& pos);

    void openFile();
    void administratorOpen();

    void openWith(const QString& application);
    void Compressions(const QString& format);

    void addToFavorites();
    void createNew(const QString& type);
    void createDirectory();

    void copyFilePath();
    void createShortCut();
    void cut();
    void copy();
    void paste();
    void toRecyleBin();
    void del();
    void rename();
    void properties();

    // File Preview
    void previewFile(const QModelIndex& index);

private:
    Ui::MainWindow *ui;

    // Style
    // view's overall style
    void interfaceStyle();
    // Adjust the displayed columns
    void viewAdjust();
    QByteArray savedHeaderState;

    int buttonHeight = 47;
    int buttonWidth = 147;
    int fontSize = 11;

    // Centerpiece
    QWidget* centralWidget;

    // File System
    QFileSystemModel* fileModel;
    QTreeView* leftView;
    QTreeView* rightView;
    SizeProgressDelegate* sizeDelegate;

    // Font
    QFont font;

    // Record file path
    QStack<QString> pathStack;  // Record the segmented path to assign to path button
    QString rootPath = "";
    QString filePath;
    QString leftPath;
    QString rightPath;

    // Froward, Back buttons
    QPushButton* backButton;
    QPushButton* forwardButton;
    // Used to control the effectiveness of forward button (only forward after backing)
    bool backFlag = false;

    // Refresh button
    QPushButton* refreshButton;

    // Path button
    QVector<QToolButton*> buttons;
    const int defaultPathButtonWidth = 70;
    const int maxPathButtonWidth = 400;
    const int moreWidth = 30;

    // Button for copying path
    QPushButton* copyPathButton;
    QClipboard* clipboard;          // Save the copied path

    //　Dark mode
    QPushButton* darkButton;
    bool isDarkMode = false;

    // Favorites drop-down box
    QPushButton* fButton;
    QComboBox* collectComboBox;
    QMap<QString, QStringList> favorites;

    // Favorites
    bool isOnlyPath = false;
    QHBoxLayout* funcLayout;
    QVBoxLayout* mainLayout;
    SizeProgressDelegate* fSizeDelegate;

    QString fCategory;
    QLabel* fLabel;
    QFont fdFont;

    QPushButton* upButton;
    QPushButton* downButton;
    QVector<QToolButton*> fButtons;
    // Make upButton ineffective in the favorites list
    NoClickFilter* buttonFilter = new NoClickFilter(this);

    // Two views of storage: favorites list and file view
    QStackedWidget* fStack;
    QListWidget* favoritesList;
    QTreeView* fileView;

    // Update the favorites path
    QStack<QString> fPathStack;
    QString initialPath = "";
    QString mainPath = "";
    QString fPath = "";
    QModelIndex fIdx;
    bool upFlag = false;

    // Search in Favorites
    QLineEdit* fSearchEdit;
    QStandardItemModel* fSearchModel;
    QFutureWatcher<QStringList> fSearchWatcher;
    bool isFSearch = false;

    void favoritesView();

    void reloadFavoritesList();
    void moveFavorite(QListWidgetItem* item);
    void deleteFavorite(QListWidgetItem* item);

    // File sort
    QComboBox* sortComboBox;
    QMap<QString, int> sortMap;     // All sorting methods
    QString currentSort;            // Current sorting method
    Qt::SortOrder currentOrder;     // Descending or ascending

    // Search
    QLineEdit* searchEdit;

    QStandardItemModel* searchModel;
    QFutureWatcher<QStringList> searchWatcher;
    bool isSearch = false;

    // Right-click menu
    RightClickMenu* rightClickMenu;
    QStringList clipPaths;
    QModelIndex currentContextIndex;
    QString currentContextPath;

    QString m_7zPath;
    QProcess* m_compressProc = nullptr;
    QProgressDialog* m_progressDialog = nullptr;
    QTimer* m_fakeProgressTimer = nullptr;
    int m_fakeProgressValue = 0;

    bool openAsAdmin = false;       // Whether it is administrator mode
    bool clipIsCut = false;         // Whether it is in the cutting state
    QStringList categories;

    // Get the file association application path
    QString getAssociatedSW(const QString& path);
    // Run the program with elevated privileges
    bool runAsAdmin(const QString& path, const QString& params);

    // Copy the entire directory
    void copyDirectory(const QString& srcPath, const QString& dstPath);
    // Clear Clipping Status
    void clearCutStateIfNeeded();

    // Compression method
    QString find7z();
    void renameCompressions(const QString& path);

    // Cross-platform shortcut creation
    bool createWindowsShortcut(const QString& targetPath, const QString& linkPath);
    bool createLinuxDesktopFile(const QString& targetPath, const QString& linkPath);
    bool createMacAlias(const QString& targetPath, const QString& linkPath);

    // Collect
    void saveFavoritesToFile();
    void deleteFileFromFavorites();
    void loadFavoritesFromFile();
    void collect(const QString& category, const QString& path);

    // Cross-platform removal
    //bool moveToTrash(const QString& path);        // macOS/Linux

    // File preview
    QDialog* currentPreviewDialog = nullptr;
    QPushButton* previewButton;
    bool isPreview;

    // Layout
    QHBoxLayout* buttonLayout;
    QSplitter* splitter;
    QVBoxLayout* layout;
};

// This model subclass only overrides data() to reflect the gray state of the cut file
class MyFileSystemModelEx : public QFileSystemModel
{
public:
    explicit MyFileSystemModelEx(MainWindow* parent)
        : QFileSystemModel(parent), mw(parent) {}

    QVariant data(const QModelIndex& index, int role) const override
    {
        if (role == Qt::ForegroundRole && mw->isCut())
        {
            QString path = filePath(index);
            if (mw->isClipped(path))
            {
                // In the cut state, the cut files/folders are displayed in light gray.
                return QBrush(Qt::lightGray);
            }
        }
        // In all other cases, the base class is called
        return QFileSystemModel::data(index, role);
    }

private:
    MainWindow* mw;
};

// File copy task
class FileCopyTask : public QObject
{
    Q_OBJECT
public:
    FileCopyTask(const QStringList& srcs, const QStringList& dsts, qint64 total,
                 const QSet<QString>& toOverwriteFiles, const QSet<QString>& toOverwriteDirs)
        : m_srcs(srcs), m_dsts(dsts), m_total(total),
          m_toOverwriteFiles(toOverwriteFiles), m_toOverwriteDirs(toOverwriteDirs) {}

signals:
    void progress(qint64 copied, qint64 total);
    void finished(bool success, const QString& err);
    void currentFile(const QString& filename);

public slots:
    void doCopy();

private:
    QStringList m_srcs, m_dsts;
    qint64 m_total;
    QSet<QString> m_toOverwriteFiles;
    QSet<QString> m_toOverwriteDirs;
};

// File delete task
class FileDeleteTask : public QObject
{
    Q_OBJECT
public:
    FileDeleteTask(const QStringList& paths, qint64 total)
        : m_paths(paths), m_total(total) {}

signals:
    void progress(qint64 deleted, qint64 total);
    void finished(bool success, const QString& err);
    void currentFile(const QString& filename);

public slots:
    void doDelete(bool toRecycleBin);

private:
    QStringList m_paths;
    qint64 m_total;
};

#endif // MAINWINDOW_H
