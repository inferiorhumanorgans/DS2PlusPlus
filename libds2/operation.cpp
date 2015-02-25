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

#include "controlunit.h"
#include "operation.h"
#include "ds2packet.h"
#include "kwppacket.h"

namespace DS2PlusPlus {

    Operation::Operation (const QString &aUuid, quint8 aControlUnitAddress, const QString &aName, const QByteArray &aCommand, BasePacket::ProtocolType aProtocol)
        : _uuid(aUuid), _name(aName), _controlUnitAddress(aControlUnitAddress), _command(aCommand), _protocol(aProtocol)
    {
    }

    const QString Operation::uuid() const
    {
        return _uuid;
    }

    const QString Operation::name() const
    {
        return _name;
    }

    const QString Operation::parentId() const
    {
        return _parentId;
    }

    void Operation::setParentId(const QString &parentId)
    {
        _parentId = parentId;
    }

    const QStringList Operation::command() const
    {
        QStringList ret;
        for (int i=0; i < _command.length(); i++) {
            ret.append(QString("0x%1").arg(static_cast<quint8>(_command.at(i)), 2, 16, QChar('0')));
        }

        return ret;
    }

    void Operation::setCommand(const QByteArray &aCommand)
    {
        _command = aCommand;
    }

    const QHash<QString, Result> Operation::results() const
    {
        return _results;
    }

    void Operation::insertResult(const QString &aName, const Result aResult)
    {
        _results.insert(aName, aResult);
    }

    BasePacket::ProtocolType Operation::protocol() const
    {
        return _protocol;
    }

    BasePacket *Operation::queryPacket() const
    {
        switch (_protocol) {
        case BasePacket::ProtocolDS2:
            return new DS2Packet(_controlUnitAddress, _command);
            break;
        case BasePacket::ProtocolKWP:
            return new KWPPacket(_controlUnitAddress, 0xF1, _command);
            break;
        default:
            throw std::invalid_argument("Invalid protocol.");
            break;
        }
    }

    void Operation::setAddress(quint8 anAddress)
    {
        _controlUnitAddress = anAddress;
    }
}
