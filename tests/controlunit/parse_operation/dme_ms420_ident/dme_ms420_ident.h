#ifndef DME_MS420_IDENT_H
#define DME_MS420_IDENT_H

#include <QObject>
#include <QString>
#include <QTest>

#include "ds2packet.h"
#include "manager.h"
#include "controlunit.h"

namespace Test_ControlUnit {
    namespace ParseOperation {

        class DME_MS420_Ident : public QObject
        {
            Q_OBJECT
        public:
            DME_MS420_Ident();

        protected:
            static const char dme_ident[];
            DS2PlusPlus::DS2PacketPtr packet;
            DS2PlusPlus::ControlUnitPtr ecu;
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

    }
}

#endif // DME_MS420_IDENT_H
