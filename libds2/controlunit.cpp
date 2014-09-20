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

#include <stdexcept>

#include <QDebug>

#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>

#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>

#include <QtEndian>

#include <QUuid>

#include "operation.h"
#include "controlunit.h"
#include "manager.h"
#include "dpp_v1_parser.h"

namespace DS2PlusPlus {

    QHash<QString, quint8> ControlUnit::_familyDictionary;
    const QString ControlUnit::ROOT_UUID = "00000000-0000-0000-0000-000000000001";

    quint8 ControlUnit::addressForFamily(const QString &aFamily)
    {
        if (_familyDictionary.isEmpty()) {
            _familyDictionary.insert("AIC",     0xE8); // Rain sensor
            _familyDictionary.insert("DME",     0x12); // Digital Motor Electronics
            _familyDictionary.insert("DDE",     0x12); // Digital Diesel Electronics
            _familyDictionary.insert("DSC",     0x56); // Dynamic Stability Control
            _familyDictionary.insert("EGS",     0x32); // Electronic Transmission Control - Electronische Getriebe Steuerung
            _familyDictionary.insert("EWS",     0x44); // Electronic Immobiliser / Elektronische Wegfahrsperre
            _familyDictionary.insert("IHKA",    0x5b); // Auto Climate Control / Integrierte Heizung Kühlung?
            _familyDictionary.insert("KOMBI",   0x80); // Instrument Cluster
            _familyDictionary.insert("LSZ",     0xd0); // Light Switching Center / Lichtschaltzentrum
            _familyDictionary.insert("LWS",     0x57); // Steering Angle Sensor / Lenkwinkelsensor
            _familyDictionary.insert("MRS",     0xA4); // Multiple Restraint System
            _familyDictionary.insert("RADIO",   0x68); // Radio
            _familyDictionary.insert("RLS",     0xE8); // Rain and light sensor
            _familyDictionary.insert("SMG",     0x32); // Sequential M Gearbox
            _familyDictionary.insert("ZKE",     0x00); // Central Body Electronics / Zentrale Karosserieelektronik
        }

        if (_familyDictionary.contains(aFamily)) {
            return _familyDictionary.value(aFamily);
        }

        return -99;
    }

    const QStringList ControlUnit::knownFamilies()
    {
        addressForFamily("");
        return _familyDictionary.keys();
    }

    const QList<quint8> ControlUnit::knownAddresses()
    {
        QSet<quint8> set;
        addressForFamily("");
        foreach (quint8 address, _familyDictionary.values()) {
            set.insert(address);
        }

        QList<quint8> ret(set.toList());
        qSort(ret);
        return ret;
    }

    const QString ControlUnit::familyForAddress(quint8 anAddress)
    {
        QStringList ret;
        addressForFamily("");
        QHashIterator<QString, quint8> i(_familyDictionary);
         while (i.hasNext()) {
             i.next();
             if (i.value() == anAddress) {
                 ret.append(i.key());
             }
         }
         if (ret.isEmpty()) {
            return QString::null;
         } else {
             ret.sort();
             return ret.join(", ");
         }
    }

    ControlUnit::ControlUnit(const QString &aUuid, Manager *aParent) :
        QObject(aParent), _manager(aParent)
    {
        if (_manager == NULL) {
            _manager = new Manager();
        }

        if (!aUuid.isNull()) {
            loadByUuid(aUuid);
        }
    }

    void ControlUnit::loadByUuid(const QString &aUuid)
    {
        QTextStream qOut(stdout);
        QTextStream qErr(stderr);

        _operations.clear();

        QString parent_id = aUuid;
        // Add a "find root UUID" method to dbm
        while (!parent_id.isEmpty()) {
            QHash<QString, QVariant> theRecord = _manager->findModuleRecordByUuid(parent_id);
            if (theRecord.isEmpty()) {
                qDebug() << "Find parent failed";
                throw std::runtime_error("Find parent failed");
            }

            if (aUuid == parent_id) {
                _dppVersion = theRecord.value("dpp_version").toInt();
                _fileVersion = theRecord.value("file_version").toInt();
                _uuid = DPP_V1_Parser::rawUuidToString(theRecord.value("uuid").toByteArray());
                _address = theRecord.value("address").toChar().toLatin1();
                _family = theRecord.value("family").toString();
                _name = theRecord.value("name").toString();
                _partNumber = theRecord.value("part_number").toULongLong();
                _hardwareNumber = theRecord.value("hardware_num").toULongLong();
                _softwareNumber = theRecord.value("software_num").toULongLong();
                _codingIndex = theRecord.value("coding_index").toULongLong();
                _bigEndian = (theRecord.value("big_endian").toULongLong() == 1) ? true : false;
                time_t mtimeInt = theRecord.value("mtime").toULongLong();
                _fileLastModified.setTime_t(mtimeInt);
            }

            if (getenv("DPP_TRACE")) {
                qDebug() << "Module: " << parent_id << " (from: " << aUuid << ")";
            }

            QSharedPointer<QSqlTableModel> operationsTable(_manager->operationsTable());
            operationsTable->setFilter(QString("module_id = %1").arg(DPP_V1_Parser::stringToUuidSQL(parent_id)));
            operationsTable->select();
            for (int i=0; i < operationsTable->rowCount(); i++) {
                QSqlRecord opRecord = operationsTable->record(i);

                QString opName = opRecord.value("name").toString();
                QString opUuid = DPP_V1_Parser::rawUuidToString(opRecord.value("uuid").toByteArray());
                QString opModule = DPP_V1_Parser::rawUuidToString(opRecord.value("module_id").toByteArray());
                QByteArray opCommand = opRecord.value("command").toByteArray();

                OperationPtr op;
                if (_operations.contains(opName)) {
                    op = _operations.value(opName);
                    if (getenv("DPP_TRACE")) {
                        qDebug() << "\tMerging operation: " << opName << " we've a higher priority implementation";
                    }

                    // If we've not yet set the command from a higher priority operation, use this one.
                    if (op->command().isEmpty()) {
                        op->setCommand(opCommand);
                    }
                } else {
                    if (getenv("DPP_TRACE")) {
                        qDebug() << "\tProcessing job " << opName;
                    }
                    op = OperationPtr(new Operation(opUuid, _address, opName, opCommand));
                }

                QSharedPointer<QSqlTableModel> results(_manager->resultsTable());
                results->setFilter(QString("operation_id = %1").arg(DPP_V1_Parser::stringToUuidSQL(opUuid)));
                results->select();

                for (int j=0; j < results->rowCount(); j++) {
                    QSqlRecord resultRecord = results->record(j);
                    Result result;
                    result.setName(resultRecord.value("name").toString());
                    result.setUuid(resultRecord.value("uuid").toString());
                    result.setType(resultRecord.value("type").toString());
                    result.setDisplayFormat(resultRecord.value("display").toString());
                    result.setStartPosition(resultRecord.value("start_pos").toInt());
                    result.setLength(resultRecord.value("length").toInt());
                    result.setMask(resultRecord.value("mask").toString());
                    result.setFactorA(resultRecord.value("factor_a").toDouble());
                    result.setFactorB(resultRecord.value("factor_b").toDouble());
                    result.setRpn(resultRecord.value("rpn").toString());

                    QJsonParseError jsonError;
                    QHash<QString, QString> ourLevels;
                    QByteArray jsonByteArray(qPrintable(resultRecord.value("levels").toString()));
                    QJsonDocument levelsDoc = QJsonDocument::fromJson(jsonByteArray, &jsonError);
                    QJsonObject ourLevelsJson = levelsDoc.object();

                    QJsonObject::Iterator levelsIterator = ourLevelsJson.begin();
                    while (levelsIterator != ourLevelsJson.end()) {
                        QJsonValue level = levelsIterator.value();
                        if (level.isString()) {
                            ourLevels.insert(levelsIterator.key(), levelsIterator.value().toString());
                        }
                        levelsIterator++;
                    }

                    result.setLevels(ourLevels);

                    if (op->results().contains(result.name())) {
                        if (getenv("DPP_TRACE")) {
                            qDebug() << "\t\tSkipping result " << result.name() << " as we've a higher priority implementation";
                        }
                    } else {
                        if (getenv("DPP_TRACE")) {
                            qDebug() << "\t\tAdding result: " << result.name();
                        }
                        op->insertResult(result.name(), result);
                    }
                }

                _operations.insert(opName, op);
            }
            parent_id = DPP_V1_Parser::rawUuidToString(theRecord.value("parent_id").toByteArray());
        }
    }

    DS2Response ControlUnit::executeOperation(const QString &name)
    {
        const OperationPtr ourOp(_operations.value(name));
        DS2PacketPtr ourOutgoingPacket(ourOp->queryPacket());
        DS2PacketPtr ourIncomingPacket(_manager->query(ourOutgoingPacket));

        return parseOperation(ourOp, ourIncomingPacket);
    }

    DS2Response ControlUnit::parseOperation(const QString &name, const DS2PacketPtr packet)
    {
        const OperationPtr theOp = (_operations.value(name));

        if (theOp.isNull()) {
            throw std::invalid_argument(qPrintable(QString("Operation '%1' could not be found in ECU %2").arg(name).arg(_uuid)));
        }

        return parseOperation(theOp, packet);
    }

    DS2Response ControlUnit::parseOperation(const OperationPtr theOp, const DS2PacketPtr packet)
    {
        if (theOp.isNull()) {
            throw std::invalid_argument(qPrintable(QString("parseOperation requires a valid operation...")));
        }

        QTextStream qOut(stdout);
        QTextStream qErr(stderr);

        DS2Response ret;

        if (packet->address() != address()) {
            QString errorString = QString("WARNING: RECV'D PACKET FOR ECU AT 0x%1. OUR ADDR IS 0x%2").arg(packet->address(), 2, 16, QChar('0')).arg(address(), 2, 16, QChar('0'));
            qErr << errorString << endl;
        }

        foreach(const Result &result, theOp->results()) {
            //qDebug() << "Result: " << result.name;

            if ((result.startPosition() >= packet->data().length() ) || ((result.startPosition() + result.length() - 1) >= packet->data().length())) {
                //qDebug() << "Skipping out of range result (" << result.uuid << "/" << result.name;
                continue;
            }
            if (result.isType("byte")) {
                ret.insert(result.name(), resultByteToVariant(packet, result));
            } else if (result.isType("short")) {
                quint16 ourNumber;
                memcpy(&ourNumber, packet->data().mid(result.startPosition(), result.length()), qMin((size_t)result.length(), sizeof(quint16)));
                if (_bigEndian) {
                    ourNumber = qFromBigEndian(ourNumber);
                } else {
                    ourNumber = qFromLittleEndian(ourNumber);
                }

                if (result.displayFormat() == "int") {
                    quint64 value = runRpnForResult<float>(packet, result, ourNumber);
                    ret.insert(result.name(), QVariant(value));
                } else if (result.displayFormat() == "float") {
                    double ourFloat = runRpnForResult<float>(packet, result, ourNumber);

                    ret.insert(result.name(), QVariant(ourFloat + result.factorB()));
                } else {
                    throw std::invalid_argument("Invalid display type for short specified.");
                }
            } else if (result.isType("hex_string")) {
                ret.insert(result.name(), resultHexStringToVariant(packet, result));
            } else if (result.isType("string")) {
                QString string;
                for (int i=0; i < result.length(); i++) {
                    QChar byte(packet->data().at(result.startPosition() + i));
                    string.append(byte);
                }
                if (result.displayFormat() == "string") {
                    ret.insert(result.name(), QVariant(string));
                } else if (result.displayFormat() == "hex_string") {
                    string.prepend("0x");
                    ret.insert(result.name(), QVariant(string));
                } else if (result.displayFormat() == "int") {
                    ret.insert(result.name(), QVariant(string.toULongLong()) );
                } else {
                    QString errorString = QString("Unknown display type for string type: ").arg(result.displayFormat());
                    throw std::invalid_argument(qPrintable(errorString));
                }
            } else if (result.isType("short_vin")) {
                QString vin;
                vin.append(packet->data().at(result.startPosition()));
                vin.append(packet->data().at(result.startPosition() + 1));

                quint32 number_part;
                memcpy(&number_part, packet->data().mid(result.startPosition() + 1, 4), 4);
                number_part = qFromBigEndian(number_part);
                number_part = (number_part & 0x00ffffff) >> 4;
                vin.append(QString::number(number_part, 16));

                ret.insert(result.name(), vin);
            } else if (result.isType("6bit-string")) {
                QByteArray encodedString = packet->data().mid(result.startPosition(), result.length());

                quint16 numBits = result.length() * 8;
                QString decodedString;

                for (int i=0; i < (numBits / 6); i++) {
                  int j = ((result.length()* 8) - (6*(i+1))) - 1;
                  char foo = decode_vin_char(j, encodedString);
                  decodedString.prepend(QChar(foo));
                }
                ret.insert(result.name(), QVariant(decodedString));

            } else if (result.isType("boolean")) {
                if (result.length() != 1) {
                    throw std::invalid_argument("Incorrect length for boolean type encountered");
                }
                unsigned char byte = packet->data().at(result.startPosition());
                bool condition = ((byte & result.mask()) > 0);
                if (result.displayFormat() == "string") {
                    ret.insert(result.name(), QVariant(result.stringForLevel(condition)));
                } else if (result.displayFormat() == "raw") {
                    ret.insert(result.name(), QVariant(condition));
                }
            } else {
                qErr << "Unknown result type: " << result.type() << endl;
            }
        }

        return ret;
    }

    quint32 ControlUnit::dppVersion() const
    {
        return _dppVersion;
    }

    quint32 ControlUnit::fileVersion() const
    {
        return _fileVersion;
    }

    QDateTime ControlUnit::fileLastModified() const
    {
        return _fileLastModified;
    }

    const QString ControlUnit::uuid() const
    {
        return _uuid;
    }

    quint8 ControlUnit::address() const
    {
        return _address;
    }
    void ControlUnit::setAddress(quint8 anAddress)
    {
        _address = anAddress;
        _family = familyForAddress(anAddress).split(", ").at(0);
        foreach (const QString &key, operations().keys()) {
            _operations[key]->setAddress(anAddress);
        }
    }

    QString ControlUnit::family() const
    {
        return _family;
    }

    bool ControlUnit::isRoot() const
    {
        return _uuid == ROOT_UUID;
    }

    const QString ControlUnit::name() const
    {
        return _name;
    }

    QHash<QString, OperationPtr > ControlUnit::operations() const
    {
        return _operations;
    }

    quint64 ControlUnit::partNumber() const {
        return _partNumber;
    }

    quint64 ControlUnit::hardwareNumber() const {
        return _hardwareNumber;
    }

    quint64 ControlUnit::softwareNumber() const {
        return _softwareNumber;
    }

    quint64 ControlUnit::codingIndex() const {
        return _codingIndex;
    }

    bool ControlUnit::bigEndian() const {
        return _bigEndian;
    }

    quint8 ControlUnit::matchFlags() const
    {
        return _matchFlags;
    }

    void ControlUnit::setMatchFlags(quint8 someFlags)
    {
        _matchFlags = someFlags;
    }

    char ControlUnit::decode_vin_char(int start, const QByteArray &bytes)
    {
      quint8 finish = start + 6;
      quint8 start_byte = start / 8;
      quint8 start_bit = (start % 8);
      quint8 finish_byte = finish / 8;
      quint8 finish_bit = finish % 8;

      if (start_byte == finish_byte) {
        quint8 mask = (1 << (finish_bit - start_bit)) - 1;
        return getCharFrom6BitInt(bytes[start_byte] & mask);
      } else {
        start_bit++;

        quint8 high_nibble = bytes[start_byte] & (0xff >> start_bit);
        quint8 low_nibble = bytes[finish_byte] & (0xff << (7 - finish_bit));
        quint8 finish = (high_nibble << (6 - (8 - start_bit))) | (low_nibble >> (7 - finish_bit));

        return getCharFrom6BitInt(finish);
      }
      return -1;
    }

    char ControlUnit::getCharFrom6BitInt(quint8 n)
    {
      if (n >= 0 && n <= 9) {
        return '0' + n;
      } else if (n >= 10 && n <= 35) {
        return 'A' + (n - 10);
      }
      return '!';
    }

    template <typename T> T ControlUnit::runRpnForResult(const DS2PacketPtr aPacket, const Result &aResult, T aValue)
    {
        if (!aResult.rpn().isEmpty()) {
            QList<T> stack;
            foreach (const QString &command, aResult.rpn()) {
                if (command.startsWith("0x")) {
                    bool ok;
                    quint64 theNum = command.toULongLong(&ok, 16);
                    if (ok) {
                        stack.push_front(theNum);
                    } else {
                        throw std::invalid_argument("Argh, tried to parse an invalid hex string");
                    }
                } else if (command == "+")  {
                    T a, b;
                    a = stack.takeFirst();
                    b = stack.takeFirst();
                    stack.push_back(b + a);
                } else if (command == "-")  {
                    T a, b;
                    a = stack.takeFirst();
                    b = stack.takeFirst();
                    stack.push_back(b - a);
                } else if (command == "/")  {
                    T a, b;
                    a = stack.takeFirst();
                    b = stack.takeFirst();
                    stack.push_back(b / a);
                } else if (command == "*")  {
                    T a, b;
                    a = stack.takeFirst();
                    b = stack.takeFirst();
                    stack.push_back(b * a);
                } else if (command == "&")  {
                    T a, b;
                    a = stack.takeFirst();
                    b = stack.takeFirst();
                    stack.push_back(static_cast<quint64>(b) & static_cast<quint64>(a));
                } else if (command == ">>") {
                    T a, b;
                    a = stack.takeFirst();
                    b = stack.takeFirst();
                    stack.push_back(static_cast<quint64>(b) >> static_cast<quint64>(a));
                } else if (command == "<<") {
                    T a, b;
                    a = stack.takeFirst();
                    b = stack.takeFirst();
                    stack.push_back(static_cast<quint64>(b) << static_cast<quint64>(a));
                } else if (command == "N")  {
                    stack.push_back(aValue);
                } else {
                    // Assume it's a base 10 integer
                    bool ok;
                    T theNum;
                    if (command.indexOf('.') > -1) {
                        // Assume float
                        theNum = command.toDouble(&ok);
                    } else {
                        // Assume int
                        theNum = command.toULongLong(&ok, 10);
                    }

                    if (ok) {
                        stack.push_front(theNum);
                    } else {
                        throw std::invalid_argument("Argh, tried to parse an invalid decimal string");
                    }
                }
            }
            return stack.takeFirst();
        }
        return aValue;
    }

    QVariant ControlUnit::resultByteToVariant(const DS2PacketPtr aPacket, const Result &aResult)
    {
        QChar zeroPadding = QChar('0');

        if (aResult.length() != 1) {
            QString ourError = QObject::tr("Length is not one for a byte data type.  Length is %1").arg(aResult.length());
            throw std::invalid_argument(qPrintable(ourError));
        }

        unsigned char byte = aPacket->data().at(aResult.startPosition());
        if (aResult.mask() != 0) {
            byte = (byte & aResult.mask()) & 0xff;
        }

        quint64 num = runRpnForResult<quint64>(aPacket, aResult, byte);

        if (aResult.displayFormat() == "hex_string") {
            QString hex;
            hex = QString("0x%1").arg(QString::number(byte, 16), 2, zeroPadding);
            return QVariant(hex);
        } else if (aResult.displayFormat() == "hex_int") {
            QString hex;
            hex = QString("%1").arg(QString::number(byte, 16), 2, zeroPadding);
            return QVariant(hex.toUInt());
        } else if (aResult.displayFormat() == "raw") {
            return QVariant(num);
        } else if (aResult.displayFormat().startsWith("string_table:")) {
            QString tableName = aResult.displayFormat().mid(13);
            QString stringValue = _manager->findStringByTableAndNumber(tableName, byte);

            if (!stringValue.isEmpty()) {
               return QVariant(stringValue);
            } else {
                QString hex;
                hex = QString("0x%1").arg(QString::number(byte, 16), 2, zeroPadding);
                return QVariant(hex);
            }

            return QVariant(tableName);
        } else if (aResult.displayFormat() == "float") {
            double value = runRpnForResult<double>(aPacket, aResult, static_cast<quint8>(byte));
            value = (byte * aResult.factorA()) + aResult.factorB();
            return QVariant(value);
        } else if (aResult.displayFormat() == "enum") {
            return QVariant(aResult.stringForLevel(byte));
        } else {
            QString ourError = QObject::tr("Unknown display format for byte type: %1").arg(aResult.displayFormat());
            throw std::invalid_argument(qPrintable(ourError));
        }
    }

    QVariant ControlUnit::resultHexStringToVariant(const DS2PacketPtr aPacket, const Result &aResult)
    {
        QChar zeroPadding = QChar('0');
        QString hex;
        for (int i=0; i < aResult.length(); i++) {
            unsigned char byte = aPacket->data().at(aResult.startPosition() + i);
            if (i == 0) {
                hex.append(QString("%1").arg(QString::number(byte & 0x0f, 16)));
            } else {
                hex.append(QString("%1").arg(QString::number(byte, 16), 2, zeroPadding));
            }
        }
        if (aResult.displayFormat() == "int") {
            quint64 number = hex.toULongLong();
            return QVariant(number);
        } else if (aResult.displayFormat() == "string") {
            return QVariant(hex);
        } else {
            QString ourError = QObject::tr("Unknown display format for hex_string type: %1").arg(aResult.displayFormat());
            throw std::runtime_error(qPrintable(ourError));
        }
    }
}
