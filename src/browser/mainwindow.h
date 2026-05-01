#pragma once

#include <QMainWindow>
#include <QList>
#include <QNetworkAccessManager>
#include <QSlider>
#include "dlna/dlnaitem.h"
#include "dlna/dlnadiscovery.h"
#include "dlna/dlnaclient.h"
#include "browser/dlnamodel.h"
#include "browser/contentview.h"

class QToolBar;
class QAction;
class QLabel;
class QToolButton;
class AddressBar;
class ContentView;
class FavoritesPanel;
class MediaViewer;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onServerFound(const DlnaServer &server);
    void onDiscoveryFinished();
    void onControlUrlReady(const QString &serverName, const QString &controlUrl);
    void onBrowseReady(const QList<DlnaItem> &items);
    void onBrowseError(const QString &message);
    void onItemActivated(int row);
    void onFavoriteActivated(const QList<DlnaLocation> &path);

    void navigateBack();
    void navigateForward();
    void navigateUp();
    void navigateHome();
    void onSortChanged(QAction *action);
    void onViewToggled();
    void addCurrentToFavorites();
    void showAbout();

private:
    void setupUi();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void navigateTo(const DlnaLocation &location);
    void browseCurrentLocation();
    void updateNavigationButtons();
    void updateAddressBar();
    void loadThumbnails(const QList<DlnaItem> &items);
    void restoreFocus();
    QString sortCriteriaString() const;
    void applySortMode(SortMode mode);
    SortMode effectiveSortMode() const;
    QString focusIdFor(const DlnaLocation &loc) const;

    // UI
    QToolBar *m_toolBar = nullptr;
    QAction *m_actBack = nullptr;
    QAction *m_actForward = nullptr;
    QAction *m_actUp = nullptr;
    QAction *m_actHome = nullptr;
    QAction *m_actAddFav = nullptr;
    QAction *m_actSortNameAsc = nullptr;
    QAction *m_actSortNameDesc = nullptr;
    QAction *m_actSortDateAsc = nullptr;
    QAction *m_actSortDateDesc = nullptr;
    QToolButton *m_btnView = nullptr;
    AddressBar *m_addressBar = nullptr;
    ContentView *m_contentView = nullptr;
    FavoritesPanel *m_favoritesPanel = nullptr;
    QLabel  *m_statusLabel = nullptr;
    QSlider *m_sizeSlider  = nullptr;
    MediaViewer *m_mediaViewer = nullptr;

    // DLNA
    DlnaDiscovery *m_discovery = nullptr;
    DlnaClient *m_client = nullptr;
    DlnaModel *m_model = nullptr;
    QNetworkAccessManager *m_thumbnailNam = nullptr;

    // Navigation state
    QList<DlnaLocation> m_history;
    QList<DlnaLocation> m_forwardStack;
    QList<DlnaServer> m_servers;
    bool m_atHome = true;

    SortMode m_sortMode = SortMode::NameAsc;
    ViewMode m_viewMode = ViewMode::List;

    int m_thumbnailGeneration = 0;
    int m_autoScanRetries = 0;
    QString m_pendingFocusId;
};
