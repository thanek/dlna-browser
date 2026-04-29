#pragma once

#include <QWidget>
#include <QStackedWidget>
#include <QListView>
#include <QTreeView>
#include "browser/dlnamodel.h"

enum class ViewMode { List, Icons };
enum class SortMode { NameAsc, NameDesc, DateAsc, DateDesc };

class ContentView : public QWidget {
    Q_OBJECT
public:
    explicit ContentView(QWidget *parent = nullptr);

    void setModel(DlnaModel *model);
    void setViewMode(ViewMode mode);
    void setCurrentRow(int row);
    void setIconScale(int scale);   // 0-100
    ViewMode viewMode() const { return m_viewMode; }

signals:
    void itemActivated(int row);

private:
    void setupListView();
    void setupIconView();

    QStackedWidget *m_stack;
    QListView *m_listView;
    QListView *m_iconView;
    DlnaModel *m_model = nullptr;
    ViewMode m_viewMode = ViewMode::List;
};
