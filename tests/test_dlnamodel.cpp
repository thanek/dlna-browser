#include "test_dlnamodel.h"

static DlnaItem makeItem(const QString &id, const QString &title,
                         DlnaItemType type, const QString &mime = {},
                         const QUrl &url = {}, const QString &date = {})
{
    DlnaItem it;
    it.id          = id;
    it.title       = title;
    it.type        = type;
    it.mimeType    = mime;
    it.resourceUrl = url;
    it.date        = date;
    return it;
}

// ─── rowCount ────────────────────────────────────────────────────────────────

void TestDlnaModel::rowCount_empty()
{
    DlnaModel m;
    QCOMPARE(m.rowCount(), 0);
}

void TestDlnaModel::rowCount_afterSetItems()
{
    DlnaModel m;
    m.setItems({makeItem("1", "A", DlnaItemType::Video),
                makeItem("2", "B", DlnaItemType::Audio)});
    QCOMPARE(m.rowCount(), 2);
}

// ─── setItems / clear ─────────────────────────────────────────────────────────

void TestDlnaModel::setItems_replacesExisting()
{
    DlnaModel m;
    m.setItems({makeItem("1", "A", DlnaItemType::Video)});
    m.setItems({makeItem("2", "B", DlnaItemType::Audio),
                makeItem("3", "C", DlnaItemType::Image)});
    QCOMPARE(m.rowCount(), 2);
    QCOMPARE(m.itemAt(0).title, "B");
}

void TestDlnaModel::clear_emptiesModel()
{
    DlnaModel m;
    m.setItems({makeItem("1", "X", DlnaItemType::Video)});
    m.clear();
    QCOMPARE(m.rowCount(), 0);
}

// ─── data() roles ─────────────────────────────────────────────────────────────

void TestDlnaModel::data_displayAndTitleRole()
{
    DlnaModel m;
    m.setItems({makeItem("1", "My Video", DlnaItemType::Video)});
    QModelIndex idx = m.index(0);
    QCOMPARE(m.data(idx, Qt::DisplayRole).toString(), "My Video");
    QCOMPARE(m.data(idx, DlnaModel::TitleRole).toString(), "My Video");
}

void TestDlnaModel::data_typeRole()
{
    DlnaModel m;
    m.setItems({makeItem("1", "A", DlnaItemType::Audio)});
    int t = m.data(m.index(0), DlnaModel::TypeRole).toInt();
    QCOMPARE(static_cast<DlnaItemType>(t), DlnaItemType::Audio);
}

void TestDlnaModel::data_mimeTypeRole()
{
    DlnaModel m;
    m.setItems({makeItem("1", "A", DlnaItemType::Video, "video/mp4")});
    QCOMPARE(m.data(m.index(0), DlnaModel::MimeTypeRole).toString(), "video/mp4");
}

void TestDlnaModel::data_resourceUrlRole()
{
    QUrl url("http://server/file.mp4");
    DlnaModel m;
    m.setItems({makeItem("1", "A", DlnaItemType::Video, "video/mp4", url)});
    QCOMPARE(m.data(m.index(0), DlnaModel::ResourceUrlRole).toUrl(), url);
}

void TestDlnaModel::data_itemIdRole()
{
    DlnaModel m;
    m.setItems({makeItem("item-99", "A", DlnaItemType::Video)});
    QCOMPARE(m.data(m.index(0), DlnaModel::ItemIdRole).toString(), "item-99");
}

void TestDlnaModel::data_dateRole()
{
    DlnaModel m;
    m.setItems({makeItem("1", "A", DlnaItemType::Video, {}, {}, "2024-06-01")});
    QCOMPARE(m.data(m.index(0), DlnaModel::DateRole).toString(), "2024-06-01");
}

void TestDlnaModel::data_invalidIndex()
{
    DlnaModel m;
    m.setItems({makeItem("1", "A", DlnaItemType::Video)});
    QVERIFY(!m.data(m.index(5)).isValid());
    QVERIFY(!m.data(QModelIndex()).isValid());
}

// ─── items() getter ───────────────────────────────────────────────────────────

void TestDlnaModel::items_getter()
{
    DlnaModel m;
    auto list = QList<DlnaItem>{makeItem("1", "A", DlnaItemType::Video),
                                makeItem("2", "B", DlnaItemType::Image)};
    m.setItems(list);
    QCOMPARE(m.items().size(), 2);
    QCOMPARE(m.items().at(0).id, "1");
    QCOMPARE(m.items().at(1).id, "2");
}

// ─── setThumbnail edge case ───────────────────────────────────────────────────

void TestDlnaModel::setThumbnail_outOfRange()
{
    DlnaModel m;
    m.setItems({makeItem("1", "A", DlnaItemType::Video)});
    // Should not crash
    m.setThumbnail(-1, QPixmap());
    m.setThumbnail(99, QPixmap());
    QVERIFY(!m.data(m.index(0), DlnaModel::ThumbnailRole).isValid());
}

// ─── appendItem ──────────────────────────────────────────────────────────────

void TestDlnaModel::appendItem_toEmpty()
{
    DlnaModel m;
    m.appendItem(makeItem("1", "A", DlnaItemType::Video));
    QCOMPARE(m.rowCount(), 1);
    QCOMPARE(m.itemAt(0).id, "1");
}

void TestDlnaModel::appendItem_toExisting()
{
    DlnaModel m;
    m.setItems({makeItem("1", "A", DlnaItemType::Video)});
    m.appendItem(makeItem("2", "B", DlnaItemType::Audio));
    QCOMPARE(m.rowCount(), 2);
    QCOMPARE(m.itemAt(1).id, "2");
    QCOMPARE(m.itemAt(0).id, "1");
}

QTEST_GUILESS_MAIN(TestDlnaModel)
