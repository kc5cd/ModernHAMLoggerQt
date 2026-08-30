#include "callsignlookupservice.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

CallsignLookup::CallsignLookup(QObject *parent)
    : QObject(parent)
    , m_manager(new QNetworkAccessManager(this))
{
}

void CallsignLookup::lookup(const QString &callsign)
{
    const QString normalized = callsign.trimmed().toUpper();
    if (normalized.isEmpty())
        return;

    const QUrl url(QStringLiteral("https://api.hamdb.org/v1/%1/json/ModernHAMLoggerQt").arg(normalized));
    QNetworkReply *reply = m_manager->get(QNetworkRequest(url));

    connect(reply, &QNetworkReply::finished, this, [this, reply, normalized]() {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            emit lookupFailed(normalized, reply->errorString());
            return;
        }

        const QJsonObject root = QJsonDocument::fromJson(reply->readAll()).object();
        const QJsonObject hamdb = root.value("hamdb").toObject();
        const QString status = hamdb.value("messages").toObject().value("status").toString();
        if (status.compare("OK", Qt::CaseInsensitive) != 0) {
            emit lookupFailed(normalized, status.isEmpty() ? QStringLiteral("Not found") : status);
            return;
        }

        const QJsonObject c = hamdb.value("callsign").toObject();
        QVariantMap data;
        const QString firstName = c.value("fname").toString();
        const QString lastName = c.value("name").toString();
        data["name"] = (firstName + " " + lastName).trimmed();
        data["gridSquare"] = c.value("grid").toString();
        data["country"] = c.value("country").toString();
        data["state"] = c.value("state").toString();

        emit lookupSucceeded(normalized, data);
    });
}
