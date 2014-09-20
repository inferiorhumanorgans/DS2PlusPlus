#include "kombi46_canbus.h"

namespace Test_ControlUnit {
    namespace ParseOperation {

        using namespace DS2PlusPlus;

        KOMBI46_CanBus::KOMBI46_CanBus()
        {
            ecu = ControlUnitPtr(new ControlUnit("80000000-0001-0000-0000-000000000000"));
        }

        #if 0
        void KOMBI46_CanBus::initTestCase()
        {
        }

        void KOMBI46_CanBus::cleanupTestCase()
        {
        }
        #endif

        void KOMBI46_CanBus::rpm()
        {
            const char rpm_packet[] = {0xA0, 0x8C, 0x1D};
            DS2PacketPtr packet(PACKET_FROM_CHARS(ControlUnit::addressForFamily("KOMBI"), rpm_packet));
            DS2Response results = ecu->parseOperation("canbus_rpm", packet);
            QVariant expectedValue((quint64)1181);
            QCOMPARE(results.value("rpm"), expectedValue);
        }

    }
}

int main(int argc, char** argv)
{
  Test_ControlUnit::ParseOperation::KOMBI46_CanBus tc;
  QTest::qExec(&tc, argc, argv);
  return 0;
}
