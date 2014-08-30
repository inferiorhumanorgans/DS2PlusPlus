#include "radio_cd53_ident.h"

using namespace DS2PlusPlus;

const char RADIO_CD53_Ident::cd53_ident[] = {0xa0, 0x86, 0x94, 0x15, 0x05, 0x44, 0x01, 0x35, 0x11, 0x25, 0x04, 0x17, 0x43, 0x30, 0x30, 0xcc};

RADIO_CD53_Ident::RADIO_CD53_Ident()
{
    packet = DS2PacketPtr(new DS2Packet(ControlUnit::ADDRESS_ZKE, QByteArray(cd53_ident, sizeof(cd53_ident) / sizeof(char))));
    json = ControlUnitPtr(new ControlUnit);
    json->loadByUuid("36DAD065-4EC9-446C-A7E6-415F601BC150");
    results = json->parseOperation("identify", packet);

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
    QCOMPARE(results.value("mfr_week"), expectedValue);
}

void RADIO_CD53_Ident::mfrYear()
{
    QVariant expectedValue((quint8)4);
    QCOMPARE(results.value("mfr_year"), expectedValue);
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
