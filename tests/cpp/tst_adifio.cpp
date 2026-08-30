#include <QtTest>

#include <QDateTime>
#include <QFile>
#include <QSettings>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTimeZone>

#include "adifio.h"
#include "operation.h"
#include "qso.h"
#include "stationprofile.h"

namespace {

// Qso has no operator== -- compare field-by-field so a failure points at
// exactly which field diverged rather than just "not equal".
void compareQso(const Qso &actual, const Qso &expected)
{
    QCOMPARE(actual.callsign, expected.callsign);
    QCOMPARE(actual.dateTimeUtc, expected.dateTimeUtc);
    QCOMPARE(actual.band, expected.band);
    QCOMPARE(actual.mode, expected.mode);
    QVERIFY(qAbs(actual.freqMhz - expected.freqMhz) < 1e-6);
    QCOMPARE(actual.rstSent, expected.rstSent);
    QCOMPARE(actual.rstRcvd, expected.rstRcvd);
    QCOMPARE(actual.name, expected.name);
    QCOMPARE(actual.gridSquare, expected.gridSquare);
    QCOMPARE(actual.country, expected.country);
    QCOMPARE(actual.notes, expected.notes);
}

bool writeBytes(const QString &path, const QByteArray &bytes)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly))
        return false;
    return file.write(bytes) == bytes.size();
}

} // namespace

class TstAdifIo : public QObject
{
    Q_OBJECT

private slots:
    void parseMinimalRecord();
    void parseFileWithNoHeader();
    void recordMissingCallIsDropped();
    void commentContainingEorLikeTextIsNotMisparsed();
    void nonAsciiValueUsesByteLength();
    void shortDateFallsBackToCurrentTime();
    void writeThenParseRoundTrip();
};

void TstAdifIo::parseMinimalRecord()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("minimal.adi");

    QVERIFY(writeBytes(path,
        "Sample header\n"
        "<ADIF_VER:5>3.1.4\n"
        "<EOH>\n\n"
        "<CALL:5>W1ABC<QSO_DATE:8>20250615<TIME_ON:6>143000"
        "<BAND:3>20m<MODE:3>SSB<EOR>\n\n"));

    const QVector<Qso> qsos = AdifIo::parseFile(path);
    QCOMPARE(qsos.size(), 1);
    QCOMPARE(qsos.first().callsign, QStringLiteral("W1ABC"));
    QCOMPARE(qsos.first().band, QStringLiteral("20m"));
    QCOMPARE(qsos.first().mode, QStringLiteral("SSB"));
    QCOMPARE(qsos.first().dateTimeUtc,
        QDateTime(QDate(2025, 6, 15), QTime(14, 30, 0), QTimeZone::UTC));
}

void TstAdifIo::parseFileWithNoHeader()
{
    // No <EOH> at all -- the parser must fall back to scanning from offset 0
    // instead of dropping every record while it waits for a header that
    // never arrives.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("noheader.adi");

    QVERIFY(writeBytes(path, "<CALL:5>W2XYZ<QSO_DATE:8>20250101<TIME_ON:4>0930<EOR>\n"));

    const QVector<Qso> qsos = AdifIo::parseFile(path);
    QCOMPARE(qsos.size(), 1);
    QCOMPARE(qsos.first().callsign, QStringLiteral("W2XYZ"));
}

void TstAdifIo::recordMissingCallIsDropped()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("nocall.adi");

    QVERIFY(writeBytes(path, "<BAND:3>20m<MODE:3>SSB<EOR>\n"));

    const QVector<Qso> qsos = AdifIo::parseFile(path);
    QCOMPARE(qsos.size(), 0);
}

void TstAdifIo::commentContainingEorLikeTextIsNotMisparsed()
{
    // The byte-exact scanner must skip exactly the declared length of
    // COMMENT, so a literal "<eor>" or "<word:digits>" substring inside a
    // user-entered note is never misread as real structure.
    const QString comment = QStringLiteral("contains <eor> and <fake:3> inside");
    const QByteArray commentBytes = comment.toUtf8();

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("comment.adi");

    QByteArray body = "<CALL:5>W3AAA<QSO_DATE:8>20250301<TIME_ON:4>1200";
    body += "<COMMENT:" + QByteArray::number(commentBytes.size()) + ">" + commentBytes;
    body += "<EOR>\n";
    QVERIFY(writeBytes(path, body));

    const QVector<Qso> qsos = AdifIo::parseFile(path);
    QCOMPARE(qsos.size(), 1);
    QCOMPARE(qsos.first().callsign, QStringLiteral("W3AAA"));
    QCOMPARE(qsos.first().notes, comment);
}

void TstAdifIo::nonAsciiValueUsesByteLength()
{
    // ADIF lengths are byte counts, not UTF-16 code units -- "é" is one
    // QChar but two UTF-8 bytes, so a value using it must be declared with
    // its byte length or the field boundary would be wrong.
    const QString name = QStringLiteral("José");
    const QByteArray nameBytes = name.toUtf8();
    QCOMPARE(nameBytes.size(), 5); // "Jos" (3) + 2-byte 'é'

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("utf8.adi");

    QByteArray body = "<CALL:5>W4BBB<QSO_DATE:8>20250401<TIME_ON:4>0800";
    body += "<NAME:" + QByteArray::number(nameBytes.size()) + ">" + nameBytes;
    body += "<EOR>\n";
    QVERIFY(writeBytes(path, body));

    const QVector<Qso> qsos = AdifIo::parseFile(path);
    QCOMPARE(qsos.size(), 1);
    QCOMPARE(qsos.first().name, name);
}

void TstAdifIo::shortDateFallsBackToCurrentTime()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("shortdate.adi");

    // No QSO_DATE tag at all -- fields.value("QSO_DATE") comes back empty
    // (size 0 < 8), triggering the documented fallback.
    QVERIFY(writeBytes(path, "<CALL:5>W5CCC<EOR>\n"));

    const QDateTime before = QDateTime::currentDateTimeUtc();
    const QVector<Qso> qsos = AdifIo::parseFile(path);
    const QDateTime after = QDateTime::currentDateTimeUtc();

    QCOMPARE(qsos.size(), 1);
    QVERIFY(qsos.first().dateTimeUtc.isValid());
    QVERIFY(qsos.first().dateTimeUtc >= before.addSecs(-1));
    QVERIFY(qsos.first().dateTimeUtc <= after.addSecs(1));
}

void TstAdifIo::writeThenParseRoundTrip()
{
    QStandardPaths::setTestModeEnabled(true);
    QSettings::setDefaultFormat(QSettings::IniFormat);

    auto *operation = new Operation();
    operation->setName("Round Trip Test");
    operation->setPotaRef("K-1234");

    Qso qso;
    qso.callsign = "K5TEST";
    qso.dateTimeUtc = QDateTime(QDate(2025, 7, 4), QTime(18, 30, 15), QTimeZone::UTC);
    qso.band = "40m";
    qso.mode = "CW";
    qso.freqMhz = 7.030000;
    qso.rstSent = "599";
    qso.rstRcvd = "579";
    qso.name = "Alice";
    qso.gridSquare = "EM12";
    qso.country = "USA";
    qso.notes = "Test contact";
    operation->log()->addQso(qso);

    auto *profile = new StationProfile();
    profile->setCallsign("W0BASE");
    profile->setGridSquare("EM10");

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("roundtrip.adi");

    QVERIFY(AdifIo::writeFile(path, operation, profile));

    const QVector<Qso> parsed = AdifIo::parseFile(path);
    QCOMPARE(parsed.size(), 1);
    compareQso(parsed.first(), qso);

    delete profile;
    delete operation;
}

QTEST_GUILESS_MAIN(TstAdifIo)
#include "tst_adifio.moc"
