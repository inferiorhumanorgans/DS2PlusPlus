#ifndef KOMBI46_IDENT_H
#define KOMBI46_IDENT_H

#include <QObject>
#include <QString>
#include <QTest>

#include "ds2packet.h"
#include "controlunit.h"

class KOMBI46_Ident : public QObject
{
    Q_OBJECT

public:
    KOMBI46_Ident();

protected:
    static const char kombi_ident[];
    DS2PlusPlus::DS2PacketPtr packet;
    DS2PlusPlus::ControlUnitPtr json;
    DS2PlusPlus::DS2Response results;

private Q_SLOTS:
    void partNumber();
    void hardwareNumber();
    void codingIndex();
    void diagIndex();
    void busIndex();
    void mfrWeek();
    void mfrYear();
    void supplier();
    void softwareNumber();
};

#endif // KOMBI46_IDENT_H

