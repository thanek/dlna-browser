#pragma once

#include <QWidget>
#include <QListWidget>
#include "dlnaitem.h"

class FavoritesPanel : public QWidget {
    Q_OBJECT
public:
    explicit FavoritesPanel(QWidget *parent = nullptr);

    void addFavorite(const QString &name, const DlnaLocation &location);
    void loadFavorites();
    void saveFavorites();

signals:
    void favoriteActivated(const DlnaLocation &location);

private slots:
    void onItemDoubleClicked(QListWidgetItem *item);
    void onContextMenu(const QPoint &pos);

private:
    QListWidget *m_list;

    struct Favorite {
        QString name;
        DlnaLocation location;
    };
    QList<Favorite> m_favorites;
};
