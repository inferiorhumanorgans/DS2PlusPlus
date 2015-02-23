#include "lsz_status.h"

#include <math.h>

namespace Test_ControlUnit {
    namespace ParseOperation {

        const char LSZ_Status::lsz_status1[] = {0xa0, 0x10, 0x40, 0xfe, 0xfe, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x88, 0x00, 0x00, 0x00, 0x22, 0x00, 0x36, 0x6b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        const char LSZ_Status::lsz_status2[] = {0xa0, 0x10, 0x40, 0xfd, 0x4d, 0x20, 0x00, 0x20, 0x3c, 0x00, 0x83, 0x00, 0x00, 0x00, 0x02, 0x00, 0x36, 0x6b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        using namespace DS2PlusPlus;

        LSZ_Status::LSZ_Status()
        {
            using namespace DS2PlusPlus;
            packet1 = DS2PacketPtr(PACKET_FROM_CHARS(ControlUnit::addressForFamily("LSZ"), lsz_status1));
            packet2 = DS2PacketPtr(PACKET_FROM_CHARS(ControlUnit::addressForFamily("LSZ"), lsz_status2));
            ecu = ControlUnitPtr(new DS2PlusPlus::ControlUnit);
            ecu->loadByUuid("D0000000-0001-0000-0000-000000000000");
            results1 = ecu->parseOperation("status", packet1);
            results2 = ecu->parseOperation("status", packet2);

            qDebug() << ResponseToJsonString(results1) << endl;
            qDebug() << ResponseToJsonString(results2) << endl;
        }

        #if 0
        void LSZ_Status::initTestCase()
        {
        }

        void LSZ_Status::cleanupTestCase()
        {
        }
        #endif

        // Wiper switch?
        void LSZ_Status::flash_to_pass_voltage()
        {
            QString expectedValue1 = QString::number(4.980392), expectedValue2 = QString::number(4.960784);
            QString value1 = QString::number(results1.value("voltage.flash_to_pass").toDouble());
            QString value2 = QString::number(results2.value("voltage.flash_to_pass").toDouble());
            QCOMPARE(value1, expectedValue1);
            QCOMPARE(value2, expectedValue2);
        }

        // Turn signal switch?
        void LSZ_Status::turn_signal_switch_voltage()
        {
            QString expectedValue1 = QString::number(4.980392), expectedValue2 = QString::number(1.509804);
            QString value1 = QString::number(results1.value("voltage.turn_signal_switch").toDouble());
            QString value2 = QString::number(results2.value("voltage.turn_signal_switch").toDouble());
            QCOMPARE(value1, expectedValue1);
            QCOMPARE(value2, expectedValue2);
        }

        // "STAT VOLTAGE SENSOR FRONT LOAD VALUE"
        void LSZ_Status::unknown1()
        {
            QString expectedValue1 = QString::number(1.176471), expectedValue2 = QString::number(1.176471);
            QString value1 = QString::number(results1.value("voltage.unknown1").toDouble());
            QString value2 = QString::number(results2.value("voltage.unknown1").toDouble());
            QCOMPARE(value1, expectedValue1);
            QCOMPARE(value2, expectedValue2);
        }

        // Dimmer voltage?
        void LSZ_Status::dimmer_voltage()
        {
            QString expectedValue1 = QString::number(1.058824), expectedValue2 = QString::number(1.058824);
            QString value1 = QString::number(results1.value("voltage.dimmer").toDouble());
            QString value2 = QString::number(results2.value("voltage.dimmer").toDouble());
            QCOMPARE(value1, expectedValue1);
            QCOMPARE(value2, expectedValue2);
        }

        // LWR pot voltage?
        void LSZ_Status::unknown2()
        {
            QString expectedValue1 = QString::number(2.098039), expectedValue2 = QString::number(2.098039);
            QString value1 = QString::number(results1.value("voltage.unknown2").toDouble());
            QString value2 = QString::number(results2.value("voltage.unknown2").toDouble());
            QCOMPARE(value1, expectedValue1);
            QCOMPARE(value2, expectedValue2);
        }

        // Photo cell voltage
        void LSZ_Status::photosensor_voltage()
        {
            QString expectedValue1 = QString::number(0.0), expectedValue2 = QString::number(0.0);
            QString value1 = QString::number(results1.value("voltage.photosensor").toDouble());
            QString value2 = QString::number(results2.value("voltage.photosensor").toDouble());
            QCOMPARE(value1, expectedValue1);
            QCOMPARE(value2, expectedValue2);
        }
    }
}

int main(int argc, char** argv)
{
  Test_ControlUnit::ParseOperation::LSZ_Status tc;
  QTest::qExec(&tc, argc, argv);
  return 0;
}

