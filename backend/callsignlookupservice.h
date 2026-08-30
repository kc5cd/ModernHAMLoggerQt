#pragma once

#include <QObject>
#include <QVariantMap>
#include <qqml.h>

class QNetworkAccessManager;

class CallsignLookup : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit CallsignLookup(QObject *parent = nullptr);

    Q_INVOKABLE void lookup(const QString &callsign);

signals:
    void lookupSucceeded(const QString &callsign, const QVariantMap &data);
    void lookupFailed(const QString &callsign, const QString &reason);

private:
    QNetworkAccessManager *m_manager;
};
