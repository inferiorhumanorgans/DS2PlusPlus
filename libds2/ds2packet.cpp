/*
 * This file is part of libds2
 * Copyright (C) 2014
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to:
 * Free Software Foundation, Inc.
 * 51 Franklin Street, Fifth Floor
 * Boston, MA  02110-1301 USA
 *
 * Or see <http://www.gnu.org/licenses/>.
 */

#include <stdexcept>

#include <QSharedPointer>

#include "ds2packet.h"

namespace DS2PlusPlus {
    DS2Packet::DS2Packet(QObject *parent) :
        BasePacket(parent)
    {
    }

    DS2Packet::DS2Packet(const QString &aPacketString)
    {
        if (aPacketString.isEmpty()) {
            throw std::invalid_argument("Empty packet string is invalid");
        }

        QStringList ourArguments = aPacketString.split(" ");

        if (ourArguments.isEmpty()) {
            throw std::invalid_argument("Packet string is invalid");
        }

        quint8 ourAddress = ourArguments.takeFirst().toUShort(NULL, 16);
        quint8 ourLength = ourArguments.takeFirst().toUShort(NULL, 16) - 2;

        QByteArray ourData;
        for (int i=0; i < ourArguments.length(); i++) {
            ourData.append(ourArguments.at(i).toUShort(NULL, 16));
        }

        _targetAddress = ourAddress;
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

    unsigned char DS2Packet::checksum(const QByteArray &someData) const
    {
        QByteArray ourData(someData);

        if (ourData.isEmpty()) {
            ourData.append(_targetAddress);
            ourData.append(_data.length() + 3);
            ourData.append(_data);
        }

        unsigned char ourChecksum = 0;
        for (int i=0; i < ourData.length(); i++) {
            ourChecksum ^= static_cast<quint8>(ourData.at(i));
        }

        return ourChecksum;
    }

    BasePacket::ProtocolType DS2Packet::protocol() const
    {
        return ProtocolDS2;
    }

    const QString DS2Packet::toByteString () const
    {
        QString ret("#<DS2 Target:%1, Len: %2, Data:%3, Chk:%4>");
        QString tmp;

        tmp.sprintf(HEX_CHAR_FORMAT, static_cast<quint8>(_targetAddress));
        ret = ret.arg(tmp);

        tmp.sprintf(HEX_CHAR_FORMAT, static_cast<quint8>(_data.length() + 3));
        ret = ret.arg(tmp);

        QStringList data;
        for (int i=0; i < _data.length(); i++) {
            tmp.sprintf(HEX_CHAR_FORMAT, static_cast<quint8>(_data.at(i)));
            data.append(tmp);
        }
        ret = ret.arg(data.join(" "));

        tmp.sprintf(HEX_CHAR_FORMAT, static_cast<quint8>(checksum()));
        ret = ret.arg(tmp);

        return ret;
    }

    const QByteArray DS2Packet::toByteArray() const
    {
        QByteArray ret;

        ret.append(_targetAddress);
        ret.append(_data.length() + 3);
        ret.append(_data);
        ret.append(checksum(ret));

        return ret;
    }
}
