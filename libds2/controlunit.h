#ifndef CONTROLUNIT_H
#define CONTROLUNIT_H

#include <QObject>
#include <QHash>
#include <QVariant>
#include <QSharedPointer>

#include "ds2packet.h"
#include "operation.h"

namespace DS2PlusPlus {
    class Manager;

    /*!
     * \brief The ControlUnit class represents a module in thecar.
     */
    class ControlUnit : public QObject
    {
        Q_OBJECT
    public:
        ControlUnit(const QString &aUuid = QString::null, Manager *aParent = 0);

        // Addresses

        static const int ADDRESS_DDE;
        static const int ADDRESS_DME;
        static const int ADDRESS_EWS;
        static const int ADDRESS_IHKA;
        static const int ADDRESS_KOMBI;
        static const int ADDRESS_LSZ;
        static const int ADDRESS_RADIO;
        static const int ADDRESS_ZKE;

        /*!
         * \brief loadByUuid fetches functional infor from the SQL database using the UUID as the key.
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

        quint32 dppVersion() const;
        Q_PROPERTY(quint32 dppVersion MEMBER _dppVersion READ dppVersion)

        quint32 fileVersion() const;
        Q_PROPERTY(quint32 fileVersion MEMBER _fileVersion READ fileVersion)

        const QString uuid() const;
        Q_PROPERTY(const QString uuid MEMBER _uuid READ uuid)

        quint8 address() const;
        Q_PROPERTY(quint8 address MEMBER _address READ address)

        const QString name() const;
        Q_PROPERTY(const QString name MEMBER _name READ name)

        QHash<QString, OperationPtr> operations() const;

        const QHash<QString, QVariant> matches() const;

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

    protected:
        quint32 _dppVersion;
        quint32 _fileVersion;
        QString _uuid;
        quint8 _address;
        QString _family;
        QString _name;
        QHash<QString, OperationPtr> _operations;
        QHash<QString, QVariant> _matches;

        Manager *_manager;
    };

    typedef QSharedPointer<ControlUnit> ControlUnitPtr;
}

#endif // CONTROLUNIT_H
