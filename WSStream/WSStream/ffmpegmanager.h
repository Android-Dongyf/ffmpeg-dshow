#ifndef FFMPEGMANAGER_H
#define FFMPEGMANAGER_H

#if _MSC_VER
#define snprintf _snprintf
#endif

#include <QObject>
#include "common.h"

#include "ffmpegall.h"

class FfmpegManager : public QObject
{
    Q_OBJECT
public:
    explicit FfmpegManager(QObject *parent = 0);

private:
    void init();
signals:

public slots:
};

#endif // FFMPEGMANAGER_H
