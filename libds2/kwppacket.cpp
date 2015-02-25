#include "kwppacket.h"

#include <stdexcept>

#include <QSharedPointer>
#include <QStringList>

namespace DS2PlusPlus {
    KWPPacket::KWPPacket(QObject *parent) :
        BasePacket(parent)
    {
        _hasSourceAddress = true;
        _sourceAddress = static_cast<quint8>(0xF1);
    }

    KWPPacket::KWPPacket(quint8 aTargetAddress, quint8 aSourceAddress, const QByteArray &someData, QObject *aParent) :
        BasePacket(aTargetAddress, aSourceAddress, someData, aParent)
    {
        _hasSourceAddress = true;
    }

    KWPPacket::KWPPacket(const QString &aPacketString)
    {
        if (aPacketString.isEmpty()) {
            throw std::invalid_argument("Packet string is empty");
        }

        QStringList ourArguments = aPacketString.split(" ");

        if (ourArguments.isEmpty()) {
            throw std::invalid_argument("Packet string is invalid");
        }

        quint8 ourTargetAddress = ourArguments.takeFirst().toUShort(NULL, 16);
        quint8 ourSourceAddress = ourArguments.takeFirst().toUShort(NULL, 16);
        quint8 ourLength = ourArguments.takeFirst().toUShort(NULL, 16) - 2;

        QByteArray ourData;
        for (int i=0; i < ourArguments.length(); i++) {
            ourData.append(ourArguments.at(i).toUShort(NULL, 16));
        }

        _hasSourceAddress = true;
        _sourceAddress = ourSourceAddress;
        _targetAddress = ourTargetAddress;
        _data = ourData;

        if ((ourLength - 1) == ourData.length()) {

        } else if (ourLength == ourData.length()) {
            // There was a checksum
            quint8 ourChecksum = _data.at(_data.length() - 1);
            _data.remove(_data.length() - 1, 1);

            if (ourChecksum != checksum()) {
                QString errorString = QString("Corrupt packet, checksum mismatch got 0x%1, expected: 0x%2").arg(ourChecksum, 2, 16, QChar('0')).arg(checksum(), 2, 16, QChar('0'));
                throw std::invalid_argument(qPrintable(errorString));
            }
        } else {
            QString errorString = QString("Corrupt packet, string specifies amount of data.  It said %1, but came with %2 bytes.").arg(QString::number(ourLength)).arg(ourData.length());
            throw std::invalid_argument(qPrintable(errorString));
        }
    }

    unsigned char KWPPacket::checksum(const QByteArray &someData) const
    {
        QByteArray ourData(someData);

        if (ourData.isEmpty()) {
            ourData.append(static_cast<quint8>(0xB8));
            ourData.append(_targetAddress);
            ourData.append(static_cast<quint8>(_sourceAddress));
            ourData.append(static_cast<quint8>(_data.length()));
            ourData.append(_data);
        }

        unsigned char ourChecksum = 0;
        for (int i=0; i < ourData.length(); i++) {
            ourChecksum ^= static_cast<quint8>(ourData.at(i));
        }

        return ourChecksum;
    }

    BasePacket::ProtocolType KWPPacket::protocol() const
    {
        return ProtocolKWP;
    }

    const QString KWPPacket::toByteString () const
    {
        QStringList ret;
        QChar zeroPadding('0');

        ret.append(QString("%1").arg(static_cast<quint8>(0xB8), 2, 16, zeroPadding));
        ret.append(QString("%1").arg(_targetAddress, 2, 16, zeroPadding));
        ret.append(QString("%1").arg(_sourceAddress, 2, 16, zeroPadding));
        ret.append(QString("%1").arg(_data.length(), 2, 16, zeroPadding));

        for (int i=0; i < _data.length(); i++) {
            ret.append(QString("%1").arg(static_cast<quint8>(_data.at(i)), 2, 16, zeroPadding));
        }

        return ret.join(" ");
    }

    const QByteArray KWPPacket::toByteArray() const
    {
        QByteArray ret;

        ret.append(0xB8);
        ret.append(_targetAddress);
        ret.append(_sourceAddress);
        ret.append(_data.length());

        ret.append(_data);
        ret.append(checksum(ret));

//        ret.data()[1] = 0x12;

        return ret;
    }
}
