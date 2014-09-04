#include <iostream>
#include <stdexcept>

#include <QTextStream>
#include <QDebug>
#include <QSharedPointer>
#include <QCommandLineParser>
#include <QUuid>

#include <QDir>
#include <QDirIterator>

#include <QSqlQuery>
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QSqlError>

#include <QSerialPort>

#include "manager.h"

#include "dpp_v1_parser.h"

void waitForThisManyBytes(QSerialPort &aPort, qint64 someNumberOfBytes)
{
    while ((!aPort.waitForReadyRead(250)) or (aPort.bytesAvailable() < someNumberOfBytes)) {
        //qDebug() << "Bytes available: " << aPort.bytesAvailable() << " / Ready to read: " << aPort.waitForReadyRead(250);
    }
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

    Manager::Manager(QSharedPointer<QCommandLineParser> aParser, QObject *parent) :
        QObject(parent), _dppDir(QString::null), _serialPort(new QSerialPort), _cliParser(aParser)
    {
        if (!_cliParser.isNull()) {
            // We should really check to see if the user has already defined this, and if so we should whine.
            QCommandLineOption targetDirectoryOption(QStringList() << "f" << "port",
                      "Read from serial port <port>.",
                      "port");
            aParser->addOption(targetDirectoryOption);
        } else {
            initializeManager();
        }
    }

    void Manager::initializeManager()
    {
        if (_serialPortPath.isEmpty()) {
            _serialPortPath = getenv("DPP_PORT");
        }

        if (!_cliParser.isNull() and _serialPortPath.isEmpty()) {
            _serialPortPath = _cliParser->value("port");
        }

        // Path of last resort.
        if (_serialPortPath == "auto") {
#ifdef __APPLE__
            _serialPortPath = "/dev/tty.usbserial";
#elif _WIN32
            _serialPortPath = "COM3:";
#else
            _serialPortPath = "/dev/tty.usbserial";
#endif
        }

        // Check to make sure our directory
        // So we can have multiple connections...
        QString connName(QUuid::createUuid().toString());
        _db = QSqlDatabase::addDatabase("QSQLITE", connName);

        QString dppDbPath = QString("%1%2%3").arg(dppDir()).arg(QDir::separator()).arg(DPP_DB_PATH);
        _db.setDatabaseName(expandTilde(dppDbPath));

        if (!_db.open()) {
            QString errorString = QString("Couldn't open the database: %1").arg(_db.lastError().databaseText());
            throw std::runtime_error(qPrintable(errorString));
        }

        // Determine serial port -- platform specific
        if (!_serialPortPath.isEmpty()) {
            _serialPort->setPortName(_serialPortPath);

            // Configure it
            _serialPort->setBaudRate(QSerialPort::Baud9600);
            _serialPort->setDataBits(QSerialPort::Data8);
            _serialPort->setParity(QSerialPort::EvenParity);
            _serialPort->setStopBits(QSerialPort::OneStop);

            // Open it
            if (_serialPort->isOpen()) {
                _serialPort->close();
            }

            if (!_serialPort->open(QIODevice::ReadWrite)) {
                QString errorString = QString("'%1': %2").arg(_serialPort->portName()).arg(_serialPort->errorString());
                throw std::ios_base::failure(qPrintable(errorString));
            }
        }

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

    QSqlTableModel *Manager::stringTable()
    {
        if (!_db.isOpen()) {
            qDebug() << "DB NOT OPEN";
        }

        QSqlTableModel *ret = new QSqlTableModel(this, _db);
        ret->setTable("strings");
        ret->setEditStrategy(QSqlTableModel::OnManualSubmit);

        return ret;

    }

    Manager::~Manager() {
        if (_db.isOpen()) {
            _db.close();
        }

        if (_serialPort) {
            delete _serialPort;
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
            _dppDir = DPP_DIR;
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
        jsonSearchPath.append(QString("%1%2%3").arg(QDir::currentPath()).arg(QDir::separator()).arg(DPP_JSON_PATH));
        jsonSearchPath.append(expandTilde(QString("~%1dpp%2%3").arg(QDir::separator()).arg(QDir::separator()).arg("json")));
        jsonSearchPath.append(QString("%1%2%3").arg(dppDir()).arg(QDir::separator()).arg(DPP_JSON_PATH));
        jsonSearchPath.append(expandTilde(QString("~%1ds2%2%3").arg(QDir::separator()).arg(QDir::separator()).arg("json")));

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
        DS2PacketPtr ret(new DS2Packet);

        _serialPort->setRequestToSend(true);
        // Send query to the ECU
        _serialPort->write((QByteArray)*aPacket);

        // Read the echo back.  We should check to see if it matches, maybe...
        waitForThisManyBytes(*_serialPort, aPacket->data().length() + 3);
        _serialPort->read(aPacket->data().length() + 3);

        // Read the result, save to thePacket
        quint8 ecuAddress;
        quint8 length;

        QByteArray inputArray;

        waitForThisManyBytes(*_serialPort, 2);
        inputArray = _serialPort->read(2);
        if (inputArray.length() != 2) {
            QString errorString = QString("Wanted length 2, got: %1").arg(inputArray.length());
            throw std::ios_base::failure(qPrintable(errorString));
        }
        ecuAddress = inputArray.at(0);
        ret->setAddress(ecuAddress);
        length = inputArray.at(1);

        if (length <= 2) {
            QString errorString = QString("Ack. Got garbage data, length must be >= 2.  Got ECU: %1, LEN: %2").arg(QString::number(ecuAddress, 16)).arg(QString::number(length, 16));
            throw std::ios_base::failure(qPrintable(errorString));
        }

        waitForThisManyBytes(*_serialPort, length - 2);
        inputArray = _serialPort->read(length - 2);
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
        QHash<QString, ControlUnitPtr > modules;
        QHash<QString, ControlUnitPtr >::Iterator moduleIt;
        bool fullMatch = false;
        QString ourKey;

        // TODO: use findAllModulesByAddress instead
        modules = findAllModules();
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
                fullMatch = true;
                ret = ecu;
                ourKey = moduleIt.key();

                quint64 actualSW = response.value("software_number").toString().toULongLong(NULL, 16);
                if (actualSW != ecu->softwareNumber()) {
                    fullMatch = false;
                    QString matchString = QString("SW version mismatch %1 != expected 0x%2")
                                            .arg(response.value("software_number").toString())
                                            .arg(ecu->softwareNumber(), 2, 16, QChar('0'));
                    qDebug() << matchString;
                }

                quint64 actualHW = response.value("hardware_number").toString().toULongLong(NULL, 16);
                if (actualHW != ecu->hardwareNumber()) {
                    fullMatch = false;
                    QString matchString = QString("HW version mismatch %1 != expected 0x%2")
                                            .arg(response.value("hardware_number").toString())
                                            .arg(ecu->hardwareNumber(), 2, 16, QChar('0'));
                    qDebug() << matchString;
                }

                quint64 actualCI = response.value("coding_index").toString().toULongLong(NULL, 16);
                if (actualCI != ecu->codingIndex()) {
                    fullMatch = false;
                    QString matchString = QString("CI mismatch %1 != expected 0x%2")
                                            .arg(response.value("coding_index").toString())
                                            .arg(ecu->codingIndex(), 2, 16, QChar('0'));
                    qDebug() << matchString;
                }

                if (fullMatch == true) {
                    break;
                }
            }
        }

        modules.remove(ourKey);
        modules.clear();

        return ret;
    }

    QHash<QString, QVariant> Manager::findModuleRecordByUuid(const QString &aUuid)
    {
        QHash<QString, QVariant> ret;
        QSqlRecord rec;
        QSqlTableModel *ourModel = modulesTable();

        ourModel->setFilter(QString("uuid = '%1'").arg(aUuid));
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

    QHash<QString, ControlUnitPtr > Manager::findAllModulesByFamily(const QString &aFamily)
    {
        QHash<QString, ControlUnitPtr > ret;
        QSqlTableModel *ourModel = modulesTable();

        ourModel->setFilter(QString("family= '%1'").arg(aFamily));
        ourModel->select();

        for (int i=0; i < ourModel->rowCount(); i++) {
            QSqlRecord theRecord = ourModel->record(i);
            QString uuid(theRecord.value(theRecord.indexOf("uuid")).toString());
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
            QString uuid(ourRecord.value(ourRecord.indexOf("uuid")).toString());
            ControlUnitPtr ecu(new ControlUnit(uuid, this));
            ret.insert(uuid, ecu);
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
        QSqlQuery removeThisStringTableQuery(_db);
        removeThisStringTableQuery.prepare("DELETE FROM strings WHERE table_uuid = :uuid");
        removeThisStringTableQuery.bindValue(":uuid", aUuid);
        return removeThisStringTableQuery.exec();
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
                                      "uuid         VARCHAR UNIQUE NOT NULL PRIMARY KEY,\n" \
                                      "address      INTEGER,\n"                 \
                                      "family       VARCHAR,\n"                 \
                                      "name         VARCHAR NOT NULL,\n"        \
                                      "parent_id    VARCHAR,\n"                 \
                                      "part_number  INTEGER,\n"                 \
                                      "hardware_num INTEGER,\n"                 \
                                      "software_num INTEGER,\n"                 \
                                      "coding_index INTEGER,\n"                 \
                                      "big_endian   INTEGER\n"                  \
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
                                      "uuid      VARCHAR UNIQUE NOT NULL PRIMARY KEY,\n" \
                                      "module_id VARCHAR NOT NULL,\n"                    \
                                      "name      VARCHAR NOT NULL,\n"                    \
                                      "command   BLOB,\n"                                \
                                      "parent_id VARCHAR,\n"                             \
                                      "CHECK ((CASE WHEN command IS NOT NULL THEN 1 ELSE 0 END + CASE WHEN parent_id IS NOT NULL THEN 1 ELSE 0 END) = 1)" \
                                      ");"                                   \
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
                                      "uuid         VARCHAR UNIQUE NOT NULL PRIMARY KEY,\n" \
                                      "operation_id VARCHAR NOT NULL,\n"        \
                                      "name         VARCHAR NOT NULL,\n"        \
                                      "type         VARCHAR NOT NULL,\n"        \
                                      "display      VARCHAR NOT NULL,\n"        \
                                      "start_pos    INTEGER NOT NULL,\n"        \
                                      "length       INTEGER NOT NULL,\n"        \
                                      "mask         INTEGER,\n"                 \
                                      "factor_a     NUMERIC,\n"                 \
                                      "factor_b     NUMERIC,\n"                 \
                                      "levels       VARCHAR\n"                  \
                                      ");"                                      \
                                  );
            if (!ret) {
                QString errorString = QString("Problem creating the results table: %1").arg(query.lastError().driverText());
                throw std::runtime_error(qPrintable(errorString));
            }
        }

        if (!_db.tables().contains("strings")) {
            qDebug() << "Need to create strings table";
            QSqlQuery query(_db);
            bool ret = query.exec("" \
                                  "CREATE TABLE strings (\n"                              \
                                      "table_uuid VARCHAR NOT NULL,\n"                    \
                                      "table_name VARCHAR NOT NULL,\n"                    \
                                      "number     INTEGER NOT NULL,\n"                    \
                                      "string     VARCHAR NOT NULL,\n"                    \
                                      "PRIMARY KEY (table_uuid, number)\n"                \
                                      ");"                                                \
                                 );
            if (!ret) {
                QString errorString = QString("Problem creating the strings table: %1").arg(query.lastError().driverText());
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
        QSharedPointer<QSqlTableModel> ourStringTable(stringTable());
        ourStringTable->setFilter(QString("((table_name = '%1') OR (table_uuid = '%2')) AND (number = %3)").arg(aStringTable).arg(aStringTable).arg(aNumber));
        ourStringTable->select();

        if (ourStringTable->rowCount() == 1) {
           QSqlRecord ourRecord = ourStringTable->record(0);
           return ourRecord.value(ourRecord.indexOf("string")).toString();
        }
        return QString::null;
    }
}
