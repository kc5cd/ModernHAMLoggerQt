#pragma once

#include <QDateTime>
#include <QJsonObject>
#include <QObject>
#include <qqml.h>

#include "qsologmodel.h"

class Operation : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Operation is created by LogbookManager")

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString potaRef READ potaRef WRITE setPotaRef NOTIFY potaRefChanged)
    Q_PROPERTY(QString sotaRef READ sotaRef WRITE setSotaRef NOTIFY sotaRefChanged)
    Q_PROPERTY(QDateTime createdUtc READ createdUtc CONSTANT)
    Q_PROPERTY(QsoLogModel *log READ log CONSTANT)
    Q_PROPERTY(int qsoCount READ qsoCount NOTIFY qsoCountChanged)

public:
    explicit Operation(QObject *parent = nullptr);

    QString name() const { return m_name; }
    void setName(const QString &name);

    QString potaRef() const { return m_potaRef; }
    void setPotaRef(const QString &ref);

    QString sotaRef() const { return m_sotaRef; }
    void setSotaRef(const QString &ref);

    QDateTime createdUtc() const { return m_createdUtc; }
    void setCreatedUtc(const QDateTime &dt) { m_createdUtc = dt; }

    QsoLogModel *log() const { return m_log; }

    int qsoCount() const { return m_log->rowCount(); }

    QJsonObject toJson() const;
    static Operation *fromJson(const QJsonObject &obj, QObject *parent = nullptr);

signals:
    void nameChanged();
    void potaRefChanged();
    void sotaRefChanged();
    void qsoCountChanged();

private:
    QString m_name;
    QString m_potaRef;
    QString m_sotaRef;
    QDateTime m_createdUtc;
    QsoLogModel *m_log;
};
