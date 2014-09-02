#ifndef ZKE5_IDENT_H
#define ZKE5_IDENT_H

#include <QObject>
#include <QString>
#include <QTest>

#include "ds2packet.h"
#include "manager.h"
#include "controlunit.h"

namespace Test_ControlUnit {
    namespace ParseOperation {

        class ZKE5_Ident : public QObject
        {
            Q_OBJECT
        public:
            ZKE5_Ident();

        protected:
            static const char zke_ident[];
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

#endif // ZKE5_IDENT_H
