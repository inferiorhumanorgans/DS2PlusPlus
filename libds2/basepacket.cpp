#include <stdexcept>

#include <json/json.h>

#include <QHash>
#include <QStringList>
#include <QVariant>
#include <QDebug>
#include <QSharedPointer>
#include <QByteArray>

#include "basepacket.h"

namespace DS2PlusPlus {
    BasePacket::BasePacket(QObject *parent) :
        QObject(parent), _hasSourceAddress(false)
    {
    }

    BasePacket::BasePacket(quint8 aTargetAddress, const QByteArray &someData, QObject *parent) :
        QObject(parent), _targetAddress(aTargetAddress), _data(someData)
    {

    }

    BasePacket::BasePacket(quint8 aTargetAddress, quint8 aSourceAddress, const QByteArray &someData, QObject *parent) :
        QObject(parent), _sourceAddress(aSourceAddress), _targetAddress(aTargetAddress), _data(someData)
    {

    }

    float BasePacket::fetchFloat(quint16 aPosition, float aMultiplicativeFactor, int anAdditiveFactor) const
    {
        return (static_cast<quint8>(_data.at(aPosition)) * aMultiplicativeFactor) + anAdditiveFactor;
    }

    QString BasePacket::fetchString(quint16 aPosition, quint16 aLength) const
    {
        return QString(_data.mid(aPosition, aLength));
    }

    quint8 BasePacket::sourceAddress() const {
        if (_hasSourceAddress) {
            return _sourceAddress;
        } else {
            throw std::runtime_error("This type of packet doesn't have a source address");
        }
    }

    void BasePacket::setSourceAddress(quint8 anAddress) {
        _sourceAddress = anAddress;
    }

    quint8 BasePacket::targetAddress() const {
        return _targetAddress;
    }

    void BasePacket::setTargetAddress(quint8 anAddress) {
        _targetAddress = anAddress;
    }

    QByteArray BasePacket::data() const {
        return _data;
    }

    void BasePacket::setData(const QByteArray &someData) {
        _data = someData;
    }

    const Json::Value *ResponseToJson(const DS2PlusPlus::PacketResponse &aResponse) {
        Json::Value root;
        foreach (const QString &key, aResponse.keys()) {
            QStringList ourHier = key.split(".");

            Json::Value *curVal = &root;

            for (int i=0; i < ourHier.length() - 1; i++) {
                QString currentKey = ourHier.at(i);
                QString nextKey;
                bool currentType = false, nextType = false;
                unsigned int curIdx = currentKey.toUInt(&currentType);
                unsigned int nxtIdx = -1;

                if (i < (ourHier.length() - 2)) {
                    nextKey = ourHier.at(i+1);
                    nxtIdx = nextKey.toUInt(&nextType);
                }

                if ((*curVal).isArray()) {
                    bool isNull = ((*curVal)[curIdx].isNull());
                    if (isNull) {
                        (*curVal)[curIdx] = nextType ? Json::Value(Json::arrayValue) : Json::Value(Json::objectValue);
                    }
                    curVal = &(*curVal)[curIdx];
                } else {
                    bool isNull = ((*curVal)[qPrintable(currentKey)].isNull());
                    if (isNull) {
                        (*curVal)[qPrintable(currentKey)] = nextType ? Json::Value(Json::arrayValue) : Json::Value(Json::objectValue);
                    }
                    curVal = &(*curVal)[qPrintable(currentKey)];
                }
            }

            Json::Value jsonValue;
            QVariant variantValue = aResponse.value(key);

            if (variantValue.type() == static_cast<QVariant::Type>(QMetaType::QString)) {
                QString ourString = variantValue.toString();

                if (ourString.isEmpty()) {
                    jsonValue = Json::nullValue;
                } else {
                    jsonValue = Json::Value(qPrintable(variantValue.toString()));
                }
            } else if (
                       (variantValue.type() == static_cast<QVariant::Type>(QMetaType::Int)) ||
                       (variantValue.type() == static_cast<QVariant::Type>(QMetaType::Long)) ||
                       (variantValue.type() == static_cast<QVariant::Type>(QMetaType::LongLong))
                       ) {
                jsonValue  = Json::Value(variantValue.toLongLong());
            } else if (
                       (variantValue.type() == static_cast<QVariant::Type>(QMetaType::UInt)) ||
                       (variantValue.type() == static_cast<QVariant::Type>(QMetaType::ULong)) ||
                       (variantValue.type() == static_cast<QVariant::Type>(QMetaType::ULongLong))
                      ) {
                jsonValue  = Json::Value(variantValue.toULongLong());
            } else if ((variantValue.type() == static_cast<QVariant::Type>(QMetaType::Double)) || (variantValue.type() == static_cast<QVariant::Type>(QMetaType::Float))) {
                jsonValue  = Json::Value(variantValue.toDouble());
            } else {
                qDebug() << "Uknown variant type: " << variantValue.typeName();
            }

            (*curVal)[qPrintable(ourHier.last())] = jsonValue;

        }

        return new Json::Value(root);
    }

    const QString ResponseToJsonString(const PacketResponse &aResponse) {
        const Json::Value *ourJson = ResponseToJson(aResponse);
        std::string ourStr = ourJson->toStyledString();
        delete ourJson;
        return QString(ourStr.c_str());
    }

    BasePacket::operator const QByteArray () const
    {
        QByteArray ret;

        if (_hasSourceAddress) {
            ret.append(0xB8);
            ret.append(_targetAddress);
            ret.append(_sourceAddress);
            ret.append(_data.length());
        } else {
            ret.append(_targetAddress);
            ret.append(_data.length() + 3);
        }
        ret.append(_data);
        ret.append(checksum(ret));

        if (_hasSourceAddress) {
            ret.data()[1] = 0x12;
        }

        if (getenv("DPP_DEBUG_CONVERSION")) {
            for (int i=0; i < ret.length(); i++) {
                printf("%d: 0x%02x\n", i, static_cast<quint8>(ret.at(i)));
            }
        }
        return ret;
    }
}

QTextStream &operator << (QTextStream &s, const DS2PlusPlus::BasePacket &packet)
{
    s << packet.toByteString();
    return s;
}

QTextStream &operator << (QTextStream &s, DS2PlusPlus::BasePacketPtr packet)
{
    s << *packet;
    return s;
}

QDebug operator << (QDebug d, const DS2PlusPlus::BasePacket &packet)
{
    QString debugString("TARGET: %1 SOURCE: %2 LENGTH: %3 CHECKSUM: %4 RAW: %5");
    QChar zeroPadding('0');

    QString targetString = QString("0x%1").arg(packet.targetAddress(), 2, 16, zeroPadding);
    QString sourceString("--");
    if (packet.hasSourceAddress()) {
        sourceString.sprintf("0x%02X", packet.sourceAddress());
    }
    QString checksumString = QString("0x%1").arg(packet.checksum(), 2, 16, zeroPadding);

    d << debugString
         .arg(targetString)
         .arg(sourceString)
         .arg(packet.hasSourceAddress() ? packet.data().length() : packet.data().length() + 3)
         .arg(checksumString)
         .arg(packet.toByteString());

    return d;
}
