#include "browser/mainwindow.h"
#include "browser/aboutdialog.h"
#include "browser/addressbar.h"
#include "browser/contentview.h"
#include "browser/favoritespanel.h"
#include "mediaviewer/mediaviewer.h"
#include "ui/faicon.h"

#include <QSettings>

#include <QToolBar>
#include <QToolButton>
#include <QAction>
#include <QActionGroup>
#include <QSplitter>
#include <QLabel>
#include <QStatusBar>
#include <QMenu>
#include <QMenuBar>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QPixmap>
#include <QThread>
#include <QSemaphore>
#include <QApplication>
#include <QStyle>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_discovery(new DlnaDiscovery(this))
    , m_client(new DlnaClient(this))
    , m_model(new DlnaModel(this))
    , m_thumbnailNam(new QNetworkAccessManager(this))
{
    setWindowTitle(tr("DLNA Browser"));
    resize(1100, 700);

    setupUi();
    setupToolBar();
    setupStatusBar();
    setupMenuBar();

    connect(m_discovery, &DlnaDiscovery::serverFound,
            this, &MainWindow::onServerFound);
    connect(m_discovery, &DlnaDiscovery::discoveryFinished,
            this, &MainWindow::onDiscoveryFinished);
    connect(m_client, &DlnaClient::controlUrlReady,
            this, &MainWindow::onControlUrlReady);
    connect(m_client, &DlnaClient::browseReady,
            this, &MainWindow::onBrowseReady);
    connect(m_client, &DlnaClient::browseError,
            this, &MainWindow::onBrowseError);
    connect(m_contentView, &ContentView::itemActivated,
            this, &MainWindow::onItemActivated);
    connect(m_favoritesPanel, &FavoritesPanel::favoriteActivated,
            this, &MainWindow::onFavoriteActivated);
    connect(m_mediaViewer, &MediaViewer::rowChanged,
            m_contentView, &ContentView::setCurrentRow);

    // Restore persisted settings
    QSettings settings;

    int scale = settings.value("iconScale", 25).toInt();
    m_sizeSlider->setValue(scale);
    m_contentView->setIconScale(scale);

    if (settings.value("viewMode").toString() == "icons") {
        m_viewMode = ViewMode::Icons;
        m_btnView->setIcon(FaIcon::icon(Fa::Grip));
        m_contentView->setViewMode(ViewMode::Icons);
    }

    // Show home (server list) on startup
    navigateHome();
}

MainWindow::~MainWindow() = default;

// ── Menu bar ─────────────────────────────────────────────────────────────────

void MainWindow::setupMenuBar()
{
    auto *helpMenu = menuBar()->addMenu(tr("Help"));
    QAction *actAbout = helpMenu->addAction(tr("About DLNA Browser…"));
    actAbout->setMenuRole(QAction::AboutRole);
    connect(actAbout, &QAction::triggered, this, &MainWindow::showAbout);
}

void MainWindow::showAbout()
{
    AboutDialog dlg(this);
    dlg.exec();
}

// ── UI Setup ────────────────────────────────────────────────────────────────

void MainWindow::setupUi()
{
    auto *splitter = new QSplitter(Qt::Horizontal, this);
    setCentralWidget(splitter);

    m_favoritesPanel = new FavoritesPanel(this);
    m_favoritesPanel->setMinimumWidth(160);
    m_favoritesPanel->setMaximumWidth(280);

    m_contentView = new ContentView(this);
    m_contentView->setModel(m_model);

    splitter->addWidget(m_favoritesPanel);
    splitter->addWidget(m_contentView);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    splitter->setSizes({200, 900});

    m_mediaViewer = new MediaViewer(nullptr);
}

void MainWindow::setupToolBar()
{
    m_toolBar = addToolBar(tr("Navigation"));
    m_toolBar->setMovable(false);
    m_toolBar->setIconSize(QSize(20, 20));

    m_actBack = m_toolBar->addAction(FaIcon::icon(Fa::ArrowLeft), tr("Back"));
    m_actForward = m_toolBar->addAction(FaIcon::icon(Fa::ArrowRight), tr("Forward"));
    m_actUp = m_toolBar->addAction(FaIcon::icon(Fa::ArrowUp), tr("Up"));
    m_actHome = m_toolBar->addAction(FaIcon::icon(Fa::House), tr("Home"));
    m_toolBar->addSeparator();

    m_addressBar = new AddressBar(this);
    m_toolBar->addWidget(m_addressBar);
    m_toolBar->addSeparator();

    // Add to favorites button
    m_actAddFav = m_toolBar->addAction(FaIcon::icon(Fa::Star), tr("Add to favorites"));

    // View toggle button
    m_btnView = new QToolButton(this);
    m_btnView->setIcon(FaIcon::icon(Fa::List));
    m_btnView->setToolTip(tr("Toggle view"));
    m_btnView->setCheckable(false);
    m_toolBar->addWidget(m_btnView);

    // Sort menu
    auto *btnSort = new QToolButton(this);
    btnSort->setIcon(FaIcon::icon(Fa::Sort));
    btnSort->setToolTip(tr("Sort"));
    btnSort->setPopupMode(QToolButton::InstantPopup);

    auto *sortMenu = new QMenu(this);
    auto *sortGroup = new QActionGroup(sortMenu);
    sortGroup->setExclusive(true);

    m_actSortNameAsc  = sortMenu->addAction(tr("Name A→Z"));
    m_actSortNameDesc = sortMenu->addAction(tr("Name Z→A"));
    m_actSortDateAsc  = sortMenu->addAction(tr("Date (oldest first)"));
    m_actSortDateDesc = sortMenu->addAction(tr("Date (newest first)"));

    for (auto *act : {m_actSortNameAsc, m_actSortNameDesc,
                      m_actSortDateAsc, m_actSortDateDesc}) {
        act->setCheckable(true);
        sortGroup->addAction(act);
    }
    m_actSortNameAsc->setChecked(true);
    btnSort->setMenu(sortMenu);
    m_toolBar->addWidget(btnSort);

    // Connections
    connect(m_actBack,    &QAction::triggered, this, &MainWindow::navigateBack);
    connect(m_actForward, &QAction::triggered, this, &MainWindow::navigateForward);
    connect(m_actUp,      &QAction::triggered, this, &MainWindow::navigateUp);
    connect(m_actHome,    &QAction::triggered, this, &MainWindow::navigateHome);
    connect(m_actAddFav,  &QAction::triggered, this, &MainWindow::addCurrentToFavorites);
    connect(m_btnView,    &QToolButton::clicked, this, &MainWindow::onViewToggled);
    connect(sortGroup, &QActionGroup::triggered, this, &MainWindow::onSortChanged);

    connect(m_addressBar, &AddressBar::navigateTo, this, [this](const QList<DlnaLocation> &path) {
        if (path.isEmpty()) { navigateHome(); return; }
        m_forwardStack.clear();
        m_history = path;
        m_atHome = false;
        browseCurrentLocation();
    });

    updateNavigationButtons();
}

void MainWindow::setupStatusBar()
{
    m_statusLabel = new QLabel(this);
    statusBar()->addWidget(m_statusLabel, 1);

    auto *iconSmall = new QLabel(this);
    iconSmall->setPixmap(FaIcon::pixmap(Fa::Minimize, QColor(160, 160, 160), 12));
    iconSmall->setContentsMargins(4, 0, 2, 0);

    m_sizeSlider = new QSlider(Qt::Horizontal, this);
    m_sizeSlider->setRange(0, 100);
    m_sizeSlider->setValue(25);
    m_sizeSlider->setFixedWidth(110);
    m_sizeSlider->setToolTip(tr("Icon size"));

    auto *iconLarge = new QLabel(this);
    iconLarge->setPixmap(FaIcon::pixmap(Fa::Maximize, QColor(160, 160, 160), 14));
    iconLarge->setContentsMargins(2, 0, 4, 0);

    statusBar()->addPermanentWidget(iconSmall);
    statusBar()->addPermanentWidget(m_sizeSlider);
    statusBar()->addPermanentWidget(iconLarge);

    connect(m_sizeSlider, &QSlider::valueChanged, this, [this](int v) {
        m_contentView->setIconScale(v);
        QSettings().setValue("iconScale", v);
    });
}

// ── Sort persistence helpers ─────────────────────────────────────────────────

static QString sortKey(const DlnaLocation &loc)
{
    return "sort/" + loc.serverName + "/" + loc.containerId;
}

static QString sortModeToString(SortMode m)
{
    switch (m) {
    case SortMode::NameAsc:  return "nameAsc";
    case SortMode::NameDesc: return "nameDesc";
    case SortMode::DateAsc:  return "dateAsc";
    case SortMode::DateDesc: return "dateDesc";
    }
    return "nameAsc";
}

static SortMode sortModeFromString(const QString &s)
{
    if (s == "nameDesc") return SortMode::NameDesc;
    if (s == "dateAsc")  return SortMode::DateAsc;
    if (s == "dateDesc") return SortMode::DateDesc;
    return SortMode::NameAsc;
}

void MainWindow::applySortMode(SortMode mode)
{
    m_sortMode = mode;
    m_actSortNameAsc ->setChecked(mode == SortMode::NameAsc);
    m_actSortNameDesc->setChecked(mode == SortMode::NameDesc);
    m_actSortDateAsc ->setChecked(mode == SortMode::DateAsc);
    m_actSortDateDesc->setChecked(mode == SortMode::DateDesc);
}

SortMode MainWindow::effectiveSortMode() const
{
    QSettings settings;
    for (int i = m_history.size() - 1; i >= 0; --i) {
        const QString key = sortKey(m_history[i]);
        if (settings.contains(key))
            return sortModeFromString(settings.value(key).toString());
    }
    return SortMode::NameAsc;
}

// ── Navigation ───────────────────────────────────────────────────────────────

void MainWindow::navigateHome()
{
    m_history.clear();
    m_forwardStack.clear();
    m_atHome = true;
    m_model->clear();

    QList<DlnaItem> serverItems;
    for (const auto &srv : m_servers) {
        DlnaItem item;
        item.id = srv.location;
        item.title = srv.name;
        item.type = DlnaItemType::Server;
        item.thumbnailUrl = srv.iconUrl;
        serverItems.append(item);
    }
    m_model->setItems(serverItems);
    restoreFocus();
    loadThumbnails(serverItems);

    updateAddressBar();

    updateNavigationButtons();

    if (m_servers.isEmpty()) {
        m_statusLabel->setText(tr("Scanning for DLNA servers…"));
        m_discovery->startDiscovery();
    } else {
        m_statusLabel->setText(tr("%1 server(s) found").arg(m_servers.size()));
    }
}

void MainWindow::navigateTo(const DlnaLocation &location)
{
    m_forwardStack.clear();
    m_history.append(location);
    m_atHome = false;
    browseCurrentLocation();
}

void MainWindow::browseCurrentLocation()
{
    if (m_atHome) { navigateHome(); return; }
    if (m_history.isEmpty()) return;

    applySortMode(effectiveSortMode());

    const DlnaLocation &loc = m_history.last();
    m_statusLabel->setText(tr("Loading %1…").arg(loc.title));
    m_model->clear();

    m_client->browse(loc.controlUrl, loc.containerId, sortCriteriaString());

    updateAddressBar();
    updateNavigationButtons();
}

void MainWindow::navigateBack()
{
    if (m_history.size() <= 1) {
        if (!m_history.isEmpty()) {
            const DlnaLocation &leaving = m_history.last();
            m_pendingFocusId = focusIdFor(leaving);
            m_forwardStack.prepend(m_history.takeLast());
        }
        m_atHome = true;
        navigateHome();
        return;
    }
    const DlnaLocation &leaving = m_history.last();
    m_pendingFocusId = focusIdFor(leaving);
    m_forwardStack.prepend(m_history.takeLast());
    browseCurrentLocation();
}

void MainWindow::navigateForward()
{
    if (m_forwardStack.isEmpty()) return;
    m_history.append(m_forwardStack.takeFirst());
    m_atHome = false;
    browseCurrentLocation();
}

void MainWindow::navigateUp()
{
    if (m_atHome) return;
    const DlnaLocation &leaving = m_history.last();
    m_pendingFocusId = focusIdFor(leaving);
    if (m_history.size() <= 1) {
        navigateHome();
        return;
    }
    m_forwardStack.clear();
    m_history.removeLast();
    browseCurrentLocation();
}

void MainWindow::updateAddressBar()
{
    QList<DlnaLocation> crumb;
    DlnaLocation home; home.title = tr("DLNA");
    crumb.append(home);
    crumb.append(m_history);
    m_addressBar->setPath(crumb);
}

void MainWindow::updateNavigationButtons()
{
    m_actBack->setEnabled(!m_atHome || !m_history.isEmpty());
    m_actForward->setEnabled(!m_forwardStack.isEmpty());
    m_actUp->setEnabled(!m_atHome);
    m_actAddFav->setEnabled(!m_atHome && !m_history.isEmpty());
}

// ── DLNA callbacks ───────────────────────────────────────────────────────────

void MainWindow::onServerFound(const DlnaServer &server)
{
    // Avoid duplicates
    for (const auto &s : m_servers)
        if (s.usn == server.usn) return;

    m_servers.append(server);

    if (m_atHome) {
        DlnaItem item;
        item.id = server.location;
        item.title = server.name;
        item.type = DlnaItemType::Server;
        item.thumbnailUrl = server.iconUrl;

        m_model->appendItem(item);

        if (!item.thumbnailUrl.isEmpty()) {
            int row = m_model->rowCount() - 1;
            QNetworkRequest req(item.thumbnailUrl);
            req.setTransferTimeout(5000);
            QNetworkReply *reply = m_thumbnailNam->get(req);
            connect(reply, &QNetworkReply::finished, this, [this, reply, row]() {
                reply->deleteLater();
                if (reply->error() != QNetworkReply::NoError) return;
                QPixmap px;
                if (px.loadFromData(reply->readAll()))
                    m_model->setThumbnail(row, px);
            });
        }

        m_statusLabel->setText(tr("Found: %1").arg(server.name));
    }
}

void MainWindow::onDiscoveryFinished()
{
    if (m_atHome) {
        int n = m_model->rowCount();
        m_statusLabel->setText(n == 0
            ? tr("No DLNA servers found on the network")
            : tr("%1 DLNA server(s) found").arg(n));
    }
}

void MainWindow::onControlUrlReady(const QString &serverName, const QString &controlUrl)
{
    Q_UNUSED(serverName)
    if (m_history.isEmpty()) return;
    m_history.last().controlUrl = controlUrl;
    applySortMode(effectiveSortMode());
    m_client->browse(controlUrl, m_history.last().containerId, sortCriteriaString());
}

void MainWindow::onBrowseReady(const QList<DlnaItem> &items)
{
    m_model->setItems(items);
    restoreFocus();

    int containers = 0, files = 0;
    for (const auto &i : items)
        i.isContainer() ? ++containers : ++files;

    QString status;
    if (containers && files)
        status = tr("%1 folder(s), %2 file(s)").arg(containers).arg(files);
    else if (containers)
        status = tr("%1 folder(s)").arg(containers);
    else
        status = tr("%1 file(s)").arg(files);
    m_statusLabel->setText(status);

    loadThumbnails(items);
}

void MainWindow::onBrowseError(const QString &message)
{
    m_statusLabel->setText(tr("Error: %1").arg(message));
}

void MainWindow::onItemActivated(int row)
{
    DlnaItem item = m_model->itemAt(row);

    if (item.type == DlnaItemType::Server) {
        // Fetch control URL then browse root
        DlnaLocation loc;
        loc.serverName = item.title;
        loc.containerId = "0";
        loc.title = item.title;
        m_forwardStack.clear();
        m_history.append(loc);
        m_atHome = false;
        updateNavigationButtons();
        m_statusLabel->setText(tr("Connecting to %1…").arg(item.title));
        updateAddressBar();
        m_client->fetchControlUrl(item.id, item.title);
        return;
    }

    if (item.isContainer()) {
        if (m_history.isEmpty()) return;
        DlnaLocation loc;
        loc.serverName = m_history.last().serverName;
        loc.controlUrl = m_history.last().controlUrl;
        loc.containerId = item.id;
        loc.title = item.title;
        navigateTo(loc);
        return;
    }

    // Leaf item — open in media viewer
    m_mediaViewer->openItem(m_model, row);
}

void MainWindow::onFavoriteActivated(const QList<DlnaLocation> &path)
{
    m_forwardStack.clear();
    m_history = QList<DlnaLocation>(path.constBegin(), path.constEnd());
    m_atHome = false;
    browseCurrentLocation();
}

// ── Toolbar actions ──────────────────────────────────────────────────────────

void MainWindow::onSortChanged(QAction *action)
{
    SortMode mode;
    if (action == m_actSortNameAsc)       mode = SortMode::NameAsc;
    else if (action == m_actSortNameDesc) mode = SortMode::NameDesc;
    else if (action == m_actSortDateAsc)  mode = SortMode::DateAsc;
    else                                   mode = SortMode::DateDesc;

    if (!m_atHome && !m_history.isEmpty())
        QSettings().setValue(sortKey(m_history.last()), sortModeToString(mode));

    applySortMode(mode);
    if (!m_atHome) browseCurrentLocation();
}

void MainWindow::onViewToggled()
{
    if (m_viewMode == ViewMode::List) {
        m_viewMode = ViewMode::Icons;
        m_btnView->setIcon(FaIcon::icon(Fa::Grip));
    } else {
        m_viewMode = ViewMode::List;
        m_btnView->setIcon(FaIcon::icon(Fa::List));
    }
    m_contentView->setViewMode(m_viewMode);
    QSettings().setValue("viewMode", m_viewMode == ViewMode::Icons ? "icons" : "list");
}

void MainWindow::addCurrentToFavorites()
{
    if (m_atHome || m_history.isEmpty()) return;
    const DlnaLocation &loc = m_history.last();
    QString name = loc.title.isEmpty() ? loc.serverName : loc.title;
    m_favoritesPanel->addFavorite(name, m_history);
}

// ── Helpers ──────────────────────────────────────────────────────────────────

QString MainWindow::sortCriteriaString() const
{
    switch (m_sortMode) {
    case SortMode::NameAsc:  return "+dc:title";
    case SortMode::NameDesc: return "-dc:title";
    case SortMode::DateAsc:  return "+dc:date";
    case SortMode::DateDesc: return "-dc:date";
    }
    return {};
}

QString MainWindow::focusIdFor(const DlnaLocation &loc) const
{
    return loc.containerId == "0" ? loc.serverName : loc.containerId;
}

void MainWindow::restoreFocus()
{
    if (m_pendingFocusId.isEmpty()) return;
    for (int row = 0; row < m_model->rowCount(); ++row) {
        const DlnaItem &item = m_model->itemAt(row);
        if (item.id == m_pendingFocusId || item.title == m_pendingFocusId) {
            m_contentView->setCurrentRow(row);
            break;
        }
    }
    m_pendingFocusId.clear();
}

void MainWindow::loadThumbnails(const QList<DlnaItem> &items)
{
    ++m_thumbnailGeneration;
    int gen = m_thumbnailGeneration;

    for (int row = 0; row < items.size(); ++row) {
        const QUrl &url = items[row].thumbnailUrl;
        if (url.isEmpty()) continue;

        QNetworkRequest req(url);
        req.setTransferTimeout(5000);
        QNetworkReply *reply = m_thumbnailNam->get(req);

        connect(reply, &QNetworkReply::finished, this, [this, reply, row, gen]() {
            reply->deleteLater();
            if (gen != m_thumbnailGeneration) return; // stale, navigation moved on
            if (reply->error() != QNetworkReply::NoError) return;

            QPixmap px;
            if (px.loadFromData(reply->readAll()))
                m_model->setThumbnail(row, px);
        });
    }
}

