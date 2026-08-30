#include "adifio.h"

#include <QFile>
#include <QMap>
#include <QRegularExpression>
#include <QTextStream>
#include <QTimeZone>

namespace {

// ADIF has no escaping mechanism, so a value must never contain '<' (it would
// be misread as the start of a tag). This app also only ever writes
// single-line values, so a stray CR/LF is stripped defensively rather than
// silently shifting the byte-length accounting below.
QString sanitizeAdifValue(QString value)
{
    value.remove(QLatin1Char('<'));
    value.remove(QLatin1Char('>'));
    value.remove(QLatin1Char('\r'));
    value.remove(QLatin1Char('\n'));
    return value;
}

// Matches one ADIF tag: <NAME:LEN> or <NAME:LEN:TYPE> (a valued field), or a
// bare <NAME> such as <EOR>/<EOH> which carries no length.
//
// No leading '^': the scanner anchors the match with AnchorAtOffsetMatchOption
// at the call site instead. '^' anchors to the absolute start of the *whole*
// subject string (offset 0), not to the offset passed to match() -- combined
// with AnchorAtOffsetMatchOption that would make every tag past position 0
// fail to match.
const QRegularExpression &tagPattern()
{
    static const QRegularExpression re(R"(<([A-Za-z_]+)(?::(\d+)(?::[^:>]*)?)?>)");
    return re;
}

// Byte-exact, forward-scanning ADIF record parser. Walks tags in the order
// they appear and skips each value by its declared *byte* length, so a value
// that happens to contain "<eor>" or a "<word:digits>"-shaped substring can
// never be misread as a real delimiter or field -- unlike the previous
// split-on-<eor>-then-regex approach, which read such content out of a
// user-entered value (e.g. Notes) as structure.
//
// Structural scanning runs against a Latin-1 decode of the raw UTF-8 bytes.
// Latin-1 maps one QChar per byte, so string indices from the regex/search
// below stay numerically identical to byte offsets in `bytes` -- essential,
// since field values are sliced out of `bytes` by declared byte length, not
// by character count. Extracted values are then decoded with fromUtf8 to
// recover the real text.
QVector<QMap<QString, QString>> scanAdifRecords(const QByteArray &bytes)
{
    const QString structural = QString::fromLatin1(bytes);
    QVector<QMap<QString, QString>> records;
    QMap<QString, QString> current;

    qsizetype pos = 0;
    while (pos < structural.size()) {
        const qsizetype lt = structural.indexOf(QLatin1Char('<'), pos);
        if (lt < 0)
            break;

        const QRegularExpressionMatch match = tagPattern().match(
            structural, lt, QRegularExpression::NormalMatch, QRegularExpression::AnchorAtOffsetMatchOption);
        if (!match.hasMatch()) {
            // A '<' that isn't a recognized tag opener -- ordinary content
            // (or an unsanitized file from another logger); keep scanning.
            pos = lt + 1;
            continue;
        }

        const QString name = match.captured(1).toUpper();
        const qsizetype tagEnd = lt + match.capturedLength();

        if (name == QLatin1String("EOR")) {
            if (current.contains(QStringLiteral("CALL")))
                records.append(current);
            current.clear();
            pos = tagEnd;
            continue;
        }

        if (match.capturedLength(2) > 0) {
            bool ok = false;
            const qsizetype len = match.captured(2).toLongLong(&ok);
            if (ok && len >= 0 && tagEnd + len <= bytes.size()) {
                current[name] = QString::fromUtf8(bytes.mid(tagEnd, len));
                pos = tagEnd + len;
                continue;
            }
        }

        // Bare tag (e.g. a stray <EOH> in the body) or a malformed length --
        // skip just the tag itself and keep scanning.
        pos = tagEnd;
    }

    if (!current.isEmpty() && current.contains(QStringLiteral("CALL")))
        records.append(current);

    return records;
}

QDateTime parseAdifDateTime(const QString &date, const QString &time)
{
    if (date.size() < 8)
        return QDateTime::currentDateTimeUtc();

    const int year = date.mid(0, 4).toInt();
    const int month = date.mid(4, 2).toInt();
    const int day = date.mid(6, 2).toInt();

    int hour = 0, minute = 0, second = 0;
    if (time.size() >= 4) {
        hour = time.mid(0, 2).toInt();
        minute = time.mid(2, 2).toInt();
    }
    if (time.size() >= 6)
        second = time.mid(4, 2).toInt();

    return QDateTime(QDate(year, month, day), QTime(hour, minute, second), QTimeZone::UTC);
}

QString adifField(const QString &name, const QString &value)
{
    // ADIF field lengths are byte counts, not UTF-16 code units -- using
    // value.length() here under-declares the length for any non-ASCII
    // character, since the file is written as UTF-8, causing other readers
    // to truncate mid-field and misparse the rest of the record.
    const QString sanitized = sanitizeAdifValue(value);
    return QStringLiteral("<%1:%2>%3\n").arg(name).arg(sanitized.toUtf8().size()).arg(sanitized);
}

void writeOperationRecords(QTextStream &out, Operation *operation, StationProfile *profile)
{
    const QString stationCallsign = profile ? profile->callsign() : QString();
    const QString myGrid = profile ? profile->gridSquare() : QString();

    for (const Qso &qso : operation->log()->qsos()) {
        out << adifField("CALL", qso.callsign);
        out << adifField("QSO_DATE", qso.dateTimeUtc.toUTC().toString("yyyyMMdd"));
        out << adifField("TIME_ON", qso.dateTimeUtc.toUTC().toString("hhmmss"));
        if (!qso.band.isEmpty())
            out << adifField("BAND", qso.band);
        if (!qso.mode.isEmpty())
            out << adifField("MODE", qso.mode);
        if (qso.freqMhz > 0)
            out << adifField("FREQ", QString::number(qso.freqMhz, 'f', 6));
        if (!qso.rstSent.isEmpty())
            out << adifField("RST_SENT", qso.rstSent);
        if (!qso.rstRcvd.isEmpty())
            out << adifField("RST_RCVD", qso.rstRcvd);
        if (!qso.name.isEmpty())
            out << adifField("NAME", qso.name);
        if (!qso.gridSquare.isEmpty())
            out << adifField("GRIDSQUARE", qso.gridSquare);
        if (!qso.country.isEmpty())
            out << adifField("COUNTRY", qso.country);
        if (!qso.notes.isEmpty())
            out << adifField("COMMENT", qso.notes);
        if (!stationCallsign.isEmpty())
            out << adifField("STATION_CALLSIGN", stationCallsign);
        if (!myGrid.isEmpty())
            out << adifField("MY_GRIDSQUARE", myGrid);
        if (!operation->potaRef().isEmpty()) {
            out << adifField("SIG", "POTA");
            out << adifField("SIG_INFO", operation->potaRef());
        }
        if (!operation->sotaRef().isEmpty())
            out << adifField("SOTA_REF", operation->sotaRef());
        out << "<EOR>\n\n";
    }
}

} // namespace

QVector<Qso> AdifIo::parseFile(const QString &path)
{
    QVector<Qso> result;

    // Read as raw bytes, not through QIODevice::Text: the byte-exact scanner
    // below computes offsets directly against this buffer, so as long as
    // structural scanning and value extraction use the same buffer, any
    // newline translation is self-consistent -- but there is no reason to
    // apply one here at all.
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return result;
    const QByteArray fileBytes = file.readAll();
    file.close();

    // The header (ADIF_VER, PROGRAMID, ...) is well-formed and not
    // user-controlled, so a simple case-insensitive search for its
    // terminator is safe; only the body -- which can contain arbitrary
    // COMMENT text -- needs the byte-exact scanner in scanAdifRecords().
    const QString headerView = QString::fromLatin1(fileBytes);
    static const QRegularExpression eohRe("<eoh>", QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch eohMatch = eohRe.match(headerView);
    const qsizetype bodyStart = eohMatch.hasMatch() ? eohMatch.capturedEnd() : 0;
    const QByteArray body = fileBytes.mid(bodyStart);

    const QVector<QMap<QString, QString>> records = scanAdifRecords(body);
    result.reserve(records.size());
    for (const QMap<QString, QString> &fields : records) {
        Qso qso;
        qso.callsign = fields.value("CALL").toUpper();
        qso.dateTimeUtc = parseAdifDateTime(fields.value("QSO_DATE"), fields.value("TIME_ON"));
        qso.band = fields.value("BAND");
        qso.mode = fields.value("MODE");
        qso.freqMhz = fields.value("FREQ").toDouble();
        qso.rstSent = fields.value("RST_SENT");
        qso.rstRcvd = fields.value("RST_RCVD");
        qso.name = fields.value("NAME");
        qso.gridSquare = fields.value("GRIDSQUARE");
        qso.country = fields.value("COUNTRY");
        qso.notes = fields.value("COMMENT");
        result.append(qso);
    }

    return result;
}

bool AdifIo::writeFile(const QString &path, Operation *operation, StationProfile *profile)
{
    if (!operation)
        return false;
    return writeFile(path, QVector<Operation *>{operation}, profile);
}

bool AdifIo::writeFile(const QString &path, const QVector<Operation *> &operations, StationProfile *profile)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&file);
    out << "ADIF export from ModernHAMLoggerQt\n";
    out << adifField("ADIF_VER", "3.1.4");
    out << adifField("PROGRAMID", "ModernHAMLoggerQt");
    out << "<EOH>\n\n";

    for (Operation *operation : operations) {
        if (operation)
            writeOperationRecords(out, operation, profile);
    }

    return true;
}
