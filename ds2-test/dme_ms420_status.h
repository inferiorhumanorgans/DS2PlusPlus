#ifndef DME_MS420_STATUS_H
#define DME_MS420_STATUS_H

#include <QObject>
#include <QString>
#include <QTest>

#include "ds2packet.h"
#include "manager.h"
#include "controlunit.h"

class DME_MS420_Status : public QObject
{
    Q_OBJECT
public:
    DME_MS420_Status();

protected:
    static const char dme_status[];
    DS2PlusPlus::DS2PacketPtr packet;
    DS2PlusPlus::ControlUnitPtr json;
    DS2PlusPlus::DS2Response results;

private Q_SLOTS:
    void oilTemp();
    void coolantTemp();
    void intakeTemp();
    void coolantOutletTemp();
    void ignitionAdvance();
    void batteryVoltage();
};

#endif // DME_MS420_STATUS_H
