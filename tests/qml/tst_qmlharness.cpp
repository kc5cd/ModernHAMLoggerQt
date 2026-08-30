#include <QtQuickTest>

#include <QObject>
#include <QSettings>
#include <QStandardPaths>

// A QML test that touches the LogbookManager singleton would otherwise load
// (and could overwrite) the real user logbook, exactly as in the C++ tests
// -- isolate before the engine is even created.
class Setup : public QObject
{
    Q_OBJECT

public slots:
    void applicationAvailable()
    {
        QStandardPaths::setTestModeEnabled(true);
        QSettings::setDefaultFormat(QSettings::IniFormat);
        QCoreApplication::setOrganizationName("ModernHAMLoggerQt");
        QCoreApplication::setApplicationName("ModernHAMLoggerQtTest");
    }
};

QUICK_TEST_MAIN_WITH_SETUP(mhl_qml_tests, Setup)

#include "tst_qmlharness.moc"
