#include <QString>
#include <QtTest>
#include <QDebug>

#include <ds2/kwppacket.h>

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
    DS2PlusPlus::KWPPacket *packet1 = new DS2PlusPlus::KWPPacket;
    QVERIFY(packet1->hasSourceAddress() == true);
    QVERIFY(packet1->protocol() == DS2PlusPlus::BasePacket::ProtocolKWP);
}

void InitializationTest::testCase2()
{
    // Make sure we start with the magic number (0xB8), then source, target
    DS2PlusPlus::KWPPacket *packet2 = new DS2PlusPlus::KWPPacket("B8 80 04 00");
    QVERIFY(packet2->hasSourceAddress() == true);
    QVERIFY(packet2->protocol() == DS2PlusPlus::BasePacket::ProtocolKWP);
}

void InitializationTest::testCase3()
{
    DS2PlusPlus::KWPPacket *packet3 = new DS2PlusPlus::KWPPacket(0x80, 0x00, QByteArray("\xB8\xF1", 2));
    QVERIFY(packet3->hasSourceAddress() == true);
    QVERIFY(packet3->protocol() == DS2PlusPlus::BasePacket::ProtocolKWP);
}

QTEST_APPLESS_MAIN(InitializationTest)

#include "tst_initializationtest.moc"
