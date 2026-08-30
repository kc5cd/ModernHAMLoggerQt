#pragma once

#include <QObject>
#include <qqml.h>

class StationProfile : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("StationProfile is created by LogbookManager")

    Q_PROPERTY(QString callsign READ callsign WRITE setCallsign NOTIFY callsignChanged)
    Q_PROPERTY(QString operatorName READ operatorName WRITE setOperatorName NOTIFY operatorNameChanged)
    Q_PROPERTY(QString gridSquare READ gridSquare WRITE setGridSquare NOTIFY gridSquareChanged)

public:
    explicit StationProfile(QObject *parent = nullptr);

    QString callsign() const { return m_callsign; }
    void setCallsign(const QString &callsign);

    QString operatorName() const { return m_operatorName; }
    void setOperatorName(const QString &name);

    QString gridSquare() const { return m_gridSquare; }
    void setGridSquare(const QString &grid);

    Q_INVOKABLE void load();
    Q_INVOKABLE void save() const;

signals:
    void callsignChanged();
    void operatorNameChanged();
    void gridSquareChanged();

private:
    QString m_callsign;
    QString m_operatorName;
    QString m_gridSquare;
};
