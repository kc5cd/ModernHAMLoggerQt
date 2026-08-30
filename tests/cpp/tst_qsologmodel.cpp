#include <QtTest>

#include <QDateTime>
#include <QTimeZone>

#include "qsologmodel.h"
#include "qso.h"

namespace {

Qso makeQso()
{
    Qso qso;
    qso.callsign = "N0CALL";
    qso.dateTimeUtc = QDateTime(QDate(2025, 5, 20), QTime(10, 0, 0), QTimeZone::UTC);
    qso.band = "20m";
    qso.mode = "FT8";
    qso.freqMhz = 14.074;
    qso.rstSent = "599";
    qso.rstRcvd = "599";
    qso.name = "Bob";
    qso.gridSquare = "DM13";
    qso.country = "USA";
    qso.notes = "Test";
    return qso;
}

} // namespace

class TstQsoLogModel : public QObject
{
    Q_OBJECT

private slots:
    void rowAndColumnCounts();
    void dataReturnsExpectedColumns();
    void setCellUppercasesCallsign();
    void setCellUppercasesMode();
    void cellTextMatchesDisplayRole();
    void jsonRoundTrip();
    void removeAtOutOfRangeIsNoOp();
};

void TstQsoLogModel::rowAndColumnCounts()
{
    QsoLogModel model;
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.columnCount(), int(QsoLogModel::ColumnCount));

    model.addQso(makeQso());
    model.addQso(makeQso());
    QCOMPARE(model.rowCount(), 2);
}

void TstQsoLogModel::dataReturnsExpectedColumns()
{
    QsoLogModel model;
    model.addQso(makeQso());

    QCOMPARE(model.data(model.index(0, QsoLogModel::CallsignColumn)).toString(),
        QStringLiteral("N0CALL"));
    QCOMPARE(model.data(model.index(0, QsoLogModel::BandColumn)).toString(),
        QStringLiteral("20m"));
    QCOMPARE(model.data(model.index(0, QsoLogModel::TimeColumn)).toString(),
        QStringLiteral("2025-05-20 10:00"));
}

void TstQsoLogModel::setCellUppercasesCallsign()
{
    QsoLogModel model;
    model.addQso(makeQso());

    QVERIFY(model.setCell(0, QsoLogModel::CallsignColumn, QStringLiteral("w1abc")));
    QCOMPARE(model.data(model.index(0, QsoLogModel::CallsignColumn)).toString(),
        QStringLiteral("W1ABC"));
}

void TstQsoLogModel::setCellUppercasesMode()
{
    QsoLogModel model;
    model.addQso(makeQso());

    QVERIFY(model.setCell(0, QsoLogModel::ModeColumn, QStringLiteral("ssb")));
    QCOMPARE(model.data(model.index(0, QsoLogModel::ModeColumn)).toString(),
        QStringLiteral("SSB"));
}

void TstQsoLogModel::cellTextMatchesDisplayRole()
{
    QsoLogModel model;
    model.addQso(makeQso());

    for (int column = 0; column < QsoLogModel::ColumnCount; ++column) {
        QCOMPARE(model.cellText(0, column),
            model.data(model.index(0, column), Qt::DisplayRole).toString());
    }
}

void TstQsoLogModel::jsonRoundTrip()
{
    QsoLogModel model;
    model.addQso(makeQso());

    const QVector<Qso> restored = QsoLogModel::fromJson(model.toJson());
    QCOMPARE(restored.size(), 1);
    QCOMPARE(restored.first().callsign, QStringLiteral("N0CALL"));
    QCOMPARE(restored.first().band, QStringLiteral("20m"));
    QCOMPARE(restored.first().dateTimeUtc, makeQso().dateTimeUtc);
}

void TstQsoLogModel::removeAtOutOfRangeIsNoOp()
{
    QsoLogModel model;
    model.addQso(makeQso());

    model.removeAt(5); // out of range
    QCOMPARE(model.rowCount(), 1);

    model.removeAt(-1); // out of range
    QCOMPARE(model.rowCount(), 1);
}

QTEST_GUILESS_MAIN(TstQsoLogModel)
#include "tst_qsologmodel.moc"
