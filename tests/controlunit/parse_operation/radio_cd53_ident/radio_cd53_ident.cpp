#include "radio_cd53_ident.h"

namespace Test_ControlUnit {
    namespace ParseOperation {

        using namespace DS2PlusPlus;

        const char RADIO_CD53_Ident::cd53_ident[] = {0xa0, 0x86, 0x94, 0x15, 0x05, 0x44, 0x01, 0x35, 0x11, 0x25, 0x04, 0x17, 0x43, 0x30, 0x30, 0xcc};

        RADIO_CD53_Ident::RADIO_CD53_Ident()
        {
            packet = DS2PacketPtr(PACKET_FROM_CHARS(ControlUnit::addressForFamily("RADIO").first(), cd53_ident));
            ecu = ControlUnitPtr(new ControlUnit);
            ecu->loadByUuid("68000000-0001-0000-0000-000000000000");
            results = ecu->parseOperation("identify", packet);
        }

        void RADIO_CD53_Ident::partNumber()
        {
            QVariant expectedValue((quint64)6941505);
            QCOMPARE(results.value("part_number"), expectedValue);
        }

        void RADIO_CD53_Ident::hardwareNumber()
        {
            QVariant expectedValue(QString("0x44"));
            QCOMPARE(results.value("hardware_number"), expectedValue);
        }

        void RADIO_CD53_Ident::codingIndex()
        {
            QVariant expectedValue(QString("0x01"));
            QCOMPARE(results.value("coding_index"), expectedValue);
        }

        void RADIO_CD53_Ident::diagIndex()
        {
            QVariant expectedValue(QString("0x35"));
            QCOMPARE(results.value("diag_index"), expectedValue);
        }

        void RADIO_CD53_Ident::busIndex()
        {
            QVariant expectedValue(QString("0x11"));
            QCOMPARE(results.value("bus_index"), expectedValue);
        }

        void RADIO_CD53_Ident::mfrWeek()
        {
            QVariant expectedValue((quint8)25);
            QCOMPARE(results.value("build_date.week"), expectedValue);
        }

        void RADIO_CD53_Ident::mfrYear()
        {
            QVariant expectedValue((quint8)4);
            QCOMPARE(results.value("build_date.year"), expectedValue);
        }

        void RADIO_CD53_Ident::supplier()
        {
            QVariant expectedValue(QString("Alpine"));
            QCOMPARE(results.value("supplier"), expectedValue);
        }

        void RADIO_CD53_Ident::softwareNumber()
        {
            QVariant expectedValue(QString("0x43"));
            QCOMPARE(results.value("software_number"), expectedValue);
        }

    }
}

int main(int argc, char** argv)
{
  Test_ControlUnit::ParseOperation::RADIO_CD53_Ident tc;
  QTest::qExec(&tc, argc, argv);
  return 0;
}
