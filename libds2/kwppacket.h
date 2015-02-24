#ifndef KWPPACKET_H
#define KWPPACKET_H

#include <QObject>
#include <QByteArray>
#include <QHash>
#include <QVariant>
#include <QDebug>

#include "basepacket.h"

namespace DS2PlusPlus {
    class KWPPacket : public BasePacket
    {
        Q_OBJECT
    public:
        explicit KWPPacket(QObject *parent = 0);
        KWPPacket(const QString &aPacketString);
        KWPPacket(quint8 aTargetAddress, quint8 aSourceAddress, const QByteArray &someData, QObject *aParent = 0);

        /*!
         * \brief checksum calculates the checksum for a packet (or for a given QByteArray)
         * \param data
         * \return
         */
        virtual unsigned char checksum(const QByteArray &data = QByteArray()) const;
        virtual const QString toByteString () const;

    protected:
        virtual const QByteArray toByteArray() const;
    };
    typedef QSharedPointer<KWPPacket> KWPPacketPtr;
}

#endif // KWPPACKET_H
