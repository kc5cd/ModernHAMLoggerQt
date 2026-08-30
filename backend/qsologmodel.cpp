#include "qsologmodel.h"

#include <QJsonObject>
#include <QTimeZone>

namespace {
// Frequencies are stored in MHz but entered and displayed in kHz.
constexpr double kKhzPerMhz = 1000.0;
const char *kTimeFormat = "yyyy-MM-dd hh:mm";
} // namespace

QsoLogModel::QsoLogModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int QsoLogModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_qsos.size();
}

int QsoLogModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return ColumnCount;
}

QVariant QsoLogModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_qsos.size())
        return {};
    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return {};

    const Qso &qso = m_qsos.at(index.row());
    switch (index.column()) {
    case TimeColumn:
        return qso.dateTimeUtc.toUTC().toString(kTimeFormat);
    case CallsignColumn:
        return qso.callsign;
    case BandColumn:
        return qso.band;
    case ModeColumn:
        return qso.mode;
    case FreqColumn:
        return qso.freqMhz > 0 ? QString::number(qRound(qso.freqMhz * kKhzPerMhz)) : QString();
    case RstSentColumn:
        return qso.rstSent;
    case RstRcvdColumn:
        return qso.rstRcvd;
    case NameColumn:
        return qso.name;
    case GridColumn:
        return qso.gridSquare;
    case CountryColumn:
        return qso.country;
    case NotesColumn:
        return qso.notes;
    default:
        return {};
    }
}

bool QsoLogModel::setCell(int row, int column, const QVariant &value)
{
    return setData(index(row, column), value, Qt::EditRole);
}

bool QsoLogModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_qsos.size())
        return false;
    if (role != Qt::EditRole && role != Qt::DisplayRole)
        return false;

    Qso &qso = m_qsos[index.row()];
    const QString text = value.toString().trimmed();

    switch (index.column()) {
    case TimeColumn: {
        QDateTime parsed = QDateTime::fromString(text, kTimeFormat);
        if (!parsed.isValid())
            return false;
        parsed.setTimeZone(QTimeZone::UTC);
        qso.dateTimeUtc = parsed;
        break;
    }
    case CallsignColumn:
        qso.callsign = text.toUpper();
        break;
    case BandColumn:
        qso.band = text;
        break;
    case ModeColumn:
        qso.mode = text.toUpper();
        break;
    case FreqColumn: {
        if (text.isEmpty()) {
            qso.freqMhz = 0.0;
            break;
        }
        bool ok = false;
        const double khz = text.toDouble(&ok);
        if (!ok || khz < 0)
            return false;
        qso.freqMhz = khz / kKhzPerMhz;
        break;
    }
    case RstSentColumn:
        qso.rstSent = text;
        break;
    case RstRcvdColumn:
        qso.rstRcvd = text;
        break;
    case NameColumn:
        qso.name = text;
        break;
    case GridColumn:
        qso.gridSquare = text.toUpper();
        break;
    case CountryColumn:
        qso.country = text;
        break;
    case NotesColumn:
        qso.notes = text;
        break;
    default:
        return false;
    }

    emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
    return true;
}

Qt::ItemFlags QsoLogModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}

QVariant QsoLogModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return QAbstractTableModel::headerData(section, orientation, role);

    switch (section) {
    case TimeColumn:
        return tr("Time (UTC)");
    case CallsignColumn:
        return tr("Call");
    case BandColumn:
        return tr("Band");
    case ModeColumn:
        return tr("Mode");
    case FreqColumn:
        return tr("Freq (kHz)");
    case RstSentColumn:
        return tr("RST S");
    case RstRcvdColumn:
        return tr("RST R");
    case NameColumn:
        return tr("Name");
    case GridColumn:
        return tr("Grid");
    case CountryColumn:
        return tr("Country");
    case NotesColumn:
        return tr("Notes");
    default:
        return {};
    }
}

void QsoLogModel::addQso(const Qso &qso)
{
    beginInsertRows(QModelIndex(), m_qsos.size(), m_qsos.size());
    m_qsos.append(qso);
    endInsertRows();
}

void QsoLogModel::removeAt(int row)
{
    removeRows(row, 1);
}

bool QsoLogModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid() || count <= 0 || row < 0 || row + count > m_qsos.size())
        return false;
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    m_qsos.remove(row, count);
    endRemoveRows();
    return true;
}

void QsoLogModel::setQsos(const QVector<Qso> &qsos)
{
    beginResetModel();
    m_qsos = qsos;
    endResetModel();
}

QJsonArray QsoLogModel::toJson() const
{
    QJsonArray array;
    for (const Qso &qso : m_qsos) {
        QJsonObject obj;
        obj["callsign"] = qso.callsign;
        obj["dateTimeUtc"] = qso.dateTimeUtc.toString(Qt::ISODate);
        obj["band"] = qso.band;
        obj["mode"] = qso.mode;
        obj["freqMhz"] = qso.freqMhz;
        obj["rstSent"] = qso.rstSent;
        obj["rstRcvd"] = qso.rstRcvd;
        obj["name"] = qso.name;
        obj["gridSquare"] = qso.gridSquare;
        obj["country"] = qso.country;
        obj["notes"] = qso.notes;
        array.append(obj);
    }
    return array;
}

QVector<Qso> QsoLogModel::fromJson(const QJsonArray &array)
{
    QVector<Qso> qsos;
    qsos.reserve(array.size());
    for (const QJsonValue &value : array) {
        const QJsonObject obj = value.toObject();
        Qso qso;
        qso.callsign = obj["callsign"].toString();
        qso.dateTimeUtc = QDateTime::fromString(obj["dateTimeUtc"].toString(), Qt::ISODate);
        qso.band = obj["band"].toString();
        qso.mode = obj["mode"].toString();
        qso.freqMhz = obj["freqMhz"].toDouble();
        qso.rstSent = obj["rstSent"].toString();
        qso.rstRcvd = obj["rstRcvd"].toString();
        qso.name = obj["name"].toString();
        qso.gridSquare = obj["gridSquare"].toString();
        qso.country = obj["country"].toString();
        qso.notes = obj["notes"].toString();
        qsos.append(qso);
    }
    return qsos;
}
