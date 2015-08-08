#ifndef IHKA46_IDENT_H
#define IHKA46_IDENT_H

#include <QObject>
#include <QString>
#include <QTest>
#include <QSharedPointer>

#include <ds2/ds2packet.h>
#include <ds2/controlunit.h>

namespace Test_ControlUnit {
    namespace ParseOperation {

        class IHKA46_Ident : public QObject
        {
            Q_OBJECT

        public:
            IHKA46_Ident();

        protected:
            static const char ihka_ident[];
            DS2PlusPlus::DS2PacketPtr packet;
            DS2PlusPlus::ControlUnitPtr json;
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

#endif // IHKA46_IDENT_H
