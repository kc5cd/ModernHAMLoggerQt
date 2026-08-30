#pragma once

#include <QList>
#include <QObject>
#include <QUrl>
#include <QVariantMap>
#include <qqml.h>

#include "operationlistmodel.h"
#include "stationprofile.h"

class LogbookManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(OperationListModel *operations READ operations CONSTANT)
    Q_PROPERTY(StationProfile *profile READ profile CONSTANT)
    Q_PROPERTY(int currentOperationIndex READ currentOperationIndex NOTIFY currentOperationIndexChanged)
    Q_PROPERTY(Operation *currentOperation READ currentOperation NOTIFY currentOperationIndexChanged)

public:
    explicit LogbookManager(QObject *parent = nullptr);
    ~LogbookManager() override;

    OperationListModel *operations() const { return m_operations; }
    StationProfile *profile() const { return m_profile; }
    int currentOperationIndex() const { return m_currentOperationIndex; }
    Operation *currentOperation() const;

    Q_INVOKABLE void addOperation(const QString &name, const QString &potaRef, const QString &sotaRef);
    Q_INVOKABLE void selectOperation(int index);
    Q_INVOKABLE void deleteOperation(int index);
    Q_INVOKABLE void logQso(const QVariantMap &fields);
    Q_INVOKABLE void deleteQso(int opIndex, int row);
    // Deletes any number of rows in one persisted operation and returns how
    // many were actually removed (out-of-range indices are skipped).
    Q_INVOKABLE int deleteQsos(int opIndex, const QList<int> &rows);
    Q_INVOKABLE bool importAdif(const QUrl &fileUrl);
    Q_INVOKABLE bool exportAdif(const QUrl &fileUrl, int opIndex);

signals:
    void currentOperationIndexChanged();

private:
    void loadState();
    void saveState() const;
    QString stateFilePath() const;
    // Appends the operation and persists any later in-place cell edits.
    void registerOperation(Operation *operation);

    OperationListModel *m_operations;
    StationProfile *m_profile;
    int m_currentOperationIndex = -1;
};
