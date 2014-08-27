#ifndef DATACOLLECTION_H
#define DATACOLLECTION_H

#include <QObject>

class DataCollection : public QObject
{
    Q_OBJECT
public:
    explicit DataCollection(QObject *aParent = 0);

signals:
    void finished();

public slots:
    void run();
};

#endif // DATACOLLECTION_H
