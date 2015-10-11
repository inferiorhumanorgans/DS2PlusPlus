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
     * \brief An arbitrary computer module in a car that can execute operations and return results.
     */
    class ControlUnit : public QObject
    {
        Q_OBJECT
    public:
        static const QString ROOT_UUID;

        ControlUnit(const QString &aUuid = QString::null, Manager *aParent = 0);

        static QList<quint8> addressForFamily(const QString &aFamily);
        static const QString familyForAddress(quint8 anAddress);
        static const QStringList knownFamilies();
        static const QList<quint8> knownAddresses();
        static const QString nameForFamily(const QString &aFamily);

        enum MatchType {
            MatchNone       = 0x00,
            MatchSW         = 0x01, // Software version matches
            MatchHW         = 0x02, // Hardawre version matches
            MatchCI         = 0x04, // Coding index matches
            MatchDI         = 0x08, // Diag Index matches
            MatchBI         = 0x10, // Bus Index matches
            MatchPN         = 0x20, // Part number matches
            MatchAll        = 0x7F, // Everything matches
        };

        /*!
         * \brief Fetches functional info from the SQL database using the UUID as the key.
         * \param aUuid
         */
        void loadByUuid(const QString &aUuid);

        /*!
         * \brief Sends a named operation to the ECU and returns the parsed response.
         * \param aName Name of the operation
         * \return
         */
        virtual PacketResponse executeOperation(const QString &aName);

        /*!
         * \brief Parses a BasePacket for a given operation.
         *
         * Typically the packet passed to this function is the packet received from a ControlUnit in response to an operation.
         * \param aName The name of an Operation
         * \param aPacket A BasePacket instance containing data to parse.
         * \return A PacketResponse instance with the data parsed out into named key value pairs with the data represented by QVariant objects.
         */
        virtual PacketResponse parseOperation(const QString &aName, const BasePacketPtr aPacket);

        /*!
         * \brief Parses a BasePacket for a given operation.
         *
         * Typically the packet passed to this function is the packet received from a ControlUnit in response to an operation.
         * \param aPacket A BasePacket instance containing data to parse.
         * \return A PacketResponse instance with the data parsed out into named key value pairs with the data represented by QVariant objects.
         */
        virtual PacketResponse parseOperation(const OperationPtr anOperation, const BasePacketPtr aPacket);

        /*! \brief Returns the \ref dppVersion property. */
        quint32 dppVersion() const;

        /*! \brief The version of the DPP JSON definition file format used to initialize this object. */
        Q_PROPERTY(quint32 dppVersion MEMBER _dppVersion READ dppVersion)

        /*! \brief Returns The \ref fileVersion property. */
        quint32 fileVersion() const;

        /*! \brief The revision of the JSON definition file used to initialize this object. */
        Q_PROPERTY(quint32 fileVersion MEMBER _fileVersion READ fileVersion)

        /*! \brief Returns the \ref fileLastModified property. */
        QDateTime fileLastModified() const;

        /*! \brief The timestamp of the last time the source file was modified. */
        Q_PROPERTY(QDateTime fileLastModified MEMBER _fileLastModified READ fileLastModified)

        /*! \brief Returns the \ref uuid property. */
        const QString uuid() const;

        /*! \brief The UUID of the JSON definition file used to initialize this object. */
        Q_PROPERTY(const QString uuid MEMBER _uuid READ uuid)

        /*! \brief Returns the \ref address property. */
        quint8 address() const;

        /*!
         * \brief Sets the \ref address property of this ControlUnit.
         * \param anAddress A valid address from 0-255.
         */
        void setAddress(quint8 anAddress);

        /*! \brief The (target) address of this ControlUnit. */
        Q_PROPERTY(quint8 address MEMBER _address READ address WRITE setAddress)

        /*! \brief Returns the \ref family property. */
        QString family() const;

        /*!
         * \brief The family (ex: DME, LWS, ZKE) that this ControlUnit belongs to.
         */
        Q_PROPERTY(QString family MEMBER _family READ family)

        /*! \brief Returns the \ref isRoot property */
        bool isRoot() const;

        /*! \brief True if this is the abstract root ControlUnit */
        Q_PROPERTY(bool isRoot READ isRoot)

        /*! \brief Returns the \ref name property */
        const QString name() const;

        /*! \brief The name of the ControlUnit in plain English. */
        Q_PROPERTY(const QString name MEMBER _name READ name)

        /*! \brief Returns the \ref partNumbers property. */
        QSet<quint64> partNumbers() const;

        /*! \brief All of the part numbers that apply to this ControlUnit. */
        Q_PROPERTY(QSet<quint64> partNumbers MEMBER _partNumbers READ partNumbers)

        /*! \brief Returns the \ref hardwareNumber property. */
        quint64 hardwareNumber() const;

        /*! \brief The hardware version of this control unit */
        Q_PROPERTY(quint64 hardwareNumber MEMBER _hardwareNumber READ hardwareNumber)

        /*! \brief Returns the \ref softwareNumber property. */
        quint64 softwareNumber() const;

        /*! \brief The software version of this control unit */
        Q_PROPERTY(quint64 softwareNumber MEMBER _softwareNumber READ softwareNumber)

        /*! \brief Returns the \ref codingIndex property. */
        quint64 codingIndex() const;

        /*! \brief The coding index of a ControlUnit. */
        Q_PROPERTY(quint64 codingIndex MEMBER _codingIndex READ codingIndex)

        /*! \brief Returns the \ref diagIndexes property. */
        QSet<quint64> diagIndexes() const;

        /*! \brief All of the part numbers that apply to this ControlUnit. */
        Q_PROPERTY(QSet<quint64> diagIndexes MEMBER _diagIndexes READ diagIndexes)

        /*! \brief Returns the \ref bigEndian property */
        bool bigEndian() const;

        /*!
         * \brief The endianness of this module.
         *
         * True if this module represents multi-word values in big-endian format, false if this module
         * represents multi-word values in little-endian format.
         */
        Q_PROPERTY(bool bigEndian MEMBER _bigEndian READ bigEndian)

        BasePacket::ProtocolType protocol() const;
        Q_PROPERTY(BasePacket::ProtocolType protocol MEMBER _protocol READ protocol)

        QHash<QString, OperationPtr> operations() const;

        quint8 matchFlags() const;
        void setMatchFlags(quint8 someFlags);
        Q_PROPERTY(quint8 matchFlags MEMBER _matchFlags READ matchFlags WRITE setMatchFlags)

    protected:
        /*!
         * \brief resultByteToVariant handles parsing byte and signed_byte data types
         * \param aPacket
         * \param aResult
         * \return
         *
         * Types handled include: integers, floats, and strings from a lookup table
         */
        QVariant resultByteToVariant(const BasePacketPtr aPacket, const Result &aResult);

        /*!
         * \brief resultShortToVariant handles parsing two word data types including short and signed_short
         * \param aPacket
         * \param aResult
         * \return
         *
         * Types handled include: integers, floats
         */
        QVariant resultShortToVariant(const BasePacketPtr aPacket, const Result &aResult);

        QVariant resultHexStringToVariant(const BasePacketPtr aPacket, const Result &aResult);

        static char getCharFrom6BitInt(quint8 n);

        static char decode_vin_char(int start, const QByteArray &bytes);

        /*!
         * \brief runRpnForResult runs the RPN calculations listed for a given result.
         */
        template <typename X> static X runRpnForResult(const Result &aResult, X aValue);

    /*! \cond internal */
    protected:
        quint32 _dppVersion;
        quint32 _fileVersion;
        QDateTime _fileLastModified;
        QString _uuid;
        quint8 _address;
        QString _family;
        QString _name;
        QHash<QString, OperationPtr> _operations;
        QSet<quint64> _partNumbers, _diagIndexes;
        quint64 _hardwareNumber, _softwareNumber, _codingIndex;
        bool _bigEndian;
        quint8 _matchFlags;
        BasePacket::ProtocolType _protocol;

        Manager *_manager;
        static QHash<QString, QList<quint8> > _familyDictionary;
        static QHash<QString, QString> _familyNames;
        static const QChar zeroPadding;
    /*! \endcond internal */
    };

    typedef QSharedPointer<ControlUnit> ControlUnitPtr;
}

#endif // CONTROLUNIT_H
