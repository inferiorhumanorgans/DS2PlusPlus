#include <QTest>

#include <ds2packet.h>

namespace Test_DS2Packet {
    class Checksum : public QObject
    {
        Q_OBJECT
    public:
        Checksum();
    private Q_SLOTS:
        void test1();
    };

    Checksum::Checksum()
      : QObject(0)
    {
    }

    void Checksum::test1()
    {
        using namespace DS2PlusPlus;
        DS2Packet packet(0x01, "\x02\x03");
        qDebug() << packet;
        QCOMPARE(static_cast<quint16>(packet.checksum()), static_cast<quint16>(0x05));
    }
}

int main(int argc, char** argv)
{
  Test_DS2Packet::Checksum tc;
  return QTest::qExec(&tc, argc, argv);
}

#include "main.moc"
