#ifndef ZKE5_DIGITAL_STATUS_H
#define ZKE5_DIGITAL_STATUS_H

#include <QObject>
#include <QString>
#include <QTest>

#include <ds2/ds2packet.h>
#include <ds2/manager.h>
#include <ds2/controlunit.h>

namespace Test_ControlUnit {
    namespace ParseOperation {

        class ZKE5_Digital_Status : public QObject
        {
            Q_OBJECT
        public:
            ZKE5_Digital_Status();

        protected:
            static const char zke_status[];
            DS2PlusPlus::DS2PacketPtr packet;
            DS2PlusPlus::ControlUnitPtr ecu;
            DS2PlusPlus::PacketResponse results;

        private Q_SLOTS:
            void tailgate();
        };

    }
}

#endif // ZKE5_DIGITAL_STATUS_H
