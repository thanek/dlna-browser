#pragma once

#include <QWidget>
#include <QStackedWidget>
#include <QListView>
#include <QTreeView>
#include "dlnamodel.h"

enum class ViewMode { List, Icons };
enum class SortMode { NameAsc, NameDesc, DateAsc, DateDesc };

class ContentView : public QWidget {
    Q_OBJECT
public:
    explicit ContentView(QWidget *parent = nullptr);

    void setModel(DlnaModel *model);
    void setViewMode(ViewMode mode);
    void setSortMode(SortMode mode);
    void setCurrentRow(int row);
    void setIconScale(int scale);   // 0-100
    ViewMode viewMode() const { return m_viewMode; }

signals:
    void itemActivated(int row);

private:
    void setupListView();
    void setupIconView();
    void applySort();

    QStackedWidget *m_stack;
    QListView *m_listView;
    QListView *m_iconView;
    DlnaModel *m_model = nullptr;
    ViewMode m_viewMode = ViewMode::List;
    SortMode m_sortMode = SortMode::NameAsc;
};
