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

#include "dpp_v1_parser.h"

#include <stdexcept>

#include <json/json.h>

#include <QDebug>
#include <QUuid>
#include <QDateTime>

#include <QSqlError>
#include <QSqlRecord>

#include "manager.h"

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
        QSharedPointer<QSqlTableModel> ourModel(_manager->modulesTable());
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

        QSqlRecord moduleRecord = ourModel->record();
        moduleRecord.setValue("uuid", stringToUuidVariant(uuid));
        moduleRecord.setValue("parent_id", stringToUuidVariant(parent_id));
        moduleRecord.setValue("file_version", moduleJson["file_version"].asInt());
        moduleRecord.setValue("dpp_version", moduleJson["dpp_version"].asInt());
        moduleRecord.setValue("name", name);
        moduleRecord.setValue("family", getQStringFromJson(moduleJson["family"]));

        QString ecuAddressString = getQStringFromJson(moduleJson["address"]);
        if (!ecuAddressString.isEmpty()) {
            bool ok;
            quint8 ecuAddressNumber = ecuAddressString.toInt(&ok, 16);
            if (ok) {
                moduleRecord.setValue("address", ecuAddressNumber);
            } else {
                QString errorString = QString("Error reading module JSON, invalid address %1").arg(ecuAddressString);
                throw std::runtime_error(qPrintable(errorString));
            }
        } else {
            moduleRecord.setValue("address", QVariant(QString::null));
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
            quint64 part_number = moduleJson["part_number"].asUInt64();
            moduleRecord.setValue("part_number", part_number);
        }

        if (!moduleJson["hardware_number"].isNull()) {
            quint64 hardware_number = QString(moduleJson["hardware_number"].asCString()).toULongLong(&ok, 16);
            if (ok) {
                moduleRecord.setValue("hardware_num", hardware_number);
            }
        }

        if (!moduleJson["software_number"].isNull()) {
            quint64 software_number = QString(moduleJson["software_number"].asCString()).toULongLong(&ok, 16);
            if (ok) {
                moduleRecord.setValue("software_num", software_number);
            }
        }

        if (!moduleJson["coding_index"].isNull()) {
            quint64 coding_index = QString(moduleJson["coding_index"].asCString()).toULongLong(&ok, 16);
            if (ok) {
                moduleRecord.setValue("coding_index", coding_index);
            }
        }

        if (moduleJson["endian"].isNull()) {
            moduleRecord.setValue("big_endian", QVariant(QString::null));
        } else {
            QString endianness = moduleJson["endian"].asCString();
            moduleRecord.setValue("big_endian", (endianness == "big") ? 1 : 0);
        }

        if (!moduleJson["file_mtime"].isNull()) {
            QString mtimeString = moduleJson["file_mtime"].asCString();
            QDateTime mtime = QDateTime::fromString(mtimeString, Qt::ISODate);
            moduleRecord.setValue("mtime", mtime.toTime_t());
        }

        quint64 operationsCount = 0;
        Json::Value ourOperations = moduleJson["operations"];
        Json::ValueIterator operationIterator = ourOperations.begin();
        QSharedPointer<QSqlTableModel> ourOperationsTable(_manager->operationsTable());
        while (operationIterator != ourOperations.end()) {
            if (!parseOperationJson(operationIterator, moduleJson, ourOperationsTable)) {
                throw std::invalid_argument("Error parsing the operation");
            }
            operationIterator++;
            operationsCount++;
        }
        qErr << "\tTotal: " << operationsCount << " operations" << endl;

        if (!ourModel->insertRecord(-1, moduleRecord)) {
            qDebug() << "insertRecord failed: " << ourModel->lastError();
        }

        if (!ourModel->submitAll()) {
            QString errorString = QString("Saving the module %1 failed: %2").arg(uuid).arg(ourModel->lastError().databaseText());
            throw std::runtime_error(qPrintable(errorString));
        }
        qErr << endl;
    }

    void DPP_V1_Parser::parseStringTableFile(const Json::Value &aJsonObject)
    {
        QSharedPointer<QSqlTableModel> ourValueModel(_manager->stringValuesTable());
        QSharedPointer<QSqlTableModel> ourTableModel(_manager->stringTablesTable());

        Json::Value stringJson = aJsonObject;

        QString uuid(getQStringFromJson(stringJson["uuid"]));
        QString tableName(getQStringFromJson(stringJson["table_name"]));

        qErr << "\tFound table: '" << tableName << "'" << endl;

        _manager->removeStringTableByUuid(uuid);

        QSqlRecord stringTableRecord = ourTableModel->record();
        stringTableRecord.setValue("name", tableName);
        stringTableRecord.setValue("uuid", stringToUuidVariant(uuid));
        if (!ourTableModel->insertRecord(-1, stringTableRecord)) {
            qDebug() << "insertRecord failed: " << ourTableModel->lastError();
        }

        if (!ourTableModel->submitAll()) {
            QString exceptionString = QString("Saving the string_table at UUID %1 failed: %2").arg(uuid).arg(ourValueModel->lastError().databaseText());
            throw std::runtime_error(qPrintable(exceptionString));
        }

        Json::Value ourStrings = stringJson["strings"];
        Json::ValueIterator stringIterator = ourStrings.begin();

        quint64 stringCount = 0;
        while (stringIterator != ourStrings.end()) {
            Json::Value ourString = *stringIterator;
            QSqlRecord stringValueRecord = ourValueModel->record();
            bool ok;
            Json::Value ourKey = stringIterator.key();
            quint8 stringNumber = QString(getQStringFromJson(ourKey)).toUInt(&ok, 16);

            stringValueRecord.setValue("table_uuid", stringToUuidVariant(uuid));
            stringValueRecord.setValue("number", stringNumber);
            stringValueRecord.setValue("string", getQStringFromJson(ourString));

            if (!ourValueModel->insertRecord(-1, stringValueRecord)) {
                qDebug() << "insertRecord failed: " << ourValueModel->lastError();
            }

            if (!ourValueModel->submitAll()) {
                QString exceptionString = QString("Saving the string value at UUID %1 failed: %2").arg(uuid).arg(ourValueModel->lastError().databaseText());
                throw std::runtime_error(qPrintable(exceptionString));
            }

            stringIterator++;
            stringCount++;
        }
        qErr << "\tTotal: " << stringCount << ((stringCount == 1) ? " string" : " strings") << endl << endl;
    }

    bool DPP_V1_Parser::parseOperationJson(Json::ValueIterator &operationIt, Json::Value &moduleJSON, QSharedPointer<QSqlTableModel> &operationsModel)
    {
        Json::Value ourOperation = *operationIt;
        QSqlRecord operationRecord = operationsModel->record();

        QString uuid(getQStringFromJson(ourOperation["uuid"]));
        QString module_id(getQStringFromJson(moduleJSON["uuid"]));

        QString parent_id = getQStringFromJson(ourOperation["parent"]);

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

        operationRecord.setValue("uuid", stringToUuidVariant(uuid));
        operationRecord.setValue("module_id", stringToUuidVariant(module_id));
        operationRecord.setValue("name", operationIt.key().asCString());
        operationRecord.setValue("parent_id", stringToUuidVariant(parent_id));

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

        operationRecord.setValue("command", QVariant(commandByteList));

        QSharedPointer<QSqlTableModel> ourModel(_manager->operationsTable());
        if (!ourModel->insertRecord(-1, operationRecord)) {
            qDebug() << "insertOpsRecord failed: " << ourModel->lastError() << endl;
            return false;
        }

        if (!ourModel->submitAll()) {
            qDebug() << "submitOpsRecords Failed: " << ourModel->lastError() << endl;
            return false;
        }

        quint64 resultsCount = 0;
        Json::Value ourResults = ourOperation["results"];
        Json::ValueIterator resultIt = ourResults.begin();
        QSharedPointer<QSqlTableModel> ourResultsModel(_manager->resultsTable());
        while (resultIt != ourResults.end()) {
            if (!parseResultJson(resultIt, ourOperation, ourResultsModel)) {
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

    bool DPP_V1_Parser::parseResultJson(Json::ValueIterator &aResultIterator, Json::Value &operationJSON, QSharedPointer<QSqlTableModel> &aResultsModel)
    {
        Json::Value ourResult = *aResultIterator;
        QSqlRecord resultRecord = aResultsModel->record();

        QString uuid(ourResult["uuid"].asCString());

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

        resultRecord.setValue("uuid",         stringToUuidVariant(uuid));
        resultRecord.setValue("operation_id", stringToUuidVariant(getQStringFromJson(operationJSON["uuid"])));
        resultRecord.setValue("name",         aResultIterator.key().asCString());
        resultRecord.setValue("type",         getQStringFromJson(ourResult["type"]));
        resultRecord.setValue("display",      getQStringFromJson(ourResult["display"]));
        resultRecord.setValue("start_pos",    ourResult["start_pos"].asInt());
        resultRecord.setValue("length",       ourResult["length"].asInt());
        resultRecord.setValue("mask",         getQStringFromJson(ourResult["mask"]));
        resultRecord.setValue("rpn",          getQStringFromJson(ourResult["rpn"]));
        resultRecord.setValue("units",        getQStringFromJson(ourResult["units"]));

        Json::FastWriter writer;
        std::string levelsString = writer.write(ourResult["levels"]);
        QString levelsQString = QString(levelsString.c_str()).trimmed();
        if (levelsQString == "null") {
            levelsQString = QString::null;
        }
        resultRecord.setValue("levels",       levelsQString);

        if (!aResultsModel->insertRecord(-1, resultRecord)) {
            qErr  << "insertResultRecord: " << aResultsModel->lastError().databaseText() << endl;
            return false;
        }

        if (!aResultsModel->submitAll()) {
            QString errorString = QString("submitResultRecords: %1 (uuid=%2)").arg(aResultsModel->lastError().databaseText()).arg(uuid);
            qErr << errorString << endl;
            return false;
        }

        return true;
    }
}
