#include "dme_ms420_ident.h"

const char DME_MS420_Ident::dme_ident[] = {0xa0, 0x37, 0x35, 0x30, 0x30, 0x32, 0x35, 0x35, 0x31, 0x35, 0x30, 0x30, 0x43, 0x30, 0x36, 0x30, 0x32, 0x37, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x31, 0x32, 0x45, 0x33, 0x30, 0x30, 0x00, 0x30, 0x32, 0x35, 0x35, 0x30, 0x32, 0x32, 0x30};

DME_MS420_Ident::DME_MS420_Ident()
{
    packet = DS2PlusPlus::DS2PacketPtr(new DS2PlusPlus::DS2Packet(0x12, QByteArray(dme_ident, sizeof(dme_ident) / sizeof(char))));
    json = DS2PlusPlus::ControlUnitPtr(new DS2PlusPlus::ControlUnit);
    //json.loadByUUID("B9D20D07-B7DA-4207-B8E1-2142AD938AD2");
    json->loadByUuid("F5C34396-809C-44C0-868E-49500414BEAA");
    results = json->parseOperation("identify", packet);
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
    QVariant expectedValue((quint8)27);
    QCOMPARE(results.value("mfr_week"), expectedValue);
}

void DME_MS420_Ident::mfrYear()
{
    QVariant expectedValue((quint8)0);
    QCOMPARE(results.value("mfr_year"), expectedValue);
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
