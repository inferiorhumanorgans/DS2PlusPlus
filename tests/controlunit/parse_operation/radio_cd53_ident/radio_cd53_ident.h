#ifndef RADIO_CD53_IDENT_H
#define RADIO_CD53_IDENT_H

#include <QObject>

#include <QObject>
#include <QString>
#include <QTest>

#include <ds2/ds2packet.h>
#include <ds2/manager.h>
#include <ds2/controlunit.h>

namespace Test_ControlUnit {
    namespace ParseOperation {

        class RADIO_CD53_Ident : public QObject
        {
            Q_OBJECT
        public:
            RADIO_CD53_Ident();

        protected:
            static const char cd53_ident[];
            DS2PlusPlus::DS2PacketPtr packet;
            DS2PlusPlus::ControlUnitPtr ecu;
            DS2PlusPlus::PacketResponse results;

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

#endif // RADIO_CD53_IDENT_H
