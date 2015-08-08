#include <QString>
#include <QtTest>
#include <QDebug>

#include <ds2/ds2packet.h>

class InitializationTest : public QObject
{
    Q_OBJECT

public:
    InitializationTest();

private Q_SLOTS:
    void testCase1();
    void testCase2();
    void testCase3();
};

InitializationTest::InitializationTest()
{
}

void InitializationTest::testCase1()
{
    DS2PlusPlus::DS2Packet *packet1 = new DS2PlusPlus::DS2Packet;
    QVERIFY(packet1->hasSourceAddress() == false);
}

void InitializationTest::testCase2()
{
    DS2PlusPlus::DS2Packet *packet2 = new DS2PlusPlus::DS2Packet("80 04 00");
    QVERIFY(packet2->hasSourceAddress() == false);
}

void InitializationTest::testCase3()
{
    DS2PlusPlus::DS2Packet *packet3 = new DS2PlusPlus::DS2Packet(0x80, "bytearray");
    QVERIFY(packet3->hasSourceAddress() == false);
}

QTEST_APPLESS_MAIN(InitializationTest)

#include "tst_initializationtest.moc"
