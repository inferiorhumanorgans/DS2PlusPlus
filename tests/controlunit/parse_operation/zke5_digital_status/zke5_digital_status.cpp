#include "zke5_digital_status.h"


namespace Test_ControlUnit {
    namespace ParseOperation {

        using namespace DS2PlusPlus;

        const char ZKE5_Digital_Status::zke_status[] = {0xa0, 0x40, 0x02, 0x88, 0xf0, 0x00, 0x00, 0x00, 0xb0, 0x00, 0x00, 0x00, 0x5c, 0x02, 0x33, 0x02, 0x2f, 0x04, 0x0d, 0x00, 0x06, 0x00, 0x00};

        ZKE5_Digital_Status::ZKE5_Digital_Status()
        {
            packet = DS2PacketPtr(PACKET_FROM_CHARS(ControlUnit::addressForFamily("ZKE").first(), zke_status));
            ecu= ControlUnitPtr(new ControlUnit);
            ecu->loadByUuid("00000000-0001-0000-0000-000000000000");
            results = ecu->parseOperation("digital_status", packet);
            qDebug() << ResponseToJsonString(results) << endl;
        }

        void ZKE5_Digital_Status::tailgate()
        {

        }

    }
}

int main(int argc, char** argv)
{
  Test_ControlUnit::ParseOperation::ZKE5_Digital_Status tc;
  QTest::qExec(&tc, argc, argv);
  return 0;
}
