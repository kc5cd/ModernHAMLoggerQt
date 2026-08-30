#include <QtTest>

#include "operation.h"
#include "operationlistmodel.h"
#include "qso.h"
#include "qsologmodel.h"

class TstOperationListModel : public QObject
{
    Q_OBJECT

private slots:
    void appendEmitsRowsInserted();
    void removeAtEmitsRowsRemoved();
    void removeAtOutOfRangeIsNoOp();
    void qsoCountChangedReportsCorrectRowAfterEarlierRemoval();
};

void TstOperationListModel::appendEmitsRowsInserted()
{
    OperationListModel model;
    QSignalSpy spy(&model, &QAbstractItemModel::rowsInserted);

    auto *operation = new Operation();
    operation->setName("Field Day");
    model.append(operation);

    QCOMPARE(model.count(), 1);
    QCOMPARE(spy.count(), 1);
}

void TstOperationListModel::removeAtEmitsRowsRemoved()
{
    OperationListModel model;
    model.append(new Operation());
    model.append(new Operation());

    QSignalSpy spy(&model, &QAbstractItemModel::rowsRemoved);
    model.removeAt(0);

    QCOMPARE(model.count(), 1);
    QCOMPARE(spy.count(), 1);
}

void TstOperationListModel::removeAtOutOfRangeIsNoOp()
{
    OperationListModel model;
    model.append(new Operation());

    model.removeAt(5);
    QCOMPARE(model.count(), 1);

    model.removeAt(-1);
    QCOMPARE(model.count(), 1);
}

void TstOperationListModel::qsoCountChangedReportsCorrectRowAfterEarlierRemoval()
{
    // Regression test for the bug fixed in ebfa9e0: append()'s
    // qsoCountChanged handler used to capture the row by value, so removing
    // an earlier operation left later operations' handlers pointing at a
    // stale row. It must resolve the row via indexOf() at emit time instead.
    OperationListModel model;
    auto *first = new Operation();
    auto *second = new Operation();
    auto *third = new Operation();
    model.append(first);
    model.append(second);
    model.append(third);

    model.removeAt(0); // first is gone; second is now row 0, third is row 1

    QSignalSpy spy(&model, &QAbstractItemModel::dataChanged);
    third->log()->addQso(Qso{}); // triggers qsoCountChanged on third

    QCOMPARE(spy.count(), 1);
    const QModelIndex topLeft = spy.first().at(0).value<QModelIndex>();
    QCOMPARE(topLeft.row(), 1); // third's current row, not its stale original row (2)
}

QTEST_GUILESS_MAIN(TstOperationListModel)
#include "tst_operationlistmodel.moc"
