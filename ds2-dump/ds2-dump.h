#ifndef DATACOLLECTION_H
#define DATACOLLECTION_H

#include <QObject>
#include <QSharedPointer>
#include <QCommandLineParser>

#include "manager.h"

class DataCollection : public QObject
{
    Q_OBJECT
public:
    explicit DataCollection(QObject *aParent = 0);

signals:
    void finished();

public slots:
    void run();

protected:
    void serialSetup(QSharedPointer<QCommandLineParser> parser);
    DS2PlusPlus::ManagerPtr dbm;
};

#endif // DATACOLLECTION_H
