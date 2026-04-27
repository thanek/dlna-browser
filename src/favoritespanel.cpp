#include "favoritespanel.h"
#include "faicon.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QListWidgetItem>
#include <QMenu>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDir>
#include <QFile>

FavoritesPanel::FavoritesPanel(QWidget *parent)
    : QWidget(parent)
    , m_list(new QListWidget(this))
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    auto *header = new QLabel(tr("Favorites"), this);
    header->setStyleSheet("padding: 6px 8px; font-weight: bold; color: palette(mid);");
    layout->addWidget(header);
    layout->addWidget(m_list);

    m_list->setFrameShape(QFrame::NoFrame);
    m_list->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(m_list, &QListWidget::itemDoubleClicked,
            this, &FavoritesPanel::onItemDoubleClicked);
    connect(m_list, &QListWidget::customContextMenuRequested,
            this, &FavoritesPanel::onContextMenu);

    loadFavorites();
}

void FavoritesPanel::addFavorite(const QString &name, const DlnaLocation &location)
{
    for (const auto &fav : m_favorites) {
        if (fav.location.controlUrl == location.controlUrl &&
            fav.location.containerId == location.containerId)
            return;
    }

    m_favorites.append({name, location});
    auto *item = new QListWidgetItem(FaIcon::icon(Fa::Star, QColor(0xff, 0xc1, 0x07)), name);
    m_list->addItem(item);
    saveFavorites();
}

void FavoritesPanel::loadFavorites()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                   + "/favorites.json";
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return;

    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    for (const auto &val : doc.array()) {
        QJsonObject obj = val.toObject();
        Favorite fav;
        fav.name = obj["name"].toString();
        fav.location.serverName  = obj["serverName"].toString();
        fav.location.controlUrl  = obj["controlUrl"].toString();
        fav.location.containerId = obj["containerId"].toString();
        fav.location.title       = obj["title"].toString();
        m_favorites.append(fav);
        m_list->addItem(new QListWidgetItem(FaIcon::icon(Fa::Star, QColor(0xff, 0xc1, 0x07)), fav.name));
    }
}

void FavoritesPanel::saveFavorites()
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    QString path = dir + "/favorites.json";

    QJsonArray arr;
    for (const auto &fav : m_favorites) {
        QJsonObject obj;
        obj["name"]        = fav.name;
        obj["serverName"]  = fav.location.serverName;
        obj["controlUrl"]  = fav.location.controlUrl;
        obj["containerId"] = fav.location.containerId;
        obj["title"]       = fav.location.title;
        arr.append(obj);
    }
    QFile f(path);
    if (f.open(QIODevice::WriteOnly))
        f.write(QJsonDocument(arr).toJson());
}

void FavoritesPanel::onItemDoubleClicked(QListWidgetItem *item)
{
    int row = m_list->row(item);
    if (row >= 0 && row < m_favorites.size())
        emit favoriteActivated(m_favorites.at(row).location);
}

void FavoritesPanel::onContextMenu(const QPoint &pos)
{
    QListWidgetItem *item = m_list->itemAt(pos);
    if (!item) return;
    int row = m_list->row(item);

    QMenu menu(this);
    QAction *removeAct = menu.addAction(tr("Remove from favorites"));
    if (menu.exec(m_list->viewport()->mapToGlobal(pos)) == removeAct) {
        m_favorites.removeAt(row);
        delete m_list->takeItem(row);
        saveFavorites();
    }
}
