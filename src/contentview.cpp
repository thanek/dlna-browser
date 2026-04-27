#include "contentview.h"

#include <QVBoxLayout>
#include <QSortFilterProxyModel>

ContentView::ContentView(QWidget *parent)
    : QWidget(parent)
    , m_stack(new QStackedWidget(this))
    , m_listView(new QListView(this))
    , m_iconView(new QListView(this))
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_stack);

    setupListView();
    setupIconView();

    m_stack->addWidget(m_listView);
    m_stack->addWidget(m_iconView);
    m_stack->setCurrentWidget(m_listView);
}

void ContentView::setupListView()
{
    m_listView->setViewMode(QListView::ListMode);
    m_listView->setIconSize(QSize(24, 24));
    m_listView->setUniformItemSizes(true);
    m_listView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_listView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(m_listView, &QListView::activated, this, [this](const QModelIndex &idx) {
        emit itemActivated(idx.row());
    });
}

void ContentView::setupIconView()
{
    m_iconView->setViewMode(QListView::IconMode);
    m_iconView->setIconSize(QSize(64, 64));
    m_iconView->setGridSize(QSize(96, 96));
    m_iconView->setResizeMode(QListView::Adjust);
    m_iconView->setUniformItemSizes(true);
    m_iconView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_iconView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_iconView->setWordWrap(true);

    connect(m_iconView, &QListView::activated, this, [this](const QModelIndex &idx) {
        emit itemActivated(idx.row());
    });
}

void ContentView::setModel(DlnaModel *model)
{
    m_model = model;
    m_listView->setModel(model);
    m_iconView->setModel(model);
}

void ContentView::setViewMode(ViewMode mode)
{
    m_viewMode = mode;
    if (mode == ViewMode::List)
        m_stack->setCurrentWidget(m_listView);
    else
        m_stack->setCurrentWidget(m_iconView);
}

void ContentView::setSortMode(SortMode mode)
{
    m_sortMode = mode;
}

void ContentView::setIconScale(int scale)
{
    int li = 16 + scale * 40 / 100;           // list icon:  16..56 px
    int ii = 48 + scale * 160 / 100;          // icon icon: 48..208 px
    int ig = ii + 32;                         // icon grid: always 32 px more than icon

    m_listView->setIconSize({li, li});
    m_iconView->setIconSize({ii, ii});
    m_iconView->setGridSize({ig, ig});
}

void ContentView::setCurrentRow(int row)
{
    if (!m_model || row < 0 || row >= m_model->rowCount()) return;
    QModelIndex idx = m_model->index(row, 0);
    m_listView->setCurrentIndex(idx);
    m_listView->scrollTo(idx);
    m_iconView->setCurrentIndex(idx);
    m_iconView->scrollTo(idx);
}
