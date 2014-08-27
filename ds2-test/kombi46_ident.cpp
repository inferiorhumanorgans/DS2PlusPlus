#include "kombi46_ident.h"

const char KOMBI46_Ident::kombi_ident[] = {0xa0, 0xf6, 0x90, 0x68, 0x96, 0x10, 0x04, 0x52, 0x11, 0x25, 0x00, 0x26, 0x20, 0x04, 0x54};

KOMBI46_Ident::KOMBI46_Ident()
{
    packet = DS2PlusPlus::DS2PacketPtr(new DS2PlusPlus::DS2Packet(0x5b, QByteArray(kombi_ident, sizeof(kombi_ident) / sizeof(char))));
    json = DS2PlusPlus::ControlUnitPtr(new DS2PlusPlus::ControlUnit);
    json->loadByUuid("B9D20D07-B7DA-4207-B8E1-2142AD938AD2");
    results = json->parseOperation("identify", packet);
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
    QCOMPARE(results.value("mfr_week"), expectedValue);
}

void KOMBI46_Ident::mfrYear()
{
    QVariant expectedValue((quint8)0);
    QCOMPARE(results.value("mfr_year"), expectedValue);
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
