#ifndef LSZ_STATUS_H
#define LSZ_STATUS_H

#include <QObject>

#include <QObject>
#include <QString>
#include <QTest>

#include "ds2packet.h"
#include "controlunit.h"

namespace Test_ControlUnit {
    namespace ParseOperation {

        class LSZ_Status : public QObject
        {
            Q_OBJECT

        public:
            LSZ_Status();

        protected:
            static const char lsz_status1[], lsz_status2[];
            DS2PlusPlus::DS2PacketPtr packet1, packet2;
            DS2PlusPlus::ControlUnitPtr ecu;
            DS2PlusPlus::DS2Response results1, results2;

        private Q_SLOTS:
            void flash_to_pass_voltage();
            void turn_signal_switch_voltage();
            void unknown1();
            void dimmer_voltage();
            void unknown2();
            void photosensor_voltage();
        };
    }
}

#endif // LSZ_STATUS_H
