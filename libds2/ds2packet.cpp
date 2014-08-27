#include "ds2packet.h"

namespace DS2PlusPlus {
    DS2Packet::DS2Packet(QObject *parent) :
        QObject(parent)
    {
    }

    DS2Packet::DS2Packet(quint8 anEcuAddress, const QByteArray &someData, QObject *parent) :
        QObject(parent), _address(anEcuAddress), _data(someData)
    {

    }

    float DS2Packet::fetchFloat(quint16 aPosition, float aMultiplicativeFactor, int anAdditiveFactor) const
    {
        return ((quint8)_data.at(aPosition) * aMultiplicativeFactor) + anAdditiveFactor;
    }

    QString DS2Packet::fetchString(quint16 aPosition, quint16 aLength) const
    {
        return QString(_data.mid(aPosition, aLength));
    }

    unsigned char DS2Packet::checksum(const QByteArray &someData) const
    {
        QByteArray ourData(someData);

        if (ourData.isEmpty()) {
            ourData.append(_address);
            ourData.append(_data.length() + 3);
            ourData.append(_data);
        }

        unsigned char ourChecksum = 0;
        for (int i=0; i < ourData.length(); i++) {
            ourChecksum ^= (unsigned char)ourData.at(i);
        }

        return ourChecksum;
    }

    quint8 DS2Packet::address() const {
        return _address;
    }

    void DS2Packet::setAddress(quint8 anAddress) {
        _address = anAddress;
    }

    QByteArray DS2Packet::data() const {
        return _data;
    }

    void DS2Packet::setData(const QByteArray &someData) {
        _data = someData;
    }

    DS2Packet::operator const QByteArray () const
    {
        QByteArray ret;
        ret.append(_address);
        ret.append(_data.length() + 3);
        ret.append(_data);

        ret.append(checksum(ret));

        return ret;
    }
}

QDebug operator << (QDebug d, const DS2PlusPlus::DS2Packet &packet)
{
    QString debugString("ECU: %1 DATA: %2 CHECKSUM: %3");
    QChar zeroPadding('0');
    QString ecuString = QString("0x%1").arg(packet.address(), 2, 16, zeroPadding);

    QByteArray data = packet.data();
    QStringList dataStringList;
    for (int i=0; i < data.length(); i++) {
        dataStringList.append(QString("0x%1").arg((unsigned char)data.at(i), 2, 16, zeroPadding));
    }

    QString checksumString = QString("0x%1").arg(packet.checksum(), 2, 16, zeroPadding);

    d << debugString.arg(ecuString).arg(dataStringList.join(", ")).arg(checksumString);

    return d;
}
