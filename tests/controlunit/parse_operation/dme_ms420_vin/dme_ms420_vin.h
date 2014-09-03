#ifndef DME_MS420_VIN_H
#define DME_MS420_VIN_H

#include <QObject>
#include <QString>
#include <QTest>

#include "ds2packet.h"
#include "manager.h"
#include "controlunit.h"

namespace Test_ControlUnit {
    namespace ParseOperation {

        class DME_MS420_VIN : public QObject
        {
            Q_OBJECT
        public:
            DME_MS420_VIN();

        protected:
            static const char dme_vin[];
            DS2PlusPlus::DS2PacketPtr packet;
            DS2PlusPlus::ControlUnitPtr ecu;
            DS2PlusPlus::DS2Response results;

        private Q_SLOTS:
            void vin();
        };

    }
}

#endif // DME_MS420_VIN_H
