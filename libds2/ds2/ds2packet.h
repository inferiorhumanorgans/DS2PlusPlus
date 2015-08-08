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

#ifndef DS2PACKET_H
#define DS2PACKET_H

#include "basepacket.h"

#define PACKET_FROM_CHARS(address, array) new DS2PlusPlus::DS2Packet(address, QByteArray(array, sizeof(array) / sizeof(char)))

namespace DS2PlusPlus
{
    /*!
     * \brief The DS2Packet class represents a raw DS2 packet sent to or received from an ECU.
     */
    class DS2Packet : public BasePacket
    {
        Q_OBJECT
    public:
        explicit DS2Packet(QObject *parent = 0);
        DS2Packet(const QString &aPacketString);
        DS2Packet(quint8 anAddress, const QByteArray &someData, QObject *aParent = 0) : BasePacket(anAddress, someData, aParent) {;}

        virtual unsigned char checksum(const QByteArray &data = QByteArray()) const;

        virtual ProtocolType protocol() const;
        virtual const QString toByteString () const;

        virtual QByteArray expectedHeaderPadding() const;

    protected:
        virtual const QByteArray toByteArray() const;
    };
    typedef QSharedPointer<DS2Packet> DS2PacketPtr;
}

namespace Json {
    class Value;
};

const Json::Value *DS2ResponseToJson(const DS2PlusPlus::PacketResponse &aResponse);
const QString DS2ResponseToJsonString(const DS2PlusPlus::PacketResponse &aResponse);
const QString DS2PacketToByteString(const DS2PlusPlus::DS2Packet &packet);

#endif // DS2PACKET_H
