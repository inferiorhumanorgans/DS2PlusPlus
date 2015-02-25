#include "kombi46_ident.h"

namespace Test_ControlUnit {
    namespace ParseOperation {

        const char KOMBI46_Ident::kombi_ident[] = {0xa0, 0xf6, 0x90, 0x68, 0x96, 0x10, 0x04, 0x52, 0x11, 0x25, 0x00, 0x26, 0x20, 0x04, 0x54};

        KOMBI46_Ident::KOMBI46_Ident()
        {
            using namespace DS2PlusPlus;
            packet = DS2PacketPtr(PACKET_FROM_CHARS(ControlUnit::addressForFamily("KOMBI").first(), kombi_ident));
            ecu = ControlUnitPtr(new ControlUnit);
            ecu->loadByUuid(ControlUnit::ROOT_UUID);
            results = ecu->parseOperation("identify", packet);
        }

        #if 0
        void KOMBI46_Ident::initTestCase()
        {
        }

        void KOMBI46_Ident::cleanupTestCase()
        {
        }
        #endif

        void KOMBI46_Ident::partNumber()
        {
            QVariant expectedValue((quint64)6906896);
            QCOMPARE(results.value("part_number"), expectedValue);
        }

        void KOMBI46_Ident::hardwareNumber()
        {
            QVariant expectedValue(QString("0x10"));
            QCOMPARE(results.value("hardware_number"), expectedValue);
        }

        void KOMBI46_Ident::codingIndex()
        {
            QVariant expectedValue(QString("0x04"));
            QCOMPARE(results.value("coding_index"), expectedValue);
        }

        void KOMBI46_Ident::diagIndex()
        {
            QVariant expectedValue(QString("0x52"));
            QCOMPARE(results.value("diag_index"), expectedValue);
        }

        void KOMBI46_Ident::busIndex()
        {
            QVariant expectedValue(QString("0x11"));
            QCOMPARE(results.value("bus_index"), expectedValue);
        }

        void KOMBI46_Ident::mfrWeek()
        {
            QVariant expectedValue((quint8)25);
            QCOMPARE(results.value("build_date.week"), expectedValue);
        }

        void KOMBI46_Ident::mfrYear()
        {
            QVariant expectedValue((quint8)0);
            QCOMPARE(results.value("build_date.year"), expectedValue);
        }

        void KOMBI46_Ident::supplier()
        {
            QVariant expectedValue(QString("MotoMeter"));
            QCOMPARE(results.value("supplier"), expectedValue);
        }

        void KOMBI46_Ident::softwareNumber()
        {
            QVariant expectedValue(QString("0x20"));
            QCOMPARE(results.value("software_number"), expectedValue);
        }

    }
}

int main(int argc, char** argv)
{
  Test_ControlUnit::ParseOperation::KOMBI46_Ident tc;
  QTest::qExec(&tc, argc, argv);
  return 0;
}
