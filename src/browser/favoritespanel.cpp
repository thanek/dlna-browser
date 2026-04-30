#include "browser/favoritespanel.h"
#include "ui/faicon.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QListWidgetItem>
#include <QMenu>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
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
    layout->setSpacing(4);

    auto *header = new QLabel(tr("Favorites"), this);
    header->setStyleSheet("padding: 6px 8px; font-weight: bold; color: palette(text);");
    layout->addWidget(header);
    layout->addWidget(m_list);

    m_list->setFrameShape(QFrame::NoFrame);
    m_list->setStyleSheet("QListWidget::item { padding: 6px 4px; }");
    m_list->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(m_list, &QListWidget::itemDoubleClicked,
            this, &FavoritesPanel::onItemDoubleClicked);
    connect(m_list, &QListWidget::customContextMenuRequested,
            this, &FavoritesPanel::onContextMenu);

    loadFavorites();
}

void FavoritesPanel::addFavorite(const QString &name, const QList<DlnaLocation> &path)
{
    if (path.isEmpty()) return;
    const DlnaLocation &last = path.last();
    for (const auto &fav : m_favorites) {
        if (!fav.path.isEmpty() &&
            fav.path.last().controlUrl  == last.controlUrl &&
            fav.path.last().containerId == last.containerId)
            return;
    }

    m_favorites.append({name, path});
    auto *item = new QListWidgetItem(FaIcon::icon(Fa::Star, QColor(0xff, 0xc1, 0x07)), name);
    m_list->addItem(item);
    saveFavorites();
}

static DlnaLocation locationFromJson(const QJsonObject &obj)
{
    DlnaLocation loc;
    loc.serverName  = obj["serverName"].toString();
    loc.controlUrl  = obj["controlUrl"].toString();
    loc.containerId = obj["containerId"].toString();
    loc.title       = obj["title"].toString();
    return loc;
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
        if (obj.contains("path")) {
            for (const auto &locVal : obj["path"].toArray())
                fav.path.append(locationFromJson(locVal.toObject()));
        } else {
            // migrate old single-location format
            fav.path.append(locationFromJson(obj));
        }
        if (fav.path.isEmpty()) continue;
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
        QJsonArray pathArr;
        for (const auto &loc : fav.path) {
            QJsonObject locObj;
            locObj["serverName"]  = loc.serverName;
            locObj["controlUrl"]  = loc.controlUrl;
            locObj["containerId"] = loc.containerId;
            locObj["title"]       = loc.title;
            pathArr.append(locObj);
        }
        QJsonObject obj;
        obj["name"] = fav.name;
        obj["path"] = pathArr;
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
        emit favoriteActivated(m_favorites.at(row).path);
}

void FavoritesPanel::onContextMenu(const QPoint &pos)
{
    QListWidgetItem *item = m_list->itemAt(pos);
    if (!item) return;
    int row = m_list->row(item);

    QMenu menu(this);
    QAction *renameAct = menu.addAction(tr("Rename…"));
    QAction *removeAct = menu.addAction(tr("Remove from favorites"));

    QAction *chosen = menu.exec(m_list->viewport()->mapToGlobal(pos));
    if (chosen == renameAct) {
        bool ok;
        QString newName = QInputDialog::getText(this, tr("Rename favorite"),
                                                tr("Name:"), QLineEdit::Normal,
                                                m_favorites[row].name, &ok);
        if (ok && !newName.trimmed().isEmpty()) {
            m_favorites[row].name = newName.trimmed();
            item->setText(newName.trimmed());
            saveFavorites();
        }
    } else if (chosen == removeAct) {
        m_favorites.removeAt(row);
        delete m_list->takeItem(row);
        saveFavorites();
    }
}
