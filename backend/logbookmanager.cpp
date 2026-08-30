#include "logbookmanager.h"

#include <algorithm>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

#include "adifio.h"

LogbookManager::LogbookManager(QObject *parent)
    : QObject(parent)
    , m_operations(new OperationListModel(this))
    , m_profile(new StationProfile(this))
{
    loadState();
}

LogbookManager::~LogbookManager() = default;

Operation *LogbookManager::currentOperation() const
{
    return m_operations->at(m_currentOperationIndex);
}

void LogbookManager::registerOperation(Operation *operation)
{
    m_operations->append(operation);
    connect(operation->log(), &QAbstractItemModel::dataChanged, this, [this]() { saveState(); });
}

void LogbookManager::addOperation(const QString &name, const QString &potaRef, const QString &sotaRef)
{
    auto *operation = new Operation(m_operations);
    operation->setName(name);
    operation->setPotaRef(potaRef);
    operation->setSotaRef(sotaRef);
    registerOperation(operation);

    m_currentOperationIndex = m_operations->count() - 1;
    emit currentOperationIndexChanged();
    saveState();
}

void LogbookManager::selectOperation(int index)
{
    if (index < 0 || index >= m_operations->count() || index == m_currentOperationIndex)
        return;
    m_currentOperationIndex = index;
    emit currentOperationIndexChanged();
    saveState();
}

void LogbookManager::deleteOperation(int index)
{
    if (index < 0 || index >= m_operations->count())
        return;

    m_operations->removeAt(index);

    // Keep the selection pointed at the same operation it was on, unless
    // that's the one just deleted -- then fall back to whatever now
    // occupies its slot (or the new last item, or none if the list is
    // empty), mirroring the clamp loadState() does for a stale saved index.
    if (index < m_currentOperationIndex)
        --m_currentOperationIndex;
    else if (index == m_currentOperationIndex)
        m_currentOperationIndex = std::min(m_currentOperationIndex, m_operations->count() - 1);

    emit currentOperationIndexChanged();
    saveState();
}

void LogbookManager::logQso(const QVariantMap &fields)
{
    Operation *operation = currentOperation();
    if (!operation)
        return;

    Qso qso;
    qso.callsign = fields.value("callsign").toString();
    qso.dateTimeUtc = QDateTime::currentDateTimeUtc();
    qso.band = fields.value("band").toString();
    qso.mode = fields.value("mode").toString();
    qso.freqMhz = fields.value("freqMhz").toDouble();
    qso.rstSent = fields.value("rstSent").toString();
    qso.rstRcvd = fields.value("rstRcvd").toString();
    qso.name = fields.value("name").toString();
    qso.gridSquare = fields.value("gridSquare").toString();
    qso.country = fields.value("country").toString();
    qso.notes = fields.value("notes").toString();

    operation->log()->addQso(qso);
    saveState();
}

void LogbookManager::deleteQso(int opIndex, int row)
{
    deleteQsos(opIndex, {row});
}

int LogbookManager::deleteQsos(int opIndex, const QList<int> &rows)
{
    Operation *operation = m_operations->at(opIndex);
    if (!operation || rows.isEmpty())
        return 0;

    QsoLogModel *log = operation->log();

    // Descending, deduplicated: removing one row can never shift an index
    // still pending, and sorted duplicates become adjacent for std::unique.
    QList<int> targets = rows;
    std::sort(targets.begin(), targets.end(), std::greater<int>());
    targets.erase(std::unique(targets.begin(), targets.end()), targets.end());

    int removed = 0;
    for (int i = 0; i < targets.size();) {
        const int last = targets.at(i);
        if (last < 0 || last >= log->rowCount()) {
            ++i;
            continue;
        }
        int j = i;
        while (j + 1 < targets.size() && targets.at(j + 1) == targets.at(j) - 1)
            ++j; // coalesce a contiguous descending run into one removeRows() call
        const int first = targets.at(j);
        const int count = last - first + 1;
        if (log->removeRows(first, count))
            removed += count;
        i = j + 1;
    }

    // saveState() is wired only to dataChanged (see registerOperation), so row
    // removals must persist explicitly or the deletion is lost on restart.
    if (removed > 0)
        saveState();
    return removed;
}

bool LogbookManager::importAdif(const QUrl &fileUrl)
{
    const QString path = fileUrl.toLocalFile();
    if (path.isEmpty())
        return false;

    const QVector<Qso> qsos = AdifIo::parseFile(path);
    if (qsos.isEmpty())
        return false;

    auto *operation = new Operation(m_operations);
    operation->setName(QFileInfo(path).completeBaseName());
    operation->log()->setQsos(qsos);
    registerOperation(operation);

    m_currentOperationIndex = m_operations->count() - 1;
    emit currentOperationIndexChanged();
    saveState();
    return true;
}

bool LogbookManager::exportAdif(const QUrl &fileUrl, int opIndex)
{
    const QString path = fileUrl.toLocalFile();
    if (path.isEmpty())
        return false;

    if (opIndex == -1)
        return AdifIo::writeFile(path, m_operations->operations(), m_profile);

    Operation *operation = m_operations->at(opIndex);
    if (!operation)
        return false;
    return AdifIo::writeFile(path, operation, m_profile);
}

QString LogbookManager::stateFilePath() const
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return dir + "/logbook.json";
}

void LogbookManager::loadState()
{
    QFile file(stateFilePath());
    if (!file.open(QIODevice::ReadOnly))
        return;

    const QJsonObject root = QJsonDocument::fromJson(file.readAll()).object();
    const QJsonArray operationsArray = root.value("operations").toArray();
    for (const QJsonValue &value : operationsArray)
        registerOperation(Operation::fromJson(value.toObject(), m_operations));

    m_currentOperationIndex = root.value("currentOperationIndex").toInt(-1);
    if (m_currentOperationIndex >= m_operations->count())
        m_currentOperationIndex = m_operations->count() - 1;
}

void LogbookManager::saveState() const
{
    QJsonArray operationsArray;
    for (Operation *operation : m_operations->operations())
        operationsArray.append(operation->toJson());

    QJsonObject root;
    root["operations"] = operationsArray;
    root["currentOperationIndex"] = m_currentOperationIndex;

    QFile file(stateFilePath());
    if (!file.open(QIODevice::WriteOnly))
        return;
    file.write(QJsonDocument(root).toJson());
}
