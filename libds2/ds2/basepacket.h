#ifndef BASEPACKET_H
#define BASEPACKET_H

#include <QObject>
#include <QByteArray>
#include <QHash>
#include <QVariant>
#include <QDebug>

namespace Json {
    class Value;
};

namespace DS2PlusPlus {
    /*!
     * \brief The PacketResponse class encapsulates a parsed response from an ECU.
     */
    typedef QHash<QString, QVariant> PacketResponse;
    const Json::Value *ResponseToJson(const PacketResponse &aResponse);
    const QString ResponseToJsonString(const PacketResponse &aResponse);
    const QString HashToJsonString(const PacketResponse &aResponse, const QString &aRootNode = QString::null);

    /*!
     * \brief The BasePacket class is an abstract class representing a data packet from a ControlUnit.
     */
    class BasePacket : public QObject
    {
        Q_OBJECT
    public:
        /*!
          * \brief The protocol used by a packet object.  Most supported modules use BMW's DS2 protocol, while Bosch Motronic units appear to use a KWP like protocol.
          */
        typedef enum {
            ProtocolNone    = 0x00,
            ProtocolDS2     = 0x01,
            ProtocolKWP     = 0x02,
            ProtocolUnknown = 0x04
        } ProtocolType;

        /*!
         * \brief Constructs an empty BasePacket.
         * \param parent
         */
        explicit BasePacket(QObject *parent = 0);

        /*!
         * \brief Constructs a BasePacket and populates it with the appropriate data.
         * \param anAddress An ECU address
         * \param someData The payload.
         * \param aParent  A QObject parent.
         */
        BasePacket(quint8 aTargetAddress, const QByteArray &someData, QObject *aParent = 0);

        /*!
         * \brief Constructs a BasePacket and populates it with the appropriate data.
         * \param anAddress An ECU address
         * \param someData The payload.
         * \param aParent  A QObject parent.
         */
        BasePacket(quint8 aTargetAddress, quint8 aSourceAddress, const QByteArray &someData, QObject *aParent = 0);

        /*!
         * \brief prettyPrintPartNumber takes a seven character string and adds BMW-style delimiters to it
         * \param aPartNumber A string representing the unformated, short part number (ex: 1234567)
         * \return A formatted QString representing the delimited part number (ex: 1.234.567)
         */
        static QString prettyPrintPartNumber(const QString &aPartNumber);

        quint8 targetAddress() const;
        void setTargetAddress(quint8 anAddress);

        /*!
         * \brief The target address of a packet.  The DS2 protocol doesn't make a source/target address distinction, only KWP does.
         */
        Q_PROPERTY(quint8 targetAddress MEMBER _targetAddress READ targetAddress WRITE setTargetAddress)

        bool hasSourceAddress() const {return _hasSourceAddress;}
        quint8 sourceAddress() const;
        void setSourceAddress(quint8 anAddress);
        Q_PROPERTY(quint8 sourceAddress MEMBER _sourceAddress READ sourceAddress WRITE setSourceAddress)

        QByteArray data() const;
        void setData(const QByteArray &someData);
        Q_PROPERTY(QByteArray data MEMBER _data READ data WRITE setData)

        /*!
         * \brief checksum calculates the checksum for a packet (or for a given QByteArray)
         * \param data
         * \return
         */
        virtual unsigned char checksum(const QByteArray &data = QByteArray()) const = 0;

        virtual ProtocolType protocol() const = 0;

        virtual const QString toByteString() const = 0;

        virtual QByteArray expectedHeaderPadding() const = 0;

        /*!
         * \brief operator QByteArray turns a packet object into a QByteArray suitable for serializing.
         */
        operator QByteArray() const;

        /*!
         * \brief HEX_CHAR_FORMAT The printf format string we use to format base-16 integers for human consumption
         */
        static const char *HEX_CHAR_FORMAT;

    protected:
        virtual const QByteArray toByteArray() const = 0;

        /*! \cond internal */
        bool _hasSourceAddress;
        quint8 _sourceAddress, _targetAddress;
        QByteArray _data;
        /*! \endcond internal */
    };

    typedef QSharedPointer<BasePacket> BasePacketPtr;
}

QTextStream &operator << (QTextStream &s, const DS2PlusPlus::BasePacket &packet);
QTextStream &operator << (QTextStream &s, DS2PlusPlus::BasePacketPtr packet);
QDebug operator << (QDebug d, const DS2PlusPlus::BasePacket &packet);

#endif // BASEPACKET_H
