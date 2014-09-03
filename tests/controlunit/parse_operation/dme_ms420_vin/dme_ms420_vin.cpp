#include "dme_ms420_vin.h"

namespace Test_ControlUnit {
    namespace ParseOperation {

        const char DME_MS420_VIN::dme_vin[] = {0xa0, 0x20, 0x2c, 0xa3, 0x56, 0x18, 0x31, 0x02, 0x89, 0x07, 0x80, 0x14, 0x51, 0x03, 0x00, 0xb3, 0x63, 0x00, 0x72, 0x7e, 0x48, 0x03, 0x4a, 0x00, 0x15, 0xf2, 0x5c, 0x00, 0x72, 0x7e, 0x47, 0x00, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00, 0xc6, 0x01, 0x00};

        DME_MS420_VIN::DME_MS420_VIN()
        {
            using namespace DS2PlusPlus;
            packet = DS2PacketPtr(PACKET_FROM_CHARS(ControlUnit::ADDRESS_DME, dme_vin));
            ecu = ControlUnitPtr(new ControlUnit);
            ecu->loadByUuid("F5C34396-809C-44C0-868E-49500414BEAA");
            results = ecu->parseOperation("vehicle_id", packet);
        }

        void DME_MS420_VIN::vin()
        {
            QVariant expectedValue(QString("WBADM6342YGU05543"));
            QCOMPARE(results.value("vin"), expectedValue);
        }
    }
}

int main(int argc, char** argv)
{
  Test_ControlUnit::ParseOperation::DME_MS420_VIN tc;
  QTest::qExec(&tc, argc, argv);
  return 0;
}
