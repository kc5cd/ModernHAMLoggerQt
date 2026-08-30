#pragma once

#include <QString>
#include <QVector>

#include "operation.h"
#include "qso.h"
#include "stationprofile.h"

class AdifIo
{
public:
    static QVector<Qso> parseFile(const QString &path);
    static bool writeFile(const QString &path, Operation *operation, StationProfile *profile);
    static bool writeFile(const QString &path, const QVector<Operation *> &operations, StationProfile *profile);
};
