#include <stdexcept>

#include <json/json.h>

#include <QHash>
#include <QStringList>
#include <QVariant>
#include <QDebug>
#include <QSharedPointer>
#include <QByteArray>

#include <ds2/basepacket.h>

namespace DS2PlusPlus {
    const char *BasePacket::HEX_CHAR_FORMAT = "%02X";

    BasePacket::BasePacket(QObject *parent) :
        QObject(parent), _hasSourceAddress(false)
    {
    }

    BasePacket::BasePacket(quint8 aTargetAddress, const QByteArray &someData, QObject *parent) :
        QObject(parent), _hasSourceAddress(false), _targetAddress(aTargetAddress), _data(someData)
    {

    }

    BasePacket::BasePacket(quint8 aTargetAddress, quint8 aSourceAddress, const QByteArray &someData, QObject *parent) :
        QObject(parent), _sourceAddress(aSourceAddress), _targetAddress(aTargetAddress), _data(someData)
    {

    }

    QString BasePacket::prettyPrintPartNumber(const QString &aPartNumber)
    {
        QString ourPartNumber = aPartNumber;
        bool partNumberIsNumeric;

        if (ourPartNumber.length() == 7 and ourPartNumber.toULongLong(&partNumberIsNumeric) == aPartNumber.toULongLong() and partNumberIsNumeric == true) {
            ourPartNumber = QString("%1.%2.%3").arg(ourPartNumber.mid(0, 1)).arg(ourPartNumber.mid(1, 3)).arg(ourPartNumber.mid(4, 3));
        }

        return ourPartNumber;
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
            } else if (strcmp(variantValue.typeName(), "QList<uchar>")==0) {
                jsonValue = Json::Value(Json::arrayValue);

                QList<quint8> ourQList = variantValue.value<QList<quint8> >();
                foreach (quint8 address, ourQList) {
                    jsonValue.append(address);
                }
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

    const QString HashToJsonString(const QHash<QString, QVariant> &aResponse, const QString &aRootNode) {
        std::string ourStr;
        const Json::Value *jsonNode = ResponseToJson(aResponse);

        if (!aRootNode.isEmpty()) {
            Json::Value realRoot;
            realRoot[qPrintable(aRootNode)] = (*jsonNode);
            ourStr = realRoot.toStyledString();
        } else {
            ourStr = jsonNode->toStyledString();
        }

        delete jsonNode;
        return QString(ourStr.c_str());
    }

    BasePacket::operator QByteArray () const
    {
        QByteArray ret = this->toByteArray();

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
    d << packet.toByteString();
    return d;
}
