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

#ifndef OPERATION_H
#define OPERATION_H

#include <QString>
#include <QStringList>
#include <QHash>

#include <result.h>
#include <ds2packet.h>

namespace DS2PlusPlus {
    class Operation
    {
    public:
        Operation (const QString &aUuid, quint8 aControlUnitAddress, const QString &aName, const QByteArray &aCommand);

        const QString uuid() const;
        const QString moduleId() const;

        const QString name() const;
        const QStringList command() const;
        void setCommand(const QByteArray &aCommand);

        const QHash<QString, Result> results() const;
        void insertResult(const QString &aName, const Result aResult);

        void setAddress(quint8 anAddress);

        DS2Packet *queryPacket() const;

    protected:
        QString _uuid, _name;
        quint8 _controlUnitAddress;
        QByteArray _command;
        QHash<QString, Result> _results;
    };

    typedef QSharedPointer<Operation> OperationPtr;
}

#endif // OPERATION_H
