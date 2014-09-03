#include <json/json.h>

#include "ds2packet.h"

namespace DS2PlusPlus {
    DS2Packet::DS2Packet(QObject *parent) :
        QObject(parent)
    {
    }

    DS2Packet::DS2Packet(quint8 anEcuAddress, const QByteArray &someData, QObject *parent) :
        QObject(parent), _address(anEcuAddress), _data(someData)
    {

    }

    float DS2Packet::fetchFloat(quint16 aPosition, float aMultiplicativeFactor, int anAdditiveFactor) const
    {
        return (static_cast<quint8>(_data.at(aPosition)) * aMultiplicativeFactor) + anAdditiveFactor;
    }

    QString DS2Packet::fetchString(quint16 aPosition, quint16 aLength) const
    {
        return QString(_data.mid(aPosition, aLength));
    }

    unsigned char DS2Packet::checksum(const QByteArray &someData) const
    {
        QByteArray ourData(someData);

        if (ourData.isEmpty()) {
            ourData.append(_address);
            ourData.append(_data.length() + 3);
            ourData.append(_data);
        }

        unsigned char ourChecksum = 0;
        for (int i=0; i < ourData.length(); i++) {
            ourChecksum ^= static_cast<quint8>(ourData.at(i));
        }

        return ourChecksum;
    }

    quint8 DS2Packet::address() const {
        return _address;
    }

    void DS2Packet::setAddress(quint8 anAddress) {
        _address = anAddress;
    }

    QByteArray DS2Packet::data() const {
        return _data;
    }

    void DS2Packet::setData(const QByteArray &someData) {
        _data = someData;
    }

    DS2Packet::operator const QByteArray () const
    {
        QByteArray ret;
        ret.append(_address);
        ret.append(_data.length() + 3);
        ret.append(_data);

        ret.append(checksum(ret));

        return ret;
    }
}

const Json::Value *DS2ResponseToJson(const DS2PlusPlus::DS2Response &aResponse)
{
    using namespace DS2PlusPlus;

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
                jsonValue = Json::Value::Value(qPrintable(variantValue.toString()));
            }
        } else if (
                   (variantValue.type() == static_cast<QVariant::Type>(QMetaType::UInt)) ||
                   (variantValue.type() == static_cast<QVariant::Type>(QMetaType::ULong)) ||
                   (variantValue.type() == static_cast<QVariant::Type>(QMetaType::ULongLong))
                  ) {
            jsonValue  = Json::Value::Value(variantValue.toULongLong());
        } else if ((variantValue.type() == static_cast<QVariant::Type>(QMetaType::Double)) || (variantValue.type() == static_cast<QVariant::Type>(QMetaType::Float))) {
            jsonValue  = Json::Value::Value(variantValue.toDouble());
        } else {
            qDebug() << "Uknown variant type: " << variantValue.typeName();
        }

        (*curVal)[qPrintable(ourHier.last())] = jsonValue;

    }

    return new Json::Value(root);
}

const QString DS2ResponseToString(const DS2PlusPlus::DS2Response &aResponse)
{
    const Json::Value *ourJson = DS2ResponseToJson(aResponse);
    std::string ourStr = ourJson->toStyledString();
    delete ourJson;
    return QString(ourStr.c_str());
}

QDebug operator << (QDebug d, const DS2PlusPlus::DS2Packet &packet)
{
    QString debugString("ECU: %1 DATA: %2 CHECKSUM: %3");
    QChar zeroPadding('0');
    QString ecuString = QString("0x%1").arg(packet.address(), 2, 16, zeroPadding);

    QByteArray data = packet.data();
    QStringList dataStringList;
    for (int i=0; i < data.length(); i++) {
        dataStringList.append(QString("0x%1").arg(static_cast<quint8>(data.at(i)), 2, 16, zeroPadding));
    }

    QString checksumString = QString("0x%1").arg(packet.checksum(), 2, 16, zeroPadding);

    d << debugString.arg(ecuString).arg(dataStringList.join(", ")).arg(checksumString);

    return d;
}
