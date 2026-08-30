#include "stationprofile.h"

#include <QSettings>

StationProfile::StationProfile(QObject *parent)
    : QObject(parent)
{
    load();
}

void StationProfile::setCallsign(const QString &callsign)
{
    if (m_callsign == callsign)
        return;
    m_callsign = callsign;
    emit callsignChanged();
}

void StationProfile::setOperatorName(const QString &name)
{
    if (m_operatorName == name)
        return;
    m_operatorName = name;
    emit operatorNameChanged();
}

void StationProfile::setGridSquare(const QString &grid)
{
    if (m_gridSquare == grid)
        return;
    m_gridSquare = grid;
    emit gridSquareChanged();
}

void StationProfile::load()
{
    QSettings settings;
    settings.beginGroup("StationProfile");
    setCallsign(settings.value("callsign").toString());
    setOperatorName(settings.value("operatorName").toString());
    setGridSquare(settings.value("gridSquare").toString());
    settings.endGroup();
}

void StationProfile::save() const
{
    QSettings settings;
    settings.beginGroup("StationProfile");
    settings.setValue("callsign", m_callsign);
    settings.setValue("operatorName", m_operatorName);
    settings.setValue("gridSquare", m_gridSquare);
    settings.endGroup();
}
