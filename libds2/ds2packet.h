#ifndef DS2PACKET_H
#define DS2PACKET_H

#include <QObject>
#include <QByteArray>
#include <QHash>
#include <QVariant>
#include <QDebug>

#define PACKET_FROM_CHARS(address, array) new DS2PlusPlus::DS2Packet(address, QByteArray(array, sizeof(array) / sizeof(char)))

namespace DS2PlusPlus
{
    /*!
     * \brief The DS2Response class encapsulates a parsed response from an ECU.
     */
    typedef QHash<QString, QVariant> DS2Response;

    /*!
     * \brief The DS2Packet class represents a raw DS2 packet sent to or received from an ECU.
     */
    class DS2Packet : public QObject
    {
        Q_OBJECT
    public:
        /*!
         * \brief Constructs an empty DS2Packet
         * \param aParent A QObject parent
         */
        explicit DS2Packet(QObject *aParent = 0);

        /*!
         * \brief Constructs a DS2Packet and populates it with the appropriate data.
         * \param anAddress An ECU address
         * \param someData The payload.
         * \param aParent  A QObject parent.
         */
        DS2Packet(quint8 anAddress, const QByteArray &someData, QObject *aParent = 0);

        quint8 address() const;
        void setAddress(quint8 anAddress);
        Q_PROPERTY(quint8 address MEMBER _address READ address WRITE setAddress)

        QByteArray data() const;
        void setData(const QByteArray &someData);
        Q_PROPERTY(QByteArray data MEMBER _data READ data WRITE setData)

        /*!
         * \brief checksum calculates the checksum for a packet (or for a given QByteArray)
         * \param data
         * \return
         */
        unsigned char checksum(const QByteArray &data = QByteArray()) const;

        float fetchFloat(quint16 aPosition, float aMultiplicativeFactor, int anAdditiveFactor = 0) const;
        QString fetchString(quint16 aPosition, quint16 aLength) const;

        operator const QByteArray() const;

    protected:
        quint8 _address;
        QByteArray _data;
    };
    typedef QSharedPointer<DS2Packet> DS2PacketPtr;
}

namespace Json {
    class Value;
};
const Json::Value *DS2ResponseToJson(const DS2PlusPlus::DS2Response &aResponse);
const QString DS2ResponseToString(const DS2PlusPlus::DS2Response &aResponse);

QDebug operator << (QDebug d, const DS2PlusPlus::DS2Packet &packet);

#endif // DS2PACKET_H
