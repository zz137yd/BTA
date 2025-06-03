#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QThreadPool>
#include <QDebug>

// 布局
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>

// 样式
#include <QFont>
#include <QFontMetrics>
#include <QIcon>


// ***  文件区域上方  ***


// 文件操作
#include <QFileSystemModel>
#include <QModelIndex>
#include <QHeaderView>
#include <QAbstractItemView>
#include <QStringList>
#include <QSettings>
#include <QDesktopServices>
#include <QUrl>

// 前进后退、刷新按钮
#include <QPushButton>
#include <QStack>

// 路径按钮
#include <QToolButton>
#include <QVector>

// 复制路径按钮
#include <QPushButton>
#include <QClipboard>
#include <QToolTip>
#include <QCursor>

// 收藏
#include <QStandardItemModel>
#include <QStackedWidget>
#include <QListWidget>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QPointer>

// 文件排序
#include <QComboBox>
#include <QMap>

// 搜索
#include <QLineEdit>
#include <QStandardItemModel>
#include <QFutureWatcher>


// ***  软件功能  ***


// 引入小工具
#include "Utils.h"

// Size进度条
#include "OccupancyDisplay.h"

// 右键菜单
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
// 错误处理
#include <QCheckBox>
#include <cerrno>
#include <cstring>

// 文件预览
#include <QDialog>
#include <QTextEdit>
#include <QFile>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MyFileSystemModelEx;
class FileDeleteTask;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    // 是否处于剪切状态
    bool isCut() const { return clipIsCut; }
    // 指定路径是否在当前剪切列表中
    bool isClipped(const QString &path) const { return clipPaths.contains(path); }

    // 删除的辅助函数
    static bool moveToRecycleBin(const QString& path);   // Windows

private slots:
    // 支持拖拽过来的文件
    void handleExternalDropped(const QStringList& paths, const QModelIndex& targetIndex);

    // 前置函数
    void updateStack(const QString& path, QStack<QString>& stack);
    void updateButtons(QVector<QToolButton*>& bs, QStack<QString>& stack);
    void updateRightView();

    // 双击打开文件夹
    void leftView_doubleClicked(const QModelIndex &index);

    // 前进后退按钮
    void backClicked();
    void forwardClicked();

    // 刷新按钮
    void refreshView();

    // 路径按钮
    void pathClicked();

    // 复制路径
    void copyPath();

    // 是否开启预览
    void preview();

    // 深色模式
    void changeDark();

    // 收藏
    void onlyPath();

    void favoritesContextMenu(const QPoint& pos);
    void favoriteItems(QListWidgetItem* item);

    void upClicked();
    void downClicked();

    void fileView_doubleClicked(const QModelIndex& index);
    void fPathClicked();

    void fSearch();
    void handleFSearchFinished();

    void favoritesDialog(const QString& category);

    // 文件排序
    void fileSort();

    // 搜索
    void search();
    void handleSearchFinished();

    // 右键菜单
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
    void paste(const QString& dir);
    void toRecyleBin();
    void del(const QStringList& paths, bool needConfirm, bool useSystemDelete);
    void del(const QStringList& paths, bool needConfirm);
    void rename();
    void properties();

    // 错误处理
    void fileDeleteError(const QString& filePath, const QString& errorMsg, QFileInfo info);

    // 文件预览
    void previewFile(const QModelIndex& index);

private:
    Ui::MainWindow *ui;

    // 样式
    // view整体样式
    void interfaceStyle();
    // 调整显示列
    void viewAdjust();
    QByteArray savedHeaderState;

    int buttonHeight = 47;
    int buttonWidth = 147;
    int fontSize = 11;

    // 中心部件
    QWidget* centralWidget;

    // 文件系统
    QFileSystemModel* fileModel;
    MyLeftView* leftView;
    QTreeView* rightView;
    SizeProgressDelegate* sizeDelegate;

    // 当在磁盘目录, 不能拖拽粘贴
    bool eventFilter(QObject* obj, QEvent* event) override;

    // 字体
    QFont font;

    // 记录文件路径
    QStack<QString> pathStack;  // 记录分割后的路径, 用来分配给路径按钮
    QString rootPath = "";
    QString filePath;
    QString leftPath;
    QString rightPath;

    // 前进后退按钮
    QPushButton* backButton;
    QPushButton* forwardButton;
    // 用以控制前进按钮的生效(只有后退过才能前进)
    bool backFlag = false;

    // 刷新按钮
    QPushButton* refreshButton;

    // 路径按钮
    QVector<QToolButton*> buttons;
    const int defaultPathButtonWidth = 70;
    const int maxPathButtonWidth = 400;
    const int moreWidth = 30;

    // 复制路径按钮
    QPushButton* copyPathButton;
    QClipboard* clipboard;          // 保存复制的路径

    //　深色模式
    QPushButton* darkButton;
    bool isDarkMode = false;

    // 收藏下拉框
    QPushButton* fButton;
    QComboBox* collectComboBox;
    QMap<QString, QStringList> favorites;

    // 收藏夹
    QDialog* fDialog;
    QPointer<QDialog> favoritesDialogPtr;    // 指向收藏夹

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
    // 让upButton在收藏列表中不生效
    NoClickFilter* buttonFilter = new NoClickFilter(this);

    // 存储收藏列表和文件显示两种视图
    QStackedWidget* fStack;
    QListWidget* favoritesList;
    MyFileView* fileView;

    // 用于更新收藏夹路径
    QStack<QString> fPathStack;
    QString initialPath = "";
    QString mainPath = "";
    QString fPath = "";
    QModelIndex fIdx;
    bool upFlag = false;

    // 收藏夹中的搜索
    QLineEdit* fSearchEdit;
    QStandardItemModel* fSearchModel;
    QFutureWatcher<QStringList> fSearchWatcher;
    bool isFSearch = false;

    void favoritesView();

    void reloadFavoritesList();
    void moveFavorite(QListWidgetItem* item);
    void deleteFavorite(QListWidgetItem* item);

    void copyFavoritePath(const QList<QListWidgetItem*>& selectedItems);
    // 批量操作
    void moveFavorite(const QList<QListWidgetItem*>& items);
    void deleteFavorite(const QList<QListWidgetItem*>& items);

    // Favorites are not minimized when the main program is minimized
    void closeEvent(QCloseEvent* event) override;

    // 文件排序
    QComboBox* sortComboBox;
    QMap<QString, int> sortMap;     // 所有排序方式
    QString currentSort;            // 当前排序方式
    Qt::SortOrder currentOrder;     // 降序或升序

    // 搜索
    QLineEdit* searchEdit;

    QStandardItemModel* searchModel;
    QFutureWatcher<QStringList> searchWatcher;
    bool isSearch = false;

    // 右键菜单
    RightClickMenu* rightClickMenu;
    QStringList clipPaths;
    QModelIndex currentContextIndex;
    QString currentContextPath;

    // 右键菜单适用于主程序和收藏夹
    // 两个变量分别表示当前视图和所选的模型索引
    QAbstractItemView* contextMenuView = nullptr;
    QModelIndex contextMenuIndex;

    QString m_7zPath;
    QProcess* m_compressProc = nullptr;
    QProgressDialog* m_progressDialog = nullptr;
    QTimer* m_fakeProgressTimer = nullptr;
    int m_fakeProgressValue = 0;

    bool openAsAdmin = false;       // 是否是管理员模式
    bool clipIsCut = false;         // 是否在剪切状态
    QStringList categories;

    // 获取文件关联应用程序路径
    QString getAssociatedSW(const QString& path);
    // 提权运行程序
    bool runAsAdmin(const QString& path, const QString& params);

    // 复制一整个目录
    void copyDirectory(const QString& srcPath, const QString& dstPath);
    // 清楚剪切状态
    void clearCutStateIfNeeded();

    // 压缩方式
    QString find7z();
    void renameCompressions(const QString& path);

    // 跨平台快捷方式创建
    bool createWindowsShortcut(const QString& targetPath, const QString& linkPath);
    bool createLinuxDesktopFile(const QString& targetPath, const QString& linkPath);
    bool createMacAlias(const QString& targetPath, const QString& linkPath);

    // 收藏
    void saveFavoritesToFile();
    void deleteFileFromFavorites();
    void loadFavoritesFromFile();
    void collect(const QString& category, const QString& path);

    // 剪切
    QMap<QString, QString> cutMoveMap;    // 剪切粘贴时，保存原路径到新路径的映射
    bool isFileAvailableForCut(const QString& path, QString& errorDetail);

    // 删除
    FileDeleteTask* fileDeleteTask = nullptr;
    void removeFromFavorites(const QString& path);
    bool deleteWithWindowsDialog(const QStringList& paths);
    QString getDeleteErrorReason(const QString& path);
    void startAsyncDelete(const QStringList& filesToDelete, qint64 totalSize);

    // 跨平台删除
    //bool moveToTrash(const QString& path);        // macOS/Linux

    // 文件预览
    QDialog* currentPreviewDialog = nullptr;
    QPushButton* previewButton;
    bool isPreview;

    // 布局
    QHBoxLayout* buttonLayout;
    QSplitter* splitter;
    QVBoxLayout* layout;
};

// 这个模型子类只重写 data(), 用来反应剪切后文件的灰色状态
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
                // 在剪切状态下，剪切的文件/文件夹显示为浅灰色
                return QBrush(Qt::lightGray);
            }
        }
        // 在所有其他情况下，使用基类
        return QFileSystemModel::data(index, role);
    }

private:
    MainWindow* mw;
};

// 文件复制任务
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

// 文件删除任务
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
    void fileDeleteError(const QString& filePath, const QString& errorMsg, QFileInfo info);
    void userDeleteChoice(int action, bool applyToAll);

public slots:
    void doDelete(bool toRecycleBin);

private:
    QStringList m_paths;
    qint64 m_total;
};

#endif // MAINWINDOW_H
