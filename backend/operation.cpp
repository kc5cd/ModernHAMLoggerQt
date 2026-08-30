#include "operation.h"

Operation::Operation(QObject *parent)
    : QObject(parent)
    , m_createdUtc(QDateTime::currentDateTimeUtc())
    , m_log(new QsoLogModel(this))
{
    connect(m_log, &QAbstractItemModel::rowsInserted, this, &Operation::qsoCountChanged);
    connect(m_log, &QAbstractItemModel::rowsRemoved, this, &Operation::qsoCountChanged);
    connect(m_log, &QAbstractItemModel::modelReset, this, &Operation::qsoCountChanged);
}

void Operation::setName(const QString &name)
{
    if (m_name == name)
        return;
    m_name = name;
    emit nameChanged();
}

void Operation::setPotaRef(const QString &ref)
{
    if (m_potaRef == ref)
        return;
    m_potaRef = ref;
    emit potaRefChanged();
}

void Operation::setSotaRef(const QString &ref)
{
    if (m_sotaRef == ref)
        return;
    m_sotaRef = ref;
    emit sotaRefChanged();
}

QJsonObject Operation::toJson() const
{
    QJsonObject obj;
    obj["name"] = m_name;
    obj["potaRef"] = m_potaRef;
    obj["sotaRef"] = m_sotaRef;
    obj["createdUtc"] = m_createdUtc.toString(Qt::ISODate);
    obj["qsos"] = m_log->toJson();
    return obj;
}

Operation *Operation::fromJson(const QJsonObject &obj, QObject *parent)
{
    auto *operation = new Operation(parent);
    operation->m_name = obj["name"].toString();
    operation->m_potaRef = obj["potaRef"].toString();
    operation->m_sotaRef = obj["sotaRef"].toString();
    operation->m_createdUtc = QDateTime::fromString(obj["createdUtc"].toString(), Qt::ISODate);
    operation->m_log->setQsos(QsoLogModel::fromJson(obj["qsos"].toArray()));
    return operation;
}
