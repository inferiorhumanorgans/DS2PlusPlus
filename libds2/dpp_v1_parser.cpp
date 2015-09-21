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

#include <ds2/dpp_v1_parser.h>

#include <stdexcept>

#include <json/json.h>

#include <QDebug>
#include <QUuid>
#include <QDateTime>

#include <QSqlError>
#include <QSqlRecord>
#include <QSqlQuery>

#include <ds2/manager.h>

QString getQStringFromJson(Json::Value &aJsonValue)
{
    if (aJsonValue.isString()) {
        return aJsonValue.asCString();
    } else if (aJsonValue.isArray()) {
        return "[ARRAY]";
    } else {
        return QString::null;
    }
}

namespace DS2PlusPlus {
    DPP_V1_Parser::DPP_V1_Parser(Manager *parent) :
        QObject(parent), _manager(parent), qOut(stdout), qErr(stderr)
    {
        reset();
    }

    void DPP_V1_Parser::reset()
    {
        _knownUuids.clear();
    }

    void DPP_V1_Parser::parseFile(const QString &aLabel, QIODevice *anInput)
    {
        Json::Value jsRoot;
        Json::Reader jsReader;
        bool jsonParseSuccess = jsReader.parse(anInput->readAll().constData(), jsRoot, false);

        qErr << "Parsing: " << aLabel;

        if (!jsonParseSuccess) {
            qErr << endl << "\tError parsing " << aLabel << ": (jsoncpp)" << endl;
            qErr << jsReader.getFormattedErrorMessages().c_str() << endl << endl;
            return;
        }

        QString fileType = jsRoot["file_type"].asCString();
        QString uuid = jsRoot["uuid"].asCString();

        if (_knownUuids.contains(uuid)) {
            QString errorString = QString("UUID collision w/ this file @ %1").arg(uuid);
            throw std::runtime_error(qPrintable(errorString));
        } else {
            _knownUuids.insert(uuid);
        }

        qErr << " (" << uuid << ")" << endl;
        if (fileType == "string_table") {
            parseStringTableFile(jsRoot);
        } else if (fileType == "ecu") {
            parseEcuFile(jsRoot);
        }
    }

    QString DPP_V1_Parser::rawUuidToString(const QByteArray &aRawUuid)
    {
        QUuid ourUuid = QUuid::fromRfc4122(aRawUuid);
        if (ourUuid.isNull()) {
            return QString::null;
        }

        return ourUuid.toString().mid(1,36).toUpper();
    }

    QString DPP_V1_Parser::stringToUuidSQL(const QString &aRawUuid)
    {
        QUuid ourUuid = QUuid(aRawUuid);
        if (ourUuid.isNull()) {
            return "NULL";
        }

        return QString("X'%1'").arg(ourUuid.toRfc4122().toHex().data());
    }

    QVariant DPP_V1_Parser::stringToUuidVariant(const QString &aUuid)
    {
        QUuid ourUuid(aUuid);
        return ourUuid.isNull() ? QVariant(QString::null) : QVariant(ourUuid.toRfc4122());
    }

    void DPP_V1_Parser::parseEcuFile(const Json::Value &aJsonObject)
    {
        Json::Value moduleJson = aJsonObject;

        QString uuid(moduleJson["uuid"].asCString());
        QString name(moduleJson["name"].asCString());
        QString parent_id(getQStringFromJson(moduleJson["parent_id"]));

        QHash<QString, QVariant> oldModule = _manager->findModuleRecordByUuid(uuid);
        if (!oldModule.isEmpty()) {
            if (getenv("DPP_TRACE")) {
                qDebug() << "\tModule exists, overwriting " << uuid;
            }
            _manager->removeModuleByUuid(uuid);
        }

        QSqlQuery transaction(_manager->sqlDatabase());
        transaction.exec("BEGIN");
        QSqlQuery insertModuleQuery(_manager->sqlDatabase());
        insertModuleQuery.prepare("INSERT INTO modules(uuid, parent_id, file_version, dpp_version, name, protocol, family, address, part_number, hardware_num, software_num, coding_index, big_endian, mtime) VALUES (:uuid, :parent_id, :file_version, :dpp_version, :name, :protocol, :family, :address, :part_number, :hardware_num, :software_num, :coding_index, :big_endian, :mtime)");

        insertModuleQuery.bindValue(":uuid", stringToUuidVariant(uuid));
        insertModuleQuery.bindValue(":parent_id", stringToUuidVariant(parent_id));
        insertModuleQuery.bindValue(":file_version", moduleJson["file_version"].asInt());
        insertModuleQuery.bindValue(":dpp_version", moduleJson["dpp_version"].asInt());
        insertModuleQuery.bindValue(":name", name);
        insertModuleQuery.bindValue(":protocol", getQStringFromJson(moduleJson["protocol"]));
        insertModuleQuery.bindValue(":family", getQStringFromJson(moduleJson["family"]));

        QString ecuAddressString = getQStringFromJson(moduleJson["address"]);
        if (!ecuAddressString.isEmpty()) {
            bool ok;
            quint8 ecuAddressNumber = ecuAddressString.toInt(&ok, 16);
            if (ok) {
                insertModuleQuery.bindValue(":address", ecuAddressNumber);
            } else {
                QString errorString = QString("Error reading module JSON, invalid address %1").arg(ecuAddressString);
                throw std::runtime_error(qPrintable(errorString));
            }
        } else {
            insertModuleQuery.bindValue(":address", QVariant(QString::null));
        }

        qErr << "\tFound module definition: " << name;
        if (!ecuAddressString.isEmpty()) {
            qErr << " at " << ecuAddressString;
        }
        qErr << endl;
        if (!parent_id.isEmpty()) {
            qErr << "\tInherits: " << parent_id << endl;
        }

        bool ok;

        if (!moduleJson["part_number"].isNull()) {
            QStringList partNumbers;

            for (Json::ArrayIndex i=0; i < moduleJson["part_number"].size(); i++) {
                partNumbers << QString::number(moduleJson["part_number"][i].asUInt64(), 10);
            }

            insertModuleQuery.bindValue(":part_number", partNumbers.join("/"));
        } else {
            insertModuleQuery.bindValue(":part_number", QVariant(QString::null));
        }


        insertModuleQuery.bindValue(":hardware_num", QVariant(QString::null));
        if (!moduleJson["hardware_number"].isNull()) {
            quint64 hardware_number = QString(moduleJson["hardware_number"].asCString()).toULongLong(&ok, 16);
            if (ok) {
                insertModuleQuery.bindValue(":hardware_num", hardware_number);
            }
        }

        insertModuleQuery.bindValue(":software_num", QVariant(QString::null));
        if (!moduleJson["software_number"].isNull()) {
            quint64 software_number = QString(moduleJson["software_number"].asCString()).toULongLong(&ok, 16);
            if (ok) {
                insertModuleQuery.bindValue(":software_num", software_number);
            }
        }

        insertModuleQuery.bindValue(":coding_index", QVariant(QString::null));
        if (!moduleJson["coding_index"].isNull()) {
            quint64 coding_index = QString(moduleJson["coding_index"].asCString()).toULongLong(&ok, 16);
            if (ok) {
                insertModuleQuery.bindValue(":coding_index", coding_index);
            }
        }

        if (moduleJson["endian"].isNull()) {
            insertModuleQuery.bindValue(":big_endian", QVariant(QString::null));
        } else {
            QString endianness = moduleJson["endian"].asCString();
            insertModuleQuery.bindValue(":big_endian", (endianness == "big") ? 1 : 0);
        }

        if (!moduleJson["file_mtime"].isNull()) {
            QString mtimeString = moduleJson["file_mtime"].asCString();
            QDateTime mtime = QDateTime::fromString(mtimeString, Qt::ISODate);
            insertModuleQuery.bindValue(":mtime", mtime.toTime_t());
        } else {
            insertModuleQuery.bindValue(":mtime", QVariant(QString::null));
        }

        quint64 operationsCount = 0;
        Json::Value ourOperations = moduleJson["operations"];
        Json::ValueIterator operationIterator = ourOperations.begin();

        while (operationIterator != ourOperations.end()) {
            if (!parseOperationJson(operationIterator, moduleJson)) {
                throw std::invalid_argument("Error parsing the operation");
            }
            operationIterator++;
            operationsCount++;
        }
        qErr << "\tTotal: " << operationsCount << " operations" << endl;

        if (!insertModuleQuery.exec()) {
            QString errorString = QString("Saving the module %1 failed: %2").arg(uuid).arg(insertModuleQuery.lastError().databaseText());
            throw std::runtime_error(qPrintable(errorString));
        } else {
            transaction.exec("COMMIT");
        }
        qErr << endl;
    }

    void DPP_V1_Parser::parseStringTableFile(const Json::Value &aJsonObject)
    {
        Json::Value stringJson = aJsonObject;

        const QString uuid(getQStringFromJson(stringJson["uuid"]));
        const QString tableName(getQStringFromJson(stringJson["table_name"]));

        qErr << "\tFound table: '" << tableName << "'" << endl;

        QSqlQuery transaction(_manager->sqlDatabase());
        transaction.exec("BEGIN");

        _manager->removeStringTableByUuid(uuid);

        QSqlQuery stringTableQuery(_manager->sqlDatabase());
        stringTableQuery.prepare("INSERT INTO string_tables (name, uuid) VALUES (:name, :uuid)");

        stringTableQuery.bindValue(":name", tableName);
        stringTableQuery.bindValue(":uuid", stringToUuidVariant(uuid));

        if (!stringTableQuery.exec()) {
            qDebug() << "insertRecord failed: " << stringTableQuery.lastError();
            return;
        }

        Json::Value ourStrings = stringJson["strings"];
        Json::ValueIterator stringIterator = ourStrings.begin();

        quint64 stringCount = 0;
        while (stringIterator != ourStrings.end()) {
            Json::Value ourString = *stringIterator;
            QSqlQuery stringTableValueQuery(_manager->sqlDatabase());
            stringTableValueQuery.prepare("INSERT INTO string_values (table_uuid, number, string) VALUES (:table_uuid, :number, :string)");

            bool ok;
            Json::Value ourKey = stringIterator.key();
            quint8 stringNumber = QString(getQStringFromJson(ourKey)).toUInt(&ok, 16);

            stringTableValueQuery.bindValue(":table_uuid", stringToUuidVariant(uuid));
            stringTableValueQuery.bindValue(":number", stringNumber);
            stringTableValueQuery.bindValue(":string", getQStringFromJson(ourString));

            if (!stringTableValueQuery.exec()) {
                const QString exceptionString = QString("Saving the string value at UUID %1 failed: %2").arg(uuid).arg(stringTableValueQuery.lastError().databaseText());
                transaction.exec("ROLLBACK");
                throw std::runtime_error(qPrintable(exceptionString));
            }

            stringIterator++;
            stringCount++;
        }
        transaction.exec("COMMIT");
        qErr << "\tTotal: " << stringCount << ((stringCount == 1) ? " string" : " strings") << endl << endl;
    }

    bool DPP_V1_Parser::parseOperationJson(Json::ValueIterator &operationIt, Json::Value &moduleJSON)
    {
        Json::Value ourOperation = *operationIt;

        QSqlQuery operationQuery(_manager->sqlDatabase());
        operationQuery.prepare("INSERT INTO operations(uuid, module_id, name, parent_id, command) VALUES(:uuid, :module_id, :name, :parent_id, :command)");

        const QString uuid(getQStringFromJson(ourOperation["uuid"]));
        QString module_id(getQStringFromJson(moduleJSON["uuid"]));

        QString parent_id = getQStringFromJson(ourOperation["parent_id"]);

        if (_knownUuids.contains(uuid)) {
            Json::Value ourKey = operationIt.key();
            QString errorString = QString("UUID collision w/ operation %1, your data is corrupt.").arg(getQStringFromJson(ourKey));
            throw std::invalid_argument(qPrintable(errorString));
        } else {
            _knownUuids.insert(uuid);
        }

        QHash<QString, QVariant> oldOperation = _manager->findOperationByUuid(uuid);
        if (!oldOperation.isEmpty()) {
            if (getenv("DPP_TRACE")) {
                qDebug() << "Operation exists, overwriting " << uuid;
            }
            _manager->removeOperationByUuid(uuid);
        }

        operationQuery.bindValue(":uuid", stringToUuidVariant(uuid));
        operationQuery.bindValue(":module_id", stringToUuidVariant(module_id));
        operationQuery.bindValue(":name", operationIt.key().asCString());
        operationQuery.bindValue(":parent_id", stringToUuidVariant(parent_id));

        QByteArray commandByteList;
        for (Json::ArrayIndex i=0; i < ourOperation["command"].size(); i++) {
            QString byteString(getQStringFromJson(ourOperation["command"][i]));
            bool ok;
            quint32 byte = byteString.toUShort(&ok, 16);
            if (!ok or (byte > UCHAR_MAX)) {
                throw std::invalid_argument("Invalid command byte");
            }
            commandByteList.append(static_cast<quint8>(byte));
        }

        operationQuery.bindValue(":command", QVariant(commandByteList));

        if (!operationQuery.exec()) {
            qDebug() << "insertOpsRecord failed: " << operationQuery.lastError() << endl;
            return false;
        }

        quint64 resultsCount = 0;
        Json::Value ourResults = ourOperation["results"];
        Json::ValueIterator resultIt = ourResults.begin();
        while (resultIt != ourResults.end()) {
            if (!parseResultJson(resultIt, ourOperation)) {
                throw std::invalid_argument("Error parsing result JSON");
            }
            resultIt++;
            resultsCount++;
        }

        qErr << "\tAdded operation '" << operationIt.key().asCString() << "'";
        if (!parent_id.isEmpty()) {
            qErr << ", inherits from: " << parent_id;
        }
        qErr << ", returns " << resultsCount << ((parent_id.isEmpty()) ? "" : " additional") << ((resultsCount == 1) ? " result" : " results") << endl;
        return true;
    }

    bool DPP_V1_Parser::parseResultJson(Json::ValueIterator &aResultIterator, Json::Value &operationJSON)
    {
        Json::Value ourResult = *aResultIterator;
        QSqlQuery resultQuery(_manager->sqlDatabase());
        resultQuery.prepare("INSERT INTO results (uuid, operation_id, parent_id, name, type, display, start_pos, mask, rpn, units, length, levels) VALUES (:uuid, :operation_id, :parent_id, :name, :type, :display, :start_pos, :mask, :rpn, :units, :length, :levels)");

        const QString uuid(ourResult["uuid"].asCString());

        if (_knownUuids.contains(uuid)) {
            QString errorString = QString("UUID collision w/ result %1, named: %2, your data is corrupt.").arg(uuid).arg(aResultIterator.key().asCString());
            throw std::invalid_argument(qPrintable(errorString));
        } else {
            _knownUuids.insert(uuid);
        }


        QHash<QString, QVariant> oldResult = _manager->findResultByUuid(uuid);
        if (!oldResult.isEmpty()) {
            if (getenv("DPP_TRACE")) {
                qDebug() << "Result exists, overwriting " << uuid;
            }
            _manager->removeResultByUuid(uuid);
        }

        resultQuery.bindValue(":uuid",         stringToUuidVariant(uuid));
        resultQuery.bindValue(":operation_id", stringToUuidVariant(getQStringFromJson(operationJSON["uuid"])));
        resultQuery.bindValue(":parent_id",    stringToUuidVariant(getQStringFromJson(ourResult["parent_id"])));
        resultQuery.bindValue(":name",         aResultIterator.key().asCString());
        resultQuery.bindValue(":type",         getQStringFromJson(ourResult["type"]));
        resultQuery.bindValue(":display",      getQStringFromJson(ourResult["display"]));
        resultQuery.bindValue(":start_pos",    ourResult["start_pos"].asInt());
        resultQuery.bindValue(":mask",         getQStringFromJson(ourResult["mask"]));
        resultQuery.bindValue(":rpn",          getQStringFromJson(ourResult["rpn"]));
        resultQuery.bindValue(":units",        getQStringFromJson(ourResult["units"]));

        if (!ourResult["length"].isNull()) {
            resultQuery.bindValue(":length",   ourResult["length"].asInt());
        } else {
            resultQuery.bindValue(":length",   QVariant(QString::null));
        }

        Json::FastWriter writer;
        std::string levelsString = writer.write(ourResult["levels"]);
        QString levelsQString = QString(levelsString.c_str()).trimmed();
        if (levelsQString == "null") {
            levelsQString = QString::null;
        }
        resultQuery.bindValue("levels",       levelsQString);

        if (!resultQuery.exec()) {
            QString errorString = QString("submitResultRecords: %1 (uuid=%2)").arg(resultQuery.lastError().databaseText()).arg(uuid);
            qErr << errorString << endl;
            return false;
        }

        return true;
    }
}
