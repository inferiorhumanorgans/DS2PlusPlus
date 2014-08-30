#include "ihka46_ident.h"

const char IHKA46_Ident::ihka_ident[] = {0xA0, 0x84, 0x10, 0x69, 0x30, 0x22, 0x03, 0x32, 0x0A, 0x26, 0x00, 0x03, 0x05};

IHKA46_Ident::IHKA46_Ident()
{
    using namespace DS2PlusPlus;
    packet = DS2PacketPtr(PACKET_FROM_CHARS(ControlUnit::ADDRESS_IHKA, ihka_ident));
    json = ControlUnitPtr(new DS2PlusPlus::ControlUnit);
    json->loadByUuid("3CD3AE6B-7826-4884-AC6A-A88FCF682B85");
    results = json->parseOperation("identify", packet);
}

#if 0
void IHKA46_Ident::initTestCase()
{
}

void IHKA46_Ident::cleanupTestCase()
{
}
#endif

void IHKA46_Ident::partNumber()
{
    QVariant expectedValue((quint64)4106930);
    QCOMPARE(results.value("part_number"), expectedValue);
}

void IHKA46_Ident::hardwareNumber()
{
    QVariant expectedValue(QString("0x22"));
    QCOMPARE(results.value("hardware_number"), expectedValue);
}

void IHKA46_Ident::codingIndex()
{
    QVariant expectedValue(QString("0x03"));
    QCOMPARE(results.value("coding_index"), expectedValue);
}

void IHKA46_Ident::diagIndex()
{
    QVariant expectedValue(QString("0x32"));
    QCOMPARE(results.value("diag_index"), expectedValue);
}

void IHKA46_Ident::busIndex()
{
    QVariant expectedValue(QString("0x00"));
    QCOMPARE(results.value("bus_index"), expectedValue);
}

void IHKA46_Ident::mfrWeek()
{
    QVariant expectedValue((quint8)26);
    QCOMPARE(results.value("mfr_week"), expectedValue);
}

void IHKA46_Ident::mfrYear()
{
    QVariant expectedValue((quint8)0);
    QCOMPARE(results.value("mfr_year"), expectedValue);
}

void IHKA46_Ident::supplier()
{
    QVariant expectedValue(QString("Hella"));
    QCOMPARE(results.value("supplier"), expectedValue);
}

void IHKA46_Ident::softwareNumber()
{
    QVariant expectedValue(QString("0x05"));
    QCOMPARE(results.value("software_number"), expectedValue);
}
