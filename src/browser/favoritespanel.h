#pragma once

#include <QListWidget>
#include "dlna/dlnaitem.h"

class FavoritesPanel : public QWidget {
    Q_OBJECT
public:
    explicit FavoritesPanel(QWidget *parent = nullptr);

    void addFavorite(const QString &name, const QList<DlnaLocation> &path);
    void loadFavorites();
    void saveFavorites();

signals:
    void favoriteActivated(const QList<DlnaLocation> &path);

private slots:
    void onItemDoubleClicked(QListWidgetItem *item);
    void onContextMenu(const QPoint &pos);

private:
    QListWidget *m_list;

    struct Favorite {
        QString name;
        QList<DlnaLocation> path;
    };
    QList<Favorite> m_favorites;
};
