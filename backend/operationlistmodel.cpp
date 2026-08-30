#include "operationlistmodel.h"

OperationListModel::OperationListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int OperationListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_operations.size();
}

QVariant OperationListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_operations.size())
        return {};

    Operation *operation = m_operations.at(index.row());
    switch (role) {
    case OperationRole:
        return QVariant::fromValue(operation);
    case NameRole:
        return operation->name();
    case QsoCountRole:
        return operation->qsoCount();
    case PotaRefRole:
        return operation->potaRef();
    case SotaRefRole:
        return operation->sotaRef();
    default:
        return {};
    }
}

QHash<int, QByteArray> OperationListModel::roleNames() const
{
    return {
        {OperationRole, "operation"},
        {NameRole, "name"},
        {QsoCountRole, "qsoCount"},
        {PotaRefRole, "potaRef"},
        {SotaRefRole, "sotaRef"},
    };
}

void OperationListModel::append(Operation *operation)
{
    operation->setParent(this);
    beginInsertRows(QModelIndex(), m_operations.size(), m_operations.size());
    m_operations.append(operation);
    endInsertRows();

    const int row = m_operations.size() - 1;
    connect(operation, &Operation::qsoCountChanged, this, [this, row]() {
        const QModelIndex idx = index(row, 0);
        emit dataChanged(idx, idx, {QsoCountRole});
    });
}

Operation *OperationListModel::at(int row) const
{
    if (row < 0 || row >= m_operations.size())
        return nullptr;
    return m_operations.at(row);
}

void OperationListModel::clear()
{
    beginResetModel();
    qDeleteAll(m_operations);
    m_operations.clear();
    endResetModel();
}
