#include <QtTest>

#include <QFile>
#include <QSettings>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QUrl>

#include "adifio.h"
#include "logbookmanager.h"
#include "operation.h"
#include "qsologmodel.h"

class TstLogbookManager : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    void addOperationPersistsAcrossInstances();
    void deleteOperationBeforeCurrentIndexDecrementsIndex();
    void deleteOperationAtCurrentIndexClampsToNewCount();
    void deleteOperationDeletingLastRemainingClearsSelection();
    void deleteQsosReturnsCountRemovedAndSkipsOutOfRange();
    void importExportAdifRoundTrip();

private:
    static QString stateFilePath();
};

void TstLogbookManager::initTestCase()
{
    // Must happen before the first LogbookManager/StationProfile is
    // constructed anywhere in the process: both read state in their
    // constructors. setTestModeEnabled redirects AppDataLocation, but
    // QSettings defaults to the registry on Windows, which it does NOT
    // redirect -- force IniFormat so StationProfile stays sandboxed too.
    QStandardPaths::setTestModeEnabled(true);
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QCoreApplication::setOrganizationName("ModernHAMLoggerQt");
    QCoreApplication::setApplicationName("ModernHAMLoggerQtTest");
}

void TstLogbookManager::cleanup()
{
    QFile::remove(stateFilePath());
}

QString TstLogbookManager::stateFilePath()
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return dir + "/logbook.json";
}

void TstLogbookManager::addOperationPersistsAcrossInstances()
{
    {
        LogbookManager manager;
        manager.addOperation("POTA Activation", "K-1234", QString());
    }

    LogbookManager reloaded;
    QCOMPARE(reloaded.operations()->count(), 1);
    QCOMPARE(reloaded.operations()->at(0)->name(), QStringLiteral("POTA Activation"));
    QCOMPARE(reloaded.operations()->at(0)->potaRef(), QStringLiteral("K-1234"));
}

void TstLogbookManager::deleteOperationBeforeCurrentIndexDecrementsIndex()
{
    // Regression test for ebfa9e0's deleteOperation index bookkeeping.
    LogbookManager manager;
    manager.addOperation("First", QString(), QString());
    manager.addOperation("Second", QString(), QString());
    manager.addOperation("Third", QString(), QString());
    manager.selectOperation(2); // "Third"

    manager.deleteOperation(0); // delete "First", before the current index

    QCOMPARE(manager.currentOperationIndex(), 1); // shifted down, still pointing at "Third"
    QCOMPARE(manager.currentOperation()->name(), QStringLiteral("Third"));
}

void TstLogbookManager::deleteOperationAtCurrentIndexClampsToNewCount()
{
    LogbookManager manager;
    manager.addOperation("First", QString(), QString());
    manager.addOperation("Second", QString(), QString());
    manager.selectOperation(1); // "Second", the last one

    manager.deleteOperation(1); // delete the current, and last, operation

    QCOMPARE(manager.operations()->count(), 1);
    QCOMPARE(manager.currentOperationIndex(), 0);
    QCOMPARE(manager.currentOperation()->name(), QStringLiteral("First"));
}

void TstLogbookManager::deleteOperationDeletingLastRemainingClearsSelection()
{
    LogbookManager manager;
    manager.addOperation("Only", QString(), QString());

    manager.deleteOperation(0);

    QCOMPARE(manager.operations()->count(), 0);
    QCOMPARE(manager.currentOperationIndex(), -1);
    QVERIFY(manager.currentOperation() == nullptr);
}

void TstLogbookManager::deleteQsosReturnsCountRemovedAndSkipsOutOfRange()
{
    LogbookManager manager;
    manager.addOperation("Contest", QString(), QString());

    for (int i = 0; i < 5; ++i)
        manager.logQso({{"callsign", QStringLiteral("N0CALL%1").arg(i)}});

    const int removed = manager.deleteQsos(0, {1, 3, 99, -1});
    QCOMPARE(removed, 2); // 99 and -1 are out of range and skipped
    QCOMPARE(manager.currentOperation()->log()->rowCount(), 3);
}

void TstLogbookManager::importExportAdifRoundTrip()
{
    LogbookManager manager;
    manager.addOperation("Export Source", QString(), QString());
    manager.logQso({
        {"callsign", "W6TEST"},
        {"band", "15m"},
        {"mode", "SSB"},
    });

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("export.adi");
    const QUrl fileUrl = QUrl::fromLocalFile(path);

    QVERIFY(manager.exportAdif(fileUrl, 0));
    QVERIFY(manager.importAdif(fileUrl));

    QCOMPARE(manager.operations()->count(), 2); // original + reimported
    Operation *imported = manager.operations()->at(1);
    QCOMPARE(imported->log()->rowCount(), 1);
    QCOMPARE(imported->log()->cellText(0, QsoLogModel::CallsignColumn), QStringLiteral("W6TEST"));
}

QTEST_GUILESS_MAIN(TstLogbookManager)
#include "tst_logbookmanager.moc"
