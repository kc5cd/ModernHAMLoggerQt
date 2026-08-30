#pragma once

#include <QAbstractTableModel>
#include <QJsonArray>
#include <QVector>
#include <qqml.h>

#include "qso.h"

class QsoLogModel : public QAbstractTableModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("QsoLogModel is created by Operation")

public:
    enum Column {
        TimeColumn = 0,
        CallsignColumn,
        BandColumn,
        ModeColumn,
        FreqColumn,
        RstSentColumn,
        RstRcvdColumn,
        NameColumn,
        GridColumn,
        CountryColumn,
        NotesColumn,
        ColumnCount,
    };
    Q_ENUM(Column)

    explicit QsoLogModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void addQso(const Qso &qso);
    // Not Q_INVOKABLE: row removal must go through LogbookManager::deleteQsos()
    // so the change is persisted (saveState() is wired to dataChanged only,
    // not to rowsRemoved/modelReset — see LogbookManager::registerOperation).
    void removeAt(int row);
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    // Explicit write path for the table's edit delegates. Delegate required
    // properties are one-way, so assigning to them would not reach setData().
    Q_INVOKABLE bool setCell(int row, int column, const QVariant &value);

    const QVector<Qso> &qsos() const { return m_qsos; }
    void setQsos(const QVector<Qso> &qsos);

    QJsonArray toJson() const;
    static QVector<Qso> fromJson(const QJsonArray &array);

private:
    QVector<Qso> m_qsos;
};
