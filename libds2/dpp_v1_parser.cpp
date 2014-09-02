#include "dpp_v1_parser.h"

#include <stdexcept>

#include <json/json.h>

#include <QDebug>

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
        moduleRecord.setValue(moduleRecord.indexOf("uuid"), uuid);
        moduleRecord.setValue(moduleRecord.indexOf("parent_id"), parent_id);
        moduleRecord.setValue(moduleRecord.indexOf("file_version"), moduleJson["file_version"].asInt());
        moduleRecord.setValue(moduleRecord.indexOf("dpp_version"), moduleJson["dpp_version"].asInt());
        moduleRecord.setValue(moduleRecord.indexOf("name"), name);
        moduleRecord.setValue(moduleRecord.indexOf("family"), getQStringFromJson(moduleJson["family"]));

        QString ecuAddressString = getQStringFromJson(moduleJson["address"]);
        if (!ecuAddressString.isEmpty()) {
            bool ok;
            quint8 ecuAddressNumber = ecuAddressString.toInt(&ok, 16);
            if (ok) {
                moduleRecord.setValue(moduleRecord.indexOf("address"), ecuAddressNumber);
            } else {
                QString errorString = QString("Error reading module JSON, invalid address %1").arg(ecuAddressString);
                throw std::runtime_error(qPrintable(errorString));
            }
        } else {
            moduleRecord.setValue(moduleRecord.indexOf("address"), QVariant(QString::null));
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
            moduleRecord.setValue(moduleRecord.indexOf("part_number"), part_number);
        }

        if (!moduleJson["hardware_number"].isNull()) {
            quint64 hardware_number = QString(moduleJson["hardware_number"].asCString()).toULongLong(&ok, 16);
            if (ok) {
                moduleRecord.setValue(moduleRecord.indexOf("hardware_num"), hardware_number);
            }
        }

        if (!moduleJson["software_number"].isNull()) {
            quint64 software_number = QString(moduleJson["software_number"].asCString()).toULongLong(&ok, 16);
            if (ok) {
                moduleRecord.setValue(moduleRecord.indexOf("software_num"), software_number);
            }
        }

        if (!moduleJson["coding_index"].isNull()) {
            quint64 coding_index = QString(moduleJson["coding_index"].asCString()).toULongLong(&ok, 16);
            if (ok) {
                moduleRecord.setValue(moduleRecord.indexOf("coding_index"), coding_index);
            }
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
        QSharedPointer<QSqlTableModel> ourModel(_manager->stringTable());
        Json::Value stringJson = aJsonObject;

        QString uuid(getQStringFromJson(stringJson["uuid"]));
        QString tableName(getQStringFromJson(stringJson["table_name"]));

        qErr << "\tFound table: '" << tableName << "'" << endl;

        _manager->removeStringTableByUuid(uuid);

        Json::Value ourStrings = stringJson["strings"];
        Json::ValueIterator stringIterator = ourStrings.begin();

        quint64 stringCount = 0;
        while (stringIterator != ourStrings.end()) {
            Json::Value ourString = *stringIterator;
            QSqlRecord stringRecord = ourModel->record();
            bool ok;
            Json::Value ourKey = stringIterator.key();
            quint8 stringNumber = QString(getQStringFromJson(ourKey)).toUInt(&ok, 16);

            stringRecord.setValue(stringRecord.indexOf("table_uuid"), uuid);
            stringRecord.setValue(stringRecord.indexOf("table_name"), tableName);
            stringRecord.setValue(stringRecord.indexOf("number"), stringNumber);
            stringRecord.setValue(stringRecord.indexOf("string"), getQStringFromJson(ourString));

            if (!ourModel->insertRecord(-1, stringRecord)) {
                qDebug() << "insertRecord failed: " << ourModel->lastError();
            }

            if (!ourModel->submitAll()) {
                QString exceptionString = QString("Saving the string table at UUID %1 failed: %2").arg(uuid).arg(ourModel->lastError().databaseText());
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

        operationRecord.setValue(operationRecord.indexOf("uuid"), uuid);
        operationRecord.setValue(operationRecord.indexOf("module_id"), module_id);
        operationRecord.setValue(operationRecord.indexOf("name"), operationIt.key().asCString());
        operationRecord.setValue(operationRecord.indexOf("parent_id"), parent_id);

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

        operationRecord.setValue(operationRecord.indexOf("command"), QVariant(commandByteList));

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

        resultRecord.setValue(resultRecord.indexOf("uuid"),         uuid);
        resultRecord.setValue(resultRecord.indexOf("operation_id"), getQStringFromJson(operationJSON["uuid"]));
        resultRecord.setValue(resultRecord.indexOf("name"),         aResultIterator.key().asCString());
        resultRecord.setValue(resultRecord.indexOf("type"),         getQStringFromJson(ourResult["type"]));
        resultRecord.setValue(resultRecord.indexOf("display"),      getQStringFromJson(ourResult["display"]));
        resultRecord.setValue(resultRecord.indexOf("start_pos"),    ourResult["start_pos"].asInt());
        resultRecord.setValue(resultRecord.indexOf("length"),       ourResult["length"].asInt());
        resultRecord.setValue(resultRecord.indexOf("mask"),         getQStringFromJson(ourResult["mask"]));
        resultRecord.setValue(resultRecord.indexOf("factor_a"),     ourResult["factor_a"].asDouble());
        resultRecord.setValue(resultRecord.indexOf("factor_b"),     ourResult["factor_b"].asDouble());


        Json::FastWriter writer;
        std::string levelsString = writer.write(ourResult["levels"]);
        QString levelsQString = QString(levelsString.c_str()).trimmed();
        if (levelsQString == "null") {
            levelsQString = QString::null;
        }
        resultRecord.setValue(resultRecord.indexOf("levels"),       levelsQString);

        if (!aResultsModel->insertRecord(-1, resultRecord)) {
            qDebug() << "insertResultRecord failed: " << aResultsModel->lastError() << endl;
            return false;
        }
        if (!aResultsModel->submitAll()) {
            qDebug() << "submitResultRecords Failed: " << aResultsModel->lastError() << endl;
            return false;
        }

        return true;
    }
}
