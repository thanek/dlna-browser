#include "browser/contentview.h"

#include <QVBoxLayout>
#include <QSortFilterProxyModel>
#include <QApplication>
#include <QPalette>
#include <QKeyEvent>

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

    m_listView->installEventFilter(this);
    m_iconView->installEventFilter(this);

    QColor selBg = qApp->palette().color(QPalette::Highlight).darker(170);
    const QString itemStyle = QString(
        "QListView { outline: 0; }"
        "QListView::item { padding: 8px; border-radius: 16px; }"
        "QListView::item:selected { background: %1; color: palette(highlighted-text); }"
        "QListView::item:hover:!selected { background: palette(midlight); }")
        .arg(selBg.name());
    m_listView->setStyleSheet(itemStyle);
    m_iconView->setStyleSheet(itemStyle);
}

void ContentView::setupView(QListView *view)
{
    view->setUniformItemSizes(true);
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(view, &QListView::activated, this, [this](const QModelIndex &idx) {
        emit itemActivated(idx.row());
    });
}

void ContentView::setupListView()
{
    m_listView->setViewMode(QListView::ListMode);
    m_listView->setIconSize(QSize(24, 24));
    setupView(m_listView);
}

void ContentView::setupIconView()
{
    m_iconView->setViewMode(QListView::IconMode);
    m_iconView->setIconSize(QSize(64, 64));
    m_iconView->setGridSize(QSize(96, 128));
    m_iconView->setResizeMode(QListView::Adjust);
    m_iconView->setWordWrap(true);
    setupView(m_iconView);
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

void ContentView::setIconScale(int scale)
{
    int li = 16 + scale * 40 / 100;           // list icon:  16..56 px
    int ii = 48 + scale * 160 / 100;          // icon icon: 48..208 px
    int gw = ii + 32;                         // grid width: 32 px padding
    int gh = ii + 64;                         // grid height: 64 px fixed label area

    m_listView->setIconSize({li, li});
    m_iconView->setIconSize({ii, ii});
    m_iconView->setGridSize({gw, gh});
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

bool ContentView::eventFilter(QObject *obj, QEvent *event)
{
    if ((obj == m_listView || obj == m_iconView) && event->type() == QEvent::KeyPress) {
        auto *ke = static_cast<QKeyEvent *>(event);
        if (ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter) {
            auto *view = static_cast<QListView *>(obj);
            QModelIndex idx = view->currentIndex();
            if (idx.isValid())
                emit itemActivated(idx.row());
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}
