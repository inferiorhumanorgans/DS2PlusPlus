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

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include <iostream>
#include <stdexcept>

#include <QTextStream>
#include <QDebug>
#include <QSharedPointer>
#include <QCommandLineParser>
#include <QUuid>

#include <QPluginLoader>
#include <QSqlDriverPlugin>

#include <QDir>
#include <QDirIterator>

#include <QSqlQuery>
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QSqlError>

#include "manager.h"

#include "dpp_v1_parser.h"

QByteArray readBytes(int fd, int length)
{
    QByteArray ret;
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fd, &fds);

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 500000;

    int remainingTimeouts = 5;

    while (ret.length() < length) {
        switch(select(fd+1, &fds, NULL, NULL, &tv)) {
        case 0:
            // timeout
            if (--remainingTimeouts == 0) {
                qDebug() << "Timed out while waiting for packet";
                return ret;
            }
            break;
        case -1:
            // error
            qDebug() << "Error from select: " << strerror(errno);
            return ret;
        case 1:
        default:
            unsigned char byte = 0;
            int bytesRead = read(fd, &byte, 1);
            if (bytesRead != 1) {
                qDebug() << "Didn't read the byte we were expecting.  Error: " << strerror(errno);
                return ret;
            }
            ret.append(byte);
            break;
        }

    }
    return ret;
}

bool fd_is_valid(int fd)
{
    return fcntl(fd, F_GETFD) != -1 || errno != EBADF;
}

namespace DS2PlusPlus {
    const QString Manager::DPP_DIR = QString("~/.dpp");
    const QString Manager::DPP_DB_PATH = QString("dppdb.sqlite3");
    const QString Manager::DPP_JSON_PATH = QString("json");

    QTextStream qOut(stdout);
    QTextStream qErr(stderr);

    QString expandTilde(const QString &path)
    {
        QString ret(path);
        if (ret.startsWith ("~/")) {
            ret.replace (0, 1, QDir::homePath());
        }

        return ret;
    }

    Manager::Manager(QSharedPointer<QCommandLineParser> aParser, int fd, QObject *parent) :
        QObject(parent), _dppDir(QString::null), _fd(fd), _cliParser(aParser)
    {
        if (!_cliParser.isNull()) {
            // We should really check to see if the user has already defined this, and if so we should whine.
            QCommandLineOption targetDirectoryOption(QStringList() << "f" << "port",
                      "Read from serial port <port>.",
                      "port");
            aParser->addOption(targetDirectoryOption);

            QCommandLineOption jsonDirOption("dpp-source-dir", "Specify location of DPP-JSON files", "dpp-source-dir");
            aParser->addOption(jsonDirOption);

            QCommandLineOption androidHack("dpp-dir", "Specify location of DPP database", "dpp-dir");
            aParser->addOption(androidHack);

            QCommandLineOption androidHack2("android-native", "Fix Android", "android-native");
            aParser->addOption(androidHack2);
        } else {
            initializeManager();
        }
    }

    void Manager::initializeManager()
    {
        // Check to make sure our directory
        // So we can have multiple connections...
        QString connName(QUuid::createUuid().toString());

        if (!_cliParser.isNull() and _cliParser->isSet("android-native")) {
            qDebug() << "Driver dir is: " << _cliParser->value("android-native");
            QPluginLoader plug(_cliParser->value("android-native") + "/libqsqlite.so");
            plug.load();

            QSqlDriverPlugin *sqlPlugin = qobject_cast<QSqlDriverPlugin *>(plug.instance());

            _db = QSqlDatabase::addDatabase(sqlPlugin->create("QSQLITE"), connName);
        } else {
            _db = QSqlDatabase::addDatabase("QSQLITE", connName);
        }

        if (!_cliParser.isNull() and _cliParser->isSet("dpp-dir")) {
            this->_dppDir = _cliParser->value("dpp-dir");
        }

        if (!_cliParser.isNull() and _cliParser->isSet("dpp-source-dir")) {
            this->_dppSourceDir = _cliParser->value("dpp-source-dir");
        }

        QString dppDbPath = QString("%1%2%3").arg(dppDir()).arg(QDir::separator()).arg(DPP_DB_PATH);
        _db.setDatabaseName(expandTilde(dppDbPath));

        if (!_db.open()) {
            QString errorString = QString("Couldn't open the database: %1").arg(_db.lastError().databaseText());
            throw std::runtime_error(qPrintable(errorString));
        }
    }

    void Manager::reloadDatabase()
    {
        if (_db.isOpen()) {
            _db.close();
        }
        _db.open();
    }

    void Manager::setFd(int aFd)
    {
        _fd = aFd;
    }

    int Manager::fd() const
    {
        return _fd;
    }

    QSqlTableModel *Manager::modulesTable() {
        if (!_db.isOpen()) {
            qDebug() << "DB NOT OPEN";
        }

        QSqlTableModel *ret = new QSqlTableModel(this, _db);
        ret->setTable("modules");
        ret->setEditStrategy(QSqlTableModel::OnManualSubmit);

        return ret;
    }

    QSqlTableModel *Manager::operationsTable() {
        if (!_db.isOpen()) {
            qDebug() << "DB NOT OPEN";
        }

        QSqlTableModel *ret = new QSqlTableModel(this, _db);
        ret->setTable("operations");
        ret->setEditStrategy(QSqlTableModel::OnManualSubmit);

        return ret;
    }

    QSqlTableModel *Manager::resultsTable() {
        if (!_db.isOpen()) {
            qDebug() << "DB NOT OPEN";
        }

        QSqlTableModel *ret = new QSqlTableModel(this, _db);
        ret->setTable("results");
        ret->setEditStrategy(QSqlTableModel::OnManualSubmit);

        return ret;
    }

    QSqlTableModel *Manager::stringValuesTable()
    {
        if (!_db.isOpen()) {
            qDebug() << "DB NOT OPEN";
        }

        QSqlTableModel *ret = new QSqlTableModel(this, _db);
        ret->setTable("string_values");
        ret->setEditStrategy(QSqlTableModel::OnManualSubmit);

        return ret;

    }

    QSqlTableModel *Manager::stringTablesTable()
    {
        if (!_db.isOpen()) {
            qDebug() << "DB NOT OPEN";
        }

        QSqlTableModel *ret = new QSqlTableModel(this, _db);
        ret->setTable("string_tables");
        ret->setEditStrategy(QSqlTableModel::OnManualSubmit);

        return ret;
    }

    Manager::~Manager() {
        if (_db.isOpen()) {
            _db.close();
        }
    }

    QString Manager::dppDir()
    {
        if (!_dppDir.isNull()) {
            return _dppDir;
        }

        const char *dppDirEnv = getenv("DPP_DIR");

        if (dppDirEnv) {
            _dppDir = QString(dppDirEnv);
        } else {
            // This is cheating
            if (QFile::exists(QDir::currentPath() + "/dppdb.sqlite3" )) {
                _dppDir = QDir::currentPath();
            } else {
                _dppDir = DPP_DIR;
            }
        }

        _dppDir = expandTilde(_dppDir);

        QDir ourDir(_dppDir);
        if (!ourDir.exists()) {
            if (!ourDir.mkpath(".")) {
                qErr << "Couldn't create DPP DIR" << endl;
            }
        }

        return _dppDir;
    }

    QString Manager::jsonDir()
    {
        QStringList jsonSearchPath;
        if (!_dppSourceDir.isEmpty()) {
            jsonSearchPath.append(expandTilde(_dppSourceDir));
        } else {
            jsonSearchPath.append(QString("%1%2%3").arg(QDir::currentPath()).arg(QDir::separator()).arg(DPP_JSON_PATH));
            jsonSearchPath.append(expandTilde(QString("~%1dpp%2%3").arg(QDir::separator()).arg(QDir::separator()).arg("json")));
            jsonSearchPath.append(QString("%1%2%3").arg(dppDir()).arg(QDir::separator()).arg(DPP_JSON_PATH));
            jsonSearchPath.append(expandTilde(QString("~%1ds2%2%3").arg(QDir::separator()).arg(QDir::separator()).arg("json")));
        }

        const char *jsonEnv = getenv("DPP_JSON_DIR");
        if (jsonEnv) {
            jsonSearchPath.prepend(expandTilde(QString(jsonEnv)));
        }

        foreach (const QString &path, jsonSearchPath) {
            QDir ourDir(path);
            if (ourDir.exists()) {
                return path;
            }
        }

        QString errorString = QString("Could not find a DPP JSON directory in: %1").arg(jsonSearchPath.join(", "));
        throw std::ios_base::failure(qPrintable(errorString));
    }

    DS2PacketPtr Manager::query(DS2PacketPtr aPacket)
    {
        if (!fd_is_valid(_fd)) {
            throw std::ios_base::failure("Serial port is not open.");
        }

        DS2PacketPtr ret(new DS2Packet);

        // Send query to the ECU
        QByteArray ourBA = static_cast<QByteArray>(*aPacket);

        int written = write(_fd, ourBA.data(), ourBA.size());
        if (written != ourBA.size()) {
            qDebug() << "Didn't write all " << written << " vs " << ourBA.size() << " Error: " << strerror(errno);
            return ret;
        }

        // Read the echo back.  We should check to see if it matches, maybe...
        usleep(80000);
        if (readBytes(_fd, ourBA.size()).size() != ourBA.size()) {
            throw std::ios_base::failure("Error reading the echo echo echo echo...");
        }

        // Read the initial header
        quint8 ecuAddress;
        quint8 length;

        QByteArray inputArray;
        inputArray = readBytes(_fd, 2);
        usleep(12500);

        if (inputArray.length() != 2) {
            QString errorString = QString("Wanted length 2, got: %1").arg(inputArray.length());
            throw std::ios_base::failure(qPrintable(errorString));
        }

        ecuAddress = inputArray.at(0);
        ret->setAddress(ecuAddress);
        length = inputArray.at(1);

        if (length < 4) {
            QString errorString = QString("Ack. Got garbage data, length must be >= 4.  Got ECU: %1, LEN: %2").arg(QString::number(ecuAddress, 16)).arg(QString::number(length, 16));
            throw std::ios_base::failure(qPrintable(errorString));
        }

        inputArray = readBytes(_fd, length - 2);

        if (inputArray.length() != (length - 2)) {
            qDebug() << "RUH ROH";
        }

        // Copy in all but the checksum.
        ret->setData(inputArray.mid(0, inputArray.length() - 1));

        // Verify the checksum
        unsigned char realChecksum = inputArray.at(inputArray.length() - 1);
#if 0
        qDebug() << QString("Checksum A: 0x%1").arg(ret->checksum(), 2, 16, QChar('0'));
        qDebug() << QString("Checksum B: 0x%1").arg(realChecksum, 2, 16, QChar('0'));
#endif

        if (ret->checksum() != realChecksum) {
            qDebug() << "Checksum mismatch...";
        }
        return ret;
    }

    ControlUnitPtr Manager::findModuleAtAddress(quint8 anAddress) {
        DS2PacketPtr ourSentPacket(new DS2Packet(anAddress, QByteArray((int)1, (unsigned char)0x00)));
        DS2PacketPtr ourReceivedPacket = query(ourSentPacket);
        qDebug() << "Our IDENT: " << *ourReceivedPacket;
        return findModuleByMatchingIdentPacket(ourReceivedPacket);
    }

    ControlUnitPtr Manager::findModuleByMatchingIdentPacket(const DS2PacketPtr aPacket) {
        ControlUnitPtr ret;
        QHash<QString, ControlUnitPtr> modules;
        QHash<QString, ControlUnitPtr>::Iterator moduleIt;
        quint8 fullMatch = ControlUnit::MatchNone;
        QString ourKey;

        modules = findAllModulesByAddress(aPacket->address());
        for (moduleIt = modules.begin(); moduleIt != modules.end(); ++moduleIt) {
            ControlUnitPtr ecu(moduleIt.value());

            if (ecu->partNumber() == 0) {
                continue;
            }

            DS2Response response(ecu->parseOperation("identify", aPacket));
            if (getenv("DPP_TRACE")) {
                qDebug() << "Checking " << ecu->name();
            }

            if (response.value("part_number").toULongLong() == ecu->partNumber()) {
                fullMatch |= ControlUnit::MatchAll;
                ret = ecu;
                ourKey = moduleIt.key();

                quint64 actualSW = response.value("software_number").toString().toULongLong(NULL, 16);
                if (actualSW != ecu->softwareNumber()) {
                    fullMatch &= ~ControlUnit::MatchAll;
                    fullMatch |= ControlUnit::MatchSWMismatch;
                    QString matchString = QString("SW version mismatch %1 != expected 0x%2")
                                            .arg(response.value("software_number").toString())
                                            .arg(ecu->softwareNumber(), 2, 16, QChar('0'));
                    //qDebug() << matchString;
                }

                quint64 actualHW = response.value("hardware_number").toString().toULongLong(NULL, 16);
                if (actualHW != ecu->hardwareNumber()) {
                    fullMatch &= ~ControlUnit::MatchAll;
                    fullMatch |= ControlUnit::MatchHWMismatch;
                    QString matchString = QString("HW version mismatch %1 != expected 0x%2")
                                            .arg(response.value("hardware_number").toString())
                                            .arg(ecu->hardwareNumber(), 2, 16, QChar('0'));
                    //qDebug() << matchString;
                }

                quint64 actualCI = response.value("coding_index").toString().toULongLong(NULL, 16);
                if (actualCI != ecu->codingIndex()) {
                    fullMatch &= ~ControlUnit::MatchAll;
                    fullMatch |= ControlUnit::MatchCIMismatch;
                    QString matchString = QString("CI mismatch %1 != expected 0x%2")
                                            .arg(response.value("coding_index").toString())
                                            .arg(ecu->codingIndex(), 2, 16, QChar('0'));
                    //qDebug() << matchString;
                }

                if (fullMatch & ControlUnit::MatchAll) {
                    break;
                }
            }
        }

        modules.remove(ourKey);
        modules.clear();

        if (!ret.isNull()) {
            ret->setMatchFlags(fullMatch);
        }

        return ret;
    }

    QHash<QString, QVariant> Manager::findModuleRecordByUuid(const QString &aUuid)
    {
        QHash<QString, QVariant> ret;
        QSqlRecord rec;
        QSqlTableModel *ourModel = modulesTable();

        ourModel->setFilter(QString("uuid = %1").arg(DPP_V1_Parser::stringToUuidSQL(aUuid)));
        ourModel->select();

        if (ourModel->rowCount() == 1) {
            rec = ourModel->record(0);
        }

        if (!rec.isEmpty()) {
            for (int i=0; i < ourModel->columnCount(); i++) {
                ret.insert(rec.fieldName(i), rec.value(i));
            }
        }

        delete ourModel;
        return ret;
    }

    QHash<QString, ControlUnitPtr> Manager::findAllModulesByFamily(const QString &aFamily)
    {
        QHash<QString, ControlUnitPtr > ret;
        QSqlTableModel *ourModel = modulesTable();

        ourModel->setFilter(QString("family = '%1'").arg(aFamily));
        ourModel->select();

        for (int i=0; i < ourModel->rowCount(); i++) {
            QSqlRecord theRecord = ourModel->record(i);
            QString uuid(DPP_V1_Parser::rawUuidToString(theRecord.value("uuid").toByteArray()));
            ControlUnitPtr ecu(new ControlUnit(uuid, this));
            ret.insert(uuid, ecu);
        }

        delete ourModel;
        return ret;
    }

    QHash<QString, ControlUnitPtr> Manager::findAllModulesByAddress(quint8 anAddress)
    {
        QHash<QString, ControlUnitPtr > ret;
        QSqlTableModel *ourModel = modulesTable();

        ourModel->setFilter(QString("address = %1").arg(static_cast<quint64>(anAddress)));
        ourModel->select();

        for (int i=0; i < ourModel->rowCount(); i++) {
            QSqlRecord theRecord = ourModel->record(i);
            QString uuid(DPP_V1_Parser::rawUuidToString(theRecord.value("uuid").toByteArray()));
            ControlUnitPtr ecu(new ControlUnit(uuid, this));
            ret.insert(uuid, ecu);
        }

        delete ourModel;
        return ret;
    }

    QHash<QString, ControlUnitPtr > Manager::findAllModules()
    {
        QHash<QString, ControlUnitPtr > ret;
        QSqlTableModel *ourModel = modulesTable();

        ourModel->setFilter(QString::null);
        ourModel->select();

        for (int i=0; i < ourModel->rowCount(); i++) {
            QSqlRecord ourRecord = ourModel->record(i);
            QString ourUuid = DPP_V1_Parser::rawUuidToString(ourRecord.value("uuid").toByteArray());
            ControlUnitPtr ecu(new ControlUnit(ourUuid, this));
            ret.insert(ourUuid, ecu);
        }

        delete ourModel;
        return ret;
    }


    bool Manager::removeModuleByUuid(const QString &aUuid)
    {
        bool ret = false;
        QSqlTableModel *ourModel = modulesTable();

        ourModel->setFilter(QString("uuid = '%1'").arg(aUuid));
        ourModel->select();

        if (ourModel->rowCount() == 1) {
            ourModel->removeRow(0);
            ret = ourModel->submitAll();
        }

        delete ourModel;
        return ret;
    }

    QHash<QString, QVariant> Manager::findOperationByUuid(const QString &aUuid)
    {
        QHash<QString, QVariant> ret;
        QSharedPointer<QSqlTableModel> ourModel(operationsTable());

        ourModel->setFilter(QString("uuid = '%1'").arg(aUuid));
        ourModel->select();

        if (ourModel->rowCount() == 1) {
            QSqlRecord theRecord = ourModel->record(0);
            for (int i=0; i < ourModel->columnCount(); i++) {
                ret.insert(theRecord.fieldName(i), theRecord.value(i));
            }
        }

        return ret;
    }

    bool Manager::removeOperationByUuid(const QString &aUuid)
    {
        QSharedPointer<QSqlTableModel> ourModel(operationsTable());

        ourModel->setFilter(QString("uuid = '%1'").arg(aUuid));
        ourModel->select();

        if (ourModel->rowCount() == 1) {
            ourModel->removeRows(0, 1);
            return ourModel->submitAll();
        } else {
            qDebug() << "Expected to find 1 row, found: " << ourModel->rowCount();
        }

        return false;
    }

    QHash<QString, QVariant> Manager::findResultByUuid(const QString &aUuid)
    {
        QHash<QString, QVariant> ret;
        QSharedPointer<QSqlTableModel> ourModel(resultsTable());

        ourModel->setFilter(QString("uuid = '%1'").arg(aUuid));
        ourModel->select();

        if (ourModel->rowCount() == 1) {
            QSqlRecord theRecord = ourModel->record(0);
            for (int i=0; i < ourModel->columnCount(); i++) {
                ret.insert(theRecord.fieldName(i), theRecord.value(i));
            }
        }

        return ret;
    }

    bool Manager::removeResultByUuid(const QString &aUuid)
    {
        QSharedPointer<QSqlTableModel> ourModel(resultsTable());

        ourModel->setFilter(QString("uuid = '%1'").arg(aUuid));
        ourModel->select();

        if (ourModel->rowCount() == 1) {
            ourModel->removeRow(0);
            return ourModel->submitAll();
        }

        return false;
    }

    bool Manager::removeStringTableByUuid(const QString &aUuid)
    {
        QSqlQuery removeTheseStringValuesByTableQuery(_db);
        removeTheseStringValuesByTableQuery.prepare("DELETE FROM string_values WHERE table_uuid = :uuid");
        removeTheseStringValuesByTableQuery.bindValue(":uuid", aUuid);

        QSqlQuery removeThisStringTableByUuidQuery(_db);
        removeThisStringTableByUuidQuery.prepare("DELETE FROM string_tables WHERE uuid = :uuid");
        removeThisStringTableByUuidQuery.bindValue(":uuid", aUuid);

        return removeTheseStringValuesByTableQuery.exec() && removeThisStringTableByUuidQuery.exec();
    }

    void Manager::initializeDatabase()
    {
        if (!_db.tables().contains("modules")) {
            qDebug() << "Need to create modules table";
            QSqlQuery query(_db);
            bool ret = query.exec(""                                            \
                                  "CREATE TABLE modules (\n"                    \
                                      "dpp_version  INTEGER NOT NULL,\n"        \
                                      "file_version INTEGER NOT NULL,\n"        \
                                      "uuid         BLOB UNIQUE NOT NULL PRIMARY KEY,\n" \
                                      "address      INTEGER,\n"                 \
                                      "family       VARCHAR,\n"                 \
                                      "name         VARCHAR NOT NULL,\n"        \
                                      "mtime        INTEGER NOT NULL,\n"        \
                                      "parent_id    VARCHAR,\n"                 \
                                      "part_number  INTEGER,\n"                 \
                                      "hardware_num INTEGER,\n"                 \
                                      "software_num INTEGER,\n"                 \
                                      "coding_index INTEGER,\n"                 \
                                      "big_endian   INTEGER\n,"                 \
                                      "CHECK (uuid <> '')\n"                    \
                                  ");"                                          \
                    );

            if (!ret) {
                QString errorString = QString("Problem creating the modules table: %1").arg(query.lastError().driverText());
                throw std::runtime_error(qPrintable(errorString));
            }
        }

        if (!_db.tables().contains("operations")) {
            qDebug() << "Need to create operations table";
            QSqlQuery query(_db);
            bool ret = query.exec("" \
                                  "CREATE TABLE operations (\n"                          \
                                      "uuid      BLOB UNIQUE NOT NULL PRIMARY KEY,\n"    \
                                      "module_id BLOB NOT NULL,\n"                       \
                                      "name      VARCHAR NOT NULL,\n"                    \
                                      "command   BLOB,\n"                                \
                                      "parent_id BLOB,\n"                                \
                                      "UNIQUE (module_id, name),\n"                      \
                                      "CHECK ((CASE WHEN command IS NOT NULL THEN 1 ELSE 0 END + CASE WHEN parent_id IS NOT NULL THEN 1 ELSE 0 END) = 1)," \
                                      "CHECK (uuid <> '')\n"                             \
                                      ");"                                               \
                                 );
            if (!ret) {
                QString errorString = QString("Problem creating the operations table: %1").arg(query.lastError().driverText());
                throw std::runtime_error(qPrintable(errorString));
            }
        }

        if (!_db.tables().contains("results")) {
            qDebug() << "Need to create results table";
            QSqlQuery query(_db);
            bool ret = query.exec("" \
                                  "CREATE TABLE results (\n"                    \
                                      "uuid         BLOB UNIQUE NOT NULL PRIMARY KEY,\n" \
                                      "operation_id BLOB NOT NULL,\n"           \
                                      "name         VARCHAR NOT NULL,\n"        \
                                      "type         VARCHAR NOT NULL,\n"        \
                                      "display      VARCHAR NOT NULL,\n"        \
                                      "start_pos    INTEGER NOT NULL,\n"        \
                                      "length       INTEGER NOT NULL,\n"        \
                                      "mask         INTEGER,\n"                 \
                                      "factor_a     NUMERIC,\n"                 \
                                      "factor_b     NUMERIC,\n"                 \
                                      "levels       VARCHAR,\n"                 \
                                      "rpn          VARCHAR,\n"                 \
                                      "UNIQUE (operation_id, name),\n"          \
                                      "CHECK (uuid <> '')\n"                    \
                                      ");"                                      \
                                  );
            if (!ret) {
                QString errorString = QString("Problem creating the results table: %1").arg(query.lastError().driverText());
                throw std::runtime_error(qPrintable(errorString));
            }
        }

        if (!_db.tables().contains("string_values")) {
            qDebug() << "Need to create string_values table";
            QSqlQuery query(_db);
            bool ret = query.exec("" \
                                  "CREATE TABLE string_values (\n"          \
                                      "table_uuid   BLOB NOT NULL,\n"       \
                                      "number       INTEGER NOT NULL,\n"    \
                                      "string       VARCHAR NOT NULL,\n"    \
                                      "PRIMARY KEY (table_uuid, number)\n"  \
                                      ");"                                  \
                                 );
            if (!ret) {
                QString errorString = QString("Problem creating the string_values table: %1").arg(query.lastError().driverText());
                throw std::runtime_error(qPrintable(errorString));
            }
        }

        if (!_db.tables().contains("string_tables")) {
            qDebug() << "Need to create string_tables table";
            QSqlQuery query(_db);
            bool ret = query.exec("" \
                                  "CREATE TABLE string_tables (\n"          \
                                      "uuid BLOB UNIQUE NOT NULL,\n"        \
                                      "name VARCHAR UNIQUE NOT NULL,\n"     \
                                      "PRIMARY KEY (uuid)\n"                \
                                      ");"                                  \
                                 );
            if (!ret) {
                QString errorString = QString("Problem creating the string_tables table: %1").arg(query.lastError().driverText());
                throw std::runtime_error(qPrintable(errorString));
            }
        }

        QStringList jsonGlob("*.json");
        QDirIterator it(jsonDir(), jsonGlob, QDir::Files | QDir::Readable | QDir::NoDotAndDotDot, QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);

        DPP_V1_Parser *parser = new DPP_V1_Parser(this);
        while (it.hasNext()) {
            QFile jsonFile;
            QString path = it.next();

            jsonFile.setFileName(path);
            jsonFile.open(QIODevice::ReadOnly | QIODevice::Text);
            parser->parseFile(it.fileName(), &jsonFile);
            jsonFile.close();
        }
        delete parser;
    }

    QString Manager::findStringByTableAndNumber(const QString &aStringTable, int aNumber)
    {
        QString uuid = DPP_V1_Parser::stringToUuidSQL(aStringTable);

        // If we were given an invalid UUID, assume we've got to look it up by name.
        // We could join if it weren't such a pain in the ass with Qt.
        if (uuid == "NULL") {
            QSharedPointer<QSqlTableModel> ourStringTableTable(stringTablesTable());
            ourStringTableTable->setFilter(QString("name = '%1'").arg(aStringTable));
            ourStringTableTable->select();
            if (ourStringTableTable->rowCount() == 1) {
                QSqlRecord ourRecord = ourStringTableTable->record(0);
                uuid = DPP_V1_Parser::rawUuidToString(ourRecord.value("uuid").toByteArray());
                uuid = DPP_V1_Parser::stringToUuidSQL(uuid);
            }
        }

        QSharedPointer<QSqlTableModel> ourStringValueTable(stringValuesTable());
        ourStringValueTable->setFilter(QString("(table_uuid = %1) AND (number = %2)").arg(uuid).arg(aNumber));
        ourStringValueTable->select();

        if (ourStringValueTable->rowCount() == 1) {
           QSqlRecord ourRecord = ourStringValueTable->record(0);
           return ourRecord.value("string").toString();
        }
        return QString::null;
    }
}
