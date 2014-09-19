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

#ifndef CONTROLUNIT_H
#define CONTROLUNIT_H

#include <QObject>
#include <QDateTime>
#include <QHash>
#include <QVariant>
#include <QSharedPointer>

#include "ds2packet.h"
#include "operation.h"

namespace DS2PlusPlus {
    class Manager;

    /*!
     * \brief The ControlUnit class represents a module in a car.
     */
    class ControlUnit : public QObject
    {
        Q_OBJECT
    public:
        static const QString ROOT_UUID;

        ControlUnit(const QString &aUuid = QString::null, Manager *aParent = 0);

        static quint8 addressForFamily(const QString &aFamily);
        static const QString familyForAddress(quint8 anAddress);
        static const QStringList knownFamilies();
        static const QList<quint8> knownAddresses();

        enum MatchType {
            MatchNone       = 0x00, // Nothing matches
            MatchSWMismatch = 0x01, // Software version mismatches
            MatchHWMismatch = 0x02, // Hardawre version mismatches
            MatchCIMismatch = 0x04, // Coding index unexpected
            MatchAll        = 0x10, // Everything matches
        };

        /*!
         * \brief loadByUuid fetches functional info from the SQL database using the UUID as the key.
         * \param aUuid
         */
        void loadByUuid(const QString &aUuid);

        /*!
         * \brief executeOperation sends a named operation to the ECU and returns the parsed response.
         * \param aName Name of the operation
         * \return
         */
        virtual DS2Response executeOperation(const QString &aName);

        /*!
         * \brief parseOperation looks up a named operation, parses and returns a DS2Packet (typically fetched from the car)
         * \param aName
         * \param aPacket
         * \return
         */
        virtual DS2Response parseOperation(const QString &aName, const DS2PacketPtr aPacket);

        /*!
         * \brief parseOperation parses and returns a DS2Packet (typically fetched from the car) for an Operation object
         * \param anOperation
         * \param aPacket
         * \return
         */
        virtual DS2Response parseOperation(const OperationPtr anOperation, const DS2PacketPtr aPacket);

        /*!
         * \brief dppVersion
         * \return Returns the version of the DPP JSON definition file format used to initialize this object.
         */
        quint32 dppVersion() const;
        Q_PROPERTY(quint32 dppVersion MEMBER _dppVersion READ dppVersion)

        /*!
         * \brief fileVersion
         * \return Returns the revision of the JSON definition file used to initialize this object.
         */
        quint32 fileVersion() const;
        Q_PROPERTY(quint32 fileVersion MEMBER _fileVersion READ fileVersion)

        QDateTime fileLastModified() const;
        Q_PROPERTY(QDateTime fileLastModified MEMBER _fileLastModified READ fileLastModified)

        /*!
         * \brief uuid
         * \return Returns the UUID of the JSON definition file used to initialize this object.
         */
        const QString uuid() const;
        Q_PROPERTY(const QString uuid MEMBER _uuid READ uuid)

        /*!
         * \brief address
         * \return Returns the address of this ECU
         */
        quint8 address() const;
        void setAddress(quint8 anAddress);
        Q_PROPERTY(quint8 address MEMBER _address READ address WRITE setAddress)

        QString family() const;
        Q_PROPERTY(QString family MEMBER _family READ family)

        bool isRoot() const;
        Q_PROPERTY(bool isRoot READ isRoot)

        /*!
         * \brief name
         * \return Returns the plain English name of this ECU
         */
        const QString name() const;
        Q_PROPERTY(const QString name MEMBER _name READ name)

        quint64 partNumber() const;
        Q_PROPERTY(quint64 partNumber MEMBER _partNumber READ partNumber)

        quint64 hardwareNumber() const;
        Q_PROPERTY(quint64 hardwareNumber MEMBER _hardwareNumber READ hardwareNumber)

        quint64 softwareNumber() const;
        Q_PROPERTY(quint64 softwareNumber MEMBER _softwareNumber READ softwareNumber)

        quint64 codingIndex() const;
        Q_PROPERTY(quint64 codingIndex MEMBER _codingIndex READ codingIndex)

        bool bigEndian() const;
        Q_PROPERTY(bool bigEndian MEMBER _bigEndian READ bigEndian)

        QHash<QString, OperationPtr> operations() const;

        quint8 matchFlags() const;
        void setMatchFlags(quint8 someFlags);
        Q_PROPERTY(quint8 matchFlags MEMBER _matchFlags READ matchFlags WRITE setMatchFlags)

    protected:
        /*!
         * \brief resultByteToVariant handles parsing any byte sized data type
         * \param aPacket
         * \param aResult
         * \return
         *
         * Types handled include: integers, floats, and strings from a lookup table
         */
        QVariant resultByteToVariant(const DS2PacketPtr aPacket, const Result &aResult);
        QVariant resultHexStringToVariant(const DS2PacketPtr aPacket, const Result &aResult);
        char getCharFrom6BitInt(quint8 n);
        char decode_vin_char(int start, const QByteArray &bytes);

        template <typename X> X runRpnForResult(const DS2PacketPtr aPacket, const Result &aResult, X aValue);

    protected:
        quint32 _dppVersion;
        quint32 _fileVersion;
        QDateTime _fileLastModified;
        QString _uuid;
        quint8 _address;
        QString _family;
        QString _name;
        QHash<QString, OperationPtr> _operations;
        quint64 _partNumber, _hardwareNumber, _softwareNumber, _codingIndex;
        bool _bigEndian;
        quint8 _matchFlags;

        Manager *_manager;
        static QHash<QString, quint8> _familyDictionary;
    };

    typedef QSharedPointer<ControlUnit> ControlUnitPtr;
}

#endif // CONTROLUNIT_H
