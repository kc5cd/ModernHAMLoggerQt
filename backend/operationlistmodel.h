#pragma once

#include <QAbstractListModel>
#include <QVector>
#include <qqml.h>

#include "operation.h"

class OperationListModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("OperationListModel is created by LogbookManager")

public:
    enum Roles {
        OperationRole = Qt::UserRole + 1,
        NameRole,
        QsoCountRole,
        PotaRefRole,
        SotaRefRole,
    };
    Q_ENUM(Roles)

    explicit OperationListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void append(Operation *operation);
    Operation *at(int row) const;
    int count() const { return m_operations.size(); }

    const QVector<Operation *> &operations() const { return m_operations; }
    void clear();

private:
    QVector<Operation *> m_operations;
};
