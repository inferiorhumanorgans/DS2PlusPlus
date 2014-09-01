#include <stdexcept>

#include <QDebug>

#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>

#include <QSerialPort>

#include <QJsonDocument>
#include <QJsonParseError>

#include <QtEndian>

#include "operation.h"
#include "controlunit.h"
#include "manager.h"

namespace DS2PlusPlus {
    const int ControlUnit::ADDRESS_DDE   = 0x12;
    const int ControlUnit::ADDRESS_DME   = 0x12;
    const int ControlUnit::ADDRESS_EWS   = -99;
    const int ControlUnit::ADDRESS_IHKA  = 0x5b;
    const int ControlUnit::ADDRESS_KOMBI = 0x80;
    const int ControlUnit::ADDRESS_LSZ   = 0xd0;
    const int ControlUnit::ADDRESS_RADIO = 0x68;
    const int ControlUnit::ADDRESS_ZKE   = 0x00;

    ControlUnit::ControlUnit(const QString &aUuid, Manager *aParent) :
        QObject(aParent)
    {
        _manager = aParent;

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

        QSqlTableModel *modulesTable = _manager->modulesTable();

        QString parent_id = aUuid;
        // Add a "find root UUID" method to dbm
        while (!parent_id.isEmpty()) {
            QHash<QString, QVariant> theRecord = _manager->findModuleRecordByUuid(parent_id);
            if (theRecord.isEmpty()) {
                qDebug() << "Find parent failed: " << modulesTable->lastError();
                delete modulesTable;
                throw std::runtime_error("Find parent failed");
            }

            if (aUuid == parent_id) {
                _dppVersion = theRecord.value("dpp_version").toInt();
                _fileVersion = theRecord.value("file_version").toInt();
                _uuid = theRecord.value("uuid").toString();
                _address = theRecord.value("address").toChar().toLatin1();
                _family = theRecord.value("family").toString();
                _name = theRecord.value("name").toString();

                QStringList matchList = theRecord.value("matches").toString().split(";");
                foreach(const QString &match, matchList) {
                    if (match.isEmpty()) {
                        continue;
                    }
                    QStringList subMatches = match.split("=");
                    QString key(subMatches.at(0));
                    QString value(subMatches.at(1));
                    if (value.startsWith("s:")) {
                        _matches.insert(key, value.mid(2));
                    } else if (value.startsWith("i:")) {
                        _matches.insert(key, value.mid(2).toUInt());
                    } else {
                        QVariant ourMatchesVariant = theRecord.value("matches");
                        delete modulesTable;
                        QString errorString = QString("Invalid match string. String was: %1 / %2").arg(ourMatchesVariant.typeName()).arg(ourMatchesVariant.toString());
                        throw std::invalid_argument(qPrintable(errorString));
                    }
                }
            }

            if (getenv("DPP_TRACE")) {
                qDebug() << "Module: " << parent_id << " (from: " << aUuid << ")";
            }

            QSharedPointer<QSqlTableModel> operationsTable(_manager->operationsTable());
            operationsTable->setFilter(QString("module_id = '%1'").arg(parent_id));
            operationsTable->select();
            for (int i=0; i < operationsTable->rowCount(); i++) {
                QSqlRecord opRecord = operationsTable->record(i);

                QString opName = opRecord.value(opRecord.indexOf("name")).toString();
                QString opUuid = opRecord.value(opRecord.indexOf("uuid")).toString();
                QString opModule = opRecord.value(opRecord.indexOf("module_id")).toString();
                QString opCommand = opRecord.value(opRecord.indexOf("command")).toString();

                if (_operations.contains(opName)) {
                    if (getenv("DPP_TRACE")) {
                        qDebug() << "Skipping operation: " << opName << " we've a higher priority implementation";
                    }
                } else {
                    OperationPtr op(new Operation(opUuid, _address, opName, opCommand));
                    QSharedPointer<QSqlTableModel> results(_manager->resultsTable());
                    results->setFilter(QString("operation_id = '%1'").arg(opUuid));
                    results->select();

                    for (int j=0; j < results->rowCount(); j++) {
                        QSqlRecord resultRecord = results->record(j);
                        Result result;
                        result.setName(resultRecord.value(resultRecord.indexOf("name")).toString());
                        result.setUuid(resultRecord.value(resultRecord.indexOf("uuid")).toString());
                        result.setType(resultRecord.value(resultRecord.indexOf("type")).toString());
                        result.setDisplayFormat(resultRecord.value(resultRecord.indexOf("display")).toString());
                        result.setStartPosition(resultRecord.value(resultRecord.indexOf("start_pos")).toInt());
                        result.setLength(resultRecord.value(resultRecord.indexOf("length")).toInt());
                        result.setMask(resultRecord.value(resultRecord.indexOf("mask")).toString());
                        result.setFactorA(resultRecord.value(resultRecord.indexOf("factor_a")).toDouble());
                        result.setFactorB(resultRecord.value(resultRecord.indexOf("factor_b")).toDouble());


                        QJsonParseError jsonError;
                        QHash<QString, QString> ourLevels;
                        QByteArray jsonByteArray(qPrintable(resultRecord.value(resultRecord.indexOf("levels")).toString()));
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

                        //qDebug() << "\t\tAdding result: " << result.name;
                        op->insertResult(result.name(), result);
                    }

                    _operations.insert(opName, op);
                }
            }
            parent_id = theRecord.value("parent_id").toString();
        }

        delete modulesTable;
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
                ourNumber = qFromLittleEndian(ourNumber);

                if (result.factorA() != 0.0) {
                    ourNumber *= result.factorA();
                }

                ret.insert(result.name(), QVariant(ourNumber + result.factorB()));
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

    const QString ControlUnit::uuid() const
    {
        return _uuid;
    }

    quint8 ControlUnit::address() const
    {
        return _address;
    }

    const QString ControlUnit::name() const
    {
        return _name;
    }

    QHash<QString, OperationPtr > ControlUnit::operations() const
    {
        return _operations;
    }

    const QHash<QString, QVariant> ControlUnit::matchCriteria() const
    {
        return _matches;
    }

    QVariant ControlUnit::resultByteToVariant(const DS2PacketPtr aPacket, const Result &aResult)
    {
        QChar zeroPadding = QChar('0');

        if (aResult.length() != 1) {
            QString ourError = QObject::tr("Length is not one for a byte data type.  Length is %1").arg(aResult.length());
            throw std::invalid_argument(qPrintable(ourError));
        }

        unsigned char byte = aPacket->data().at(aResult.startPosition());
        if (aResult.displayFormat() == "hex_string") {
            QString hex;
            hex = QString("0x%1").arg(QString::number(byte, 16), 2, zeroPadding);
            return QVariant(hex);
        } else if (aResult.displayFormat() == "hex_int") {
            QString hex;
            hex = QString("%1").arg(QString::number(byte, 16), 2, zeroPadding);
            return QVariant(hex.toUInt());
        } else if (aResult.displayFormat() == "raw") {
            return QVariant(byte);
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
            double value = (byte * aResult.factorA()) + aResult.factorB();
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
