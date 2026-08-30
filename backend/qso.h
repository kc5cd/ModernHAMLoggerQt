#pragma once

#include <QDateTime>
#include <QString>

struct Qso
{
    QString callsign;
    QDateTime dateTimeUtc;
    QString band;
    QString mode;
    double freqMhz = 0.0;
    QString rstSent;
    QString rstRcvd;
    QString name;
    QString gridSquare;
    QString country;
    QString notes;
};
