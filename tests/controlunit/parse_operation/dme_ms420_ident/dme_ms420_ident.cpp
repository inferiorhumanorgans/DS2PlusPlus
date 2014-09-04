#include "dme_ms420_ident.h"

namespace Test_ControlUnit {
    namespace ParseOperation {

        const char DME_MS420_Ident::dme_ident[] = {0xa0, 0x37, 0x35, 0x30, 0x30, 0x32, 0x35, 0x35, 0x31, 0x35, 0x30, 0x30, 0x43, 0x30, 0x36, 0x30, 0x32, 0x37, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x31, 0x32, 0x45, 0x33, 0x30, 0x30, 0x00, 0x30, 0x32, 0x35, 0x35, 0x30, 0x32, 0x32, 0x30};

        DME_MS420_Ident::DME_MS420_Ident()
        {
            using namespace DS2PlusPlus;
            packet = DS2PacketPtr(PACKET_FROM_CHARS(ControlUnit::addressForFamily("DME"), dme_ident));
            ecu = ControlUnitPtr(new ControlUnit);
            //json.loadByUUID("B9D20D07-B7DA-4207-B8E1-2142AD938AD2");
            ecu->loadByUuid("F5C34396-809C-44C0-868E-49500414BEAA");
            results = ecu->parseOperation("identify", packet);
        }

        void DME_MS420_Ident::partNumber()
        {
            QVariant expectedValue((quint64)7500255);
            QCOMPARE(results.value("part_number"), expectedValue);
        }

        void DME_MS420_Ident::hardwareNumber()
        {
            QVariant expectedValue(QString("0x15"));
            QCOMPARE(results.value("hardware_number"), expectedValue);
        }

        void DME_MS420_Ident::codingIndex()
        {
            QVariant expectedValue(QString("0x00"));
            QCOMPARE(results.value("coding_index"), expectedValue);
        }

        void DME_MS420_Ident::diagIndex()
        {
            QVariant expectedValue(QString("0xC0"));
            QCOMPARE(results.value("diag_index"), expectedValue);
        }

        void DME_MS420_Ident::busIndex()
        {
            QVariant expectedValue(QString("0x60"));
            QCOMPARE(results.value("bus_index"), expectedValue);
        }

        void DME_MS420_Ident::mfrWeek()
        {
            QVariant expectedValue(static_cast<quint8>(27));
            QCOMPARE(results.value("build_date.week"), expectedValue);
        }

        void DME_MS420_Ident::mfrYear()
        {
            QVariant expectedValue(static_cast<quint8>(0));
            QCOMPARE(results.value("build_date.year"), expectedValue);
        }

        void DME_MS420_Ident::supplier()
        {
            QVariant expectedValue(QString("0000115852"));
            QCOMPARE(results.value("supplier"), expectedValue);
        }

        void DME_MS420_Ident::softwareNumber()
        {
            QVariant expectedValue(QString("0xE3"));
            QCOMPARE(results.value("software_number"), expectedValue);
        }

    }
}

int main(int argc, char** argv)
{
  Test_ControlUnit::ParseOperation::DME_MS420_Ident tc;
  QTest::qExec(&tc, argc, argv);
  return 0;
}
