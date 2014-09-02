#ifndef KOMBI46_CANBUS_H
#define KOMBI46_CANBUS_H

#include <QObject>
#include <QString>
#include <QTest>

#include "ds2packet.h"
#include "controlunit.h"

namespace Test_ControlUnit {
    namespace ParseOperation {

        class KOMBI46_CanBus : public QObject
        {
            Q_OBJECT

        public:
            KOMBI46_CanBus();

        protected:
            DS2PlusPlus::ControlUnitPtr ecu;

        private Q_SLOTS:
            void rpm();
        };

    }
}

#endif // KOMBI46_CANBUS_H
