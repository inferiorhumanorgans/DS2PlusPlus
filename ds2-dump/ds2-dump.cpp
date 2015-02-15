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

#include <sys/time.h>
#include <unistd.h>

#include <iostream>

#include <QDebug>
#include <QFile>
#include <QDate>

#include <ds2packet.h>
#include <manager.h>
#include <controlunit.h>
#include <exceptions.h>

#include "ds2-dump.h"

DataCollection::DataCollection(QObject *parent) :
    QObject(parent), qOut(stdout), qErr(stderr)
{
}

void DataCollection::run()
{
    using namespace DS2PlusPlus;

    parser = QSharedPointer<QCommandLineParser>(new QCommandLineParser);
    parser->setApplicationDescription("Dumps a DS2 packet from an ECU");
    parser->addHelpOption();
    parser->addVersionOption();

    dbm = ManagerPtr(new Manager(parser));

    QCommandLineOption reloadJsonOption(QStringList() << "r" << "reload" << "load", "Load JSON data into SQL db");
    parser->addOption(reloadJsonOption);

    QCommandLineOption setEcuOption(QStringList() << "e" << "ecu", "The ECU to operate on (family name, numerical address, or UUID).", "ecu");
    parser->addOption(setEcuOption);

    QCommandLineOption listFamiliesOption(QStringList() << "f" << "families", "Print the known ECU families.");
    parser->addOption(listFamiliesOption);

    QCommandLineOption listEcuOption(QStringList() << "c" << "ecus", "Print the known ECUs for a given family.", "family");
    parser->addOption(listEcuOption);

    QCommandLineOption listOperationsOption(QStringList() << "o" << "operations", "Print the known operations for a given ECU.  ECU UUID must be specified with --ecu.");
    parser->addOption(listOperationsOption);

    QCommandLineOption iterateOption(QStringList() << "n" << "iterate", "Iterate <n> number of times.", "n");
    parser->addOption(iterateOption);

    QCommandLineOption textPacketOption(QStringList() << "i" << "input-packet", "Treat this argument as a packet instead of reading from the serial port.  Base 16, space delimited.", "input-packet");
    parser->addOption(textPacketOption);

    QCommandLineOption detectEcuOption(QStringList() << "P" << "probe", "Probe an ECU at <ecu> for its identity.");
    parser->addOption(detectEcuOption);

    QCommandLineOption autoDiscoverOption(QStringList() << "A" << "probe-all", "Probe all known ECU addresses and print the results.");
    parser->addOption(autoDiscoverOption);

    QCommandLineOption runJobOption(QStringList() << "J" << "run-operation", "Run an operation on an ECU, prints results as JSON to stdout.  Must also specify --ecu.", "operation");
    parser->addOption(runJobOption);

    QCommandLineOption datalogOption(QStringList() << "D" << "data-log", "Create a CSV log, write until interrupted.", "ecu-jobs-and-results");
    parser->addOption(datalogOption);

    QCommandLineOption rawQueryOption(QStringList() << "Q" << "query", "Send packet to ECU, print raw output.", "query");
    parser->addOption(rawQueryOption);

    parser->process(*QCoreApplication::instance());

    try {
        if (!parser->isSet("reload") && !parser->isSet("families") && !parser->isSet("ecus") && !parser->isSet("operations")) {
            if (!parser->isSet("input-packet")) {
                if (!parser->isSet("port")) {
                    throw CommandlineArgumentException("A serial port is required.");
                }
                this->serialSetup(parser);
            }
        }

        dbm->initializeManager();

        // This will rescan all the JSON files, we should be smarter about doing this.
        if (parser->isSet("reload")) {
            dbm->initializeDatabase();
        }

        if (parser->isSet("families")) {
            listFamilies();
            emit finished();
            return;
        }

        if (parser->isSet("ecus")) {
            listEcus();
            emit finished();
            return;
        }

        if (parser->isSet("ecu")) {
            QString ecuString = parser->value("ecu");

            if (ecuString.length() == 36) {
                ecuUuid = ecuString;
            } else {
                bool ok;
                quint8 ourAddress;
                if (ecuString.startsWith("0x")) {
                    ourAddress=ecuString.toUShort(&ok, 16);
                } else {
                    ourAddress=ecuString.toUShort(&ok, 10);
                }
                if (ok) {
                    ecuAddressList << ourAddress;
                } else {
                    ecuAddressList << ControlUnit::addressForFamily(ecuString.toUpper());
                    if (ecuAddressList.isEmpty()) {
                        throw CommandlineArgumentException("Please specify a valid positive integer or an ECU family name for the ECU address.");
                    }
                }
            }
        }

        if (parser->isSet("operations")) {
            listOperations();
            emit finished();
            return;
        }

        if (parser->isSet("probe-all")) {
            this->probeAll();
            emit finished();
            return;
        }

        if (!parser->isSet("ecu") && !parser->isSet("data-log") && !parser->isSet("reload") && !parser->isSet("query")) {
            throw CommandlineArgumentException("A valid ECU family or address is required to proceed further.");
        }

        if (parser->isSet("run-operation")) {
            runOperation();
            emit finished();
            return;
        }

        if (parser->isSet("query")) {
            rawQuery();
            emit finished();
            return;
        }

        if (parser->isSet("probe")) {
            DS2PlusPlus::ControlUnitPtr autoDetect;

            foreach (quint8 ecuAddress, ecuAddressList) {
                try {
                    autoDetect = DS2PlusPlus::ControlUnitPtr(dbm->findModuleAtAddress(ecuAddress));
                } catch(DS2PlusPlus::TimeoutException) {
                    continue;
                }
                if (!autoDetect.isNull()) {
                    break;
                }
            }

            if (!autoDetect.isNull()) {
                qOut << QString("At 0x%1 we think we have: %2").arg(autoDetect->address(), 2, 16, QChar('0')).arg(autoDetect->name()) << endl;
                DS2Response ourResponse = autoDetect->executeOperation("identify");
                qOut << "Identity:" << endl << DS2ResponseToJsonString(ourResponse) << endl;
            } else {
            }
            emit finished();
            return;
        }

        if (parser->isSet("data-log")) {
            dataLog();
            emit finished();
            return;
        }
    } catch (CommandlineArgumentException exception) {
        qErr << "There was a problem with your invocation: " << exception.what() << endl << endl;
        parser->showHelp(-1);
    }

    emit finished();
}

void DataCollection::listFamilies()
{
    using namespace DS2PlusPlus;

    QStringList families = ControlUnit::knownFamilies();
    families.sort();
    qOut << "Known ECU families: " << families.join(", ") << endl;

    return;
}

void DataCollection::listEcus()
{
    using namespace DS2PlusPlus;

    QHash<QString, ControlUnitPtr> ourEcus;

    QString aFamily = parser->value("ecus");
    if (aFamily == "ALL") {
        ourEcus = dbm->findAllModules();
    } else {
        if (ControlUnit::knownFamilies().contains(aFamily)) {
            ourEcus = dbm->findAllModulesByFamily(aFamily);
        } else {
            throw CommandlineArgumentException("Unknown family specified");
        }
    }

    QMap<QString, QString> ecuSortMap;
    foreach(const ControlUnitPtr ourEcu, ourEcus.values()) {
        ecuSortMap.insert(ourEcu->name(), ourEcu->uuid());
    }

    int totalLen = 38+40+9+21;
    qOut << qSetFieldWidth(38) << left << "UUID";
    qOut << qSetFieldWidth(40) << left << "Name";
    qOut << qSetFieldWidth(9)  << left << "Version";
    qOut << qSetFieldWidth(21) << left << "Last Modified";

    if (aFamily == "ALL") {
        qOut << qSetFieldWidth(8) << left << "Family";
        totalLen += 8;
    }

    qOut << qSetFieldWidth(1) << endl;


    qOut << qSetFieldWidth(totalLen) << qSetPadChar('-') << "" << qSetFieldWidth(1) << qSetPadChar(' ') << endl;

    foreach (const QString &ourUuid, ecuSortMap.values()) {
        ControlUnitPtr ourEcu = ourEcus[ourUuid];
        qOut << qSetFieldWidth(38) << left << ourEcu->uuid();
        qOut << qSetFieldWidth(40) << left << ourEcu->name();
        qOut << qSetFieldWidth(9)  << left << ourEcu->fileVersion();
        qOut << qSetFieldWidth(21) << left << ourEcu->fileLastModified().toString(Qt::ISODate);

        if (aFamily == "ALL") {
            qOut << qSetFieldWidth(8) << left << ourEcu->family();
        }

        qOut << qSetFieldWidth(1) << endl;
    }
    qOut << endl;
}

void DataCollection::listOperations()
{
    using namespace DS2PlusPlus;

    if (ecuUuid.isEmpty()) {
        qErr << "A valid ECU UUID is required to proceed further." << endl;
        emit finished();
        return;
    }

    ControlUnitPtr ourEcu(new ControlUnit(ecuUuid));
    QHash<QString, OperationPtr> ourOperations = ourEcu->operations();
    qOut << "Operations for: " << ourEcu->name() << " (" << ourEcu->uuid() << ")" << endl << endl;

    QHash<QString, QStringList> opsHash;
    foreach (const OperationPtr ourOp, ourOperations.values()) {

        QList<Result> results = ourOp->results().values();
        QStringList resultNames;
        foreach (const Result &result, results) {
            resultNames.append(result.name());
        }
        resultNames.sort();
        opsHash[ourOp->name()] = resultNames;
    }

    QStringList opNames = opsHash.keys();
    opNames.sort();
    foreach (const QString &ourOpName, opNames) {
        qOut << qSetFieldWidth(11) << left << "Operation:";
        qOut << qSetFieldWidth(1)  << ourOpName << endl;
        qOut << qSetFieldWidth(11) << "Returns:";
        qOut << qSetFieldWidth(1)  << opsHash[ourOpName].join(", ") << endl;
        qOut << endl;
    }
}

void DataCollection::probeAll()
{
    using namespace DS2PlusPlus;

    QList<quint8> addresses = ControlUnit::knownAddresses();

    int totalLen = 12+40+15+20+40;
    qOut << qSetFieldWidth(12) << left << "Family";
    qOut << qSetFieldWidth(40) << left << "Name";
    qOut << qSetFieldWidth(15) << left << "Part Number";
    qOut << qSetFieldWidth(20) << left << "Manufacturer";
    qOut << qSetFieldWidth(40) << left << "Notes";
    qOut << qSetFieldWidth(1)  << endl;
    qOut << qSetFieldWidth(totalLen) << qSetPadChar('-') << "" << qSetFieldWidth(1) << qSetPadChar(' ') << endl;

    foreach (quint8 address, addresses) {
        usleep(100000);

        DS2PlusPlus::ControlUnitPtr autoDetect;
        try {
            autoDetect = DS2PlusPlus::ControlUnitPtr (dbm->findModuleAtAddress(address));
        } catch(DS2PlusPlus::TimeoutException exception) {
            continue;
        }

        if (autoDetect.isNull()) {
            autoDetect = ControlUnitPtr(new ControlUnit(ControlUnit::ROOT_UUID, &(*dbm)));
            autoDetect->setAddress(address);
        }

        usleep(100000);
        DS2Response ourResponse;
        try {
            if (getenv("DPP_TRACE")) {
                qDebug() << "Running identify";
            }

            ourResponse = autoDetect->executeOperation("identify");
        } catch(DS2PlusPlus::TimeoutException exception) {
            qDebug() << "Timeout (identify)";
            continue;
        }

        qOut << qSetFieldWidth(12) << left << (autoDetect->isRoot() ? ControlUnit::familyForAddress(address) : autoDetect->family());
        qOut << qSetFieldWidth(40) << left << (autoDetect->isRoot() ? "Unknown" : autoDetect->name());
        qOut << qSetFieldWidth(15) << left << ourResponse.value("part_number").toString();
        qOut << qSetFieldWidth(20) << left << ourResponse.value("supplier").toString();

        QStringList notes;
        if (autoDetect->matchFlags() & ControlUnit::MatchSWMismatch) {
            notes.append("SW mismatch");
        }

        if (autoDetect->matchFlags() & ControlUnit::MatchHWMismatch) {
            notes.append("HW mismatch");
        }

        if (autoDetect->matchFlags() & ControlUnit::MatchCIMismatch) {
            notes.append("CI mismatch");
        }

        DS2Response ourVin;
        if (autoDetect->operations().contains("vehicle_id")) {
            usleep(250000);
            try {
                ourVin = autoDetect->executeOperation("vehicle_id");
                if (ourVin.contains("vin")) {
                    notes.append(QString("vin=%1").arg(ourVin.value("vin").toString()));
                }
            } catch(DS2PlusPlus::TimeoutException exception) {
            }
        } else if (autoDetect->operations().contains("vehicle_id_short")) {
            usleep(250000);
            try {
                DS2Response ourVin = autoDetect->executeOperation("vehicle_id_short");
                if (ourVin.contains("short_vin")) {
                    notes.append(QString("vin=%1").arg(ourVin.value("short_vin").toString()));
                }
            } catch(DS2PlusPlus::TimeoutException exception) {
            }
        }

        if (autoDetect->operations().contains("dtc_count")) {
            usleep(250000);
            try {
                DS2Response ourFaultCountResponse;
                ourFaultCountResponse = autoDetect->executeOperation("dtc_count");
                if (ourFaultCountResponse.contains("error_code.count")) {
                    quint64 ourFaultCount = ourFaultCountResponse.value("error_code.count").toULongLong();
                    if (ourFaultCount > 0) {
                        notes.append(QString("faults=%1").arg(ourFaultCount));
                    }
                }
            } catch(DS2PlusPlus::TimeoutException exception) {
            }
        }

        qOut << qSetFieldWidth(40) << left << notes.join(", ");
        qOut << qSetFieldWidth(1)  << endl;
    }
    return;
}

void DataCollection::rawQuery()
{
    using namespace DS2PlusPlus;

    DS2PacketPtr queryPacket(new DS2Packet(parser->value("query")));
    DS2PacketPtr responsePacket =  dbm->query(queryPacket);

    qOut << "Response: " << responsePacket << endl;
}

void DataCollection::runOperation()
{
    using namespace DS2PlusPlus;

    ControlUnitPtr autoDetect;
    DS2PacketPtr ourPacket;

    if (parser->isSet("input-packet")) {
        qOut << "Reading from packet specified on command line." << endl << endl;
    }

    if (ecuUuid.isEmpty()) {
        if (parser->isSet("input-packet")) {
            qOut << "Auto detection from a command line packet is not supported." << endl << "Please specify an ECU UUID when specifying a packet on the command line." << endl;
            finished();
            return;
        }

        foreach(quint8 ecuAddress, ecuAddressList) {
            autoDetect = dbm->findModuleAtAddress(ecuAddress);
            if (!autoDetect.isNull()) {
                break;
            }
        }
    } else {
        autoDetect = ControlUnitPtr(new ControlUnit(ecuUuid, &(*dbm)));
        ecuAddressList << autoDetect->address();

        if (parser->isSet("input-packet")) {
            ourPacket = DS2PacketPtr(new DS2Packet(parser->value("input-packet")));
        } else {
            qDebug() << "No Input packet specified";
        }
    }

    if (!autoDetect.isNull()) {
        QString ourJob = parser->value("run-operation");
        qOut << QString("At 0x%1 we think we have: %2").arg(autoDetect->address(), 2, 16, QChar('0')).arg(autoDetect->name()) << endl;

        bool ok;
        quint64 iterations;
        if (!parser->value("iterate").isEmpty()) {
            iterations = parser->value("iterate").toULongLong(&ok);
            if (!ok) {
                qErr << "Please specify a valid positive integer for the number of iterations." << endl;
                emit finished();
                return;
            }
        } else {
            iterations = 1;
        }

        for (quint64 i=0; i < iterations; i++) {
            DS2Response ourResponse;
            if (!ourPacket.isNull()) {
                ourResponse = autoDetect->parseOperation(ourJob, ourPacket);
            } else {
                ourResponse = autoDetect->executeOperation(ourJob);
            }
            qOut << "\"" << ourJob << "\"" << ": " << DS2ResponseToJsonString(ourResponse) << endl;
            if ((i < iterations - 1) and (iterations > 1)) {
                sleep(1);
            }
        }

    } else {
        qOut << "Couldn't find a match" << endl;
    }
}

class DataLogEntry {
public:
    QString ecuName;
    QString jobName;
    QStringList results;
};

void DataCollection::dataLog()
{
    using namespace DS2PlusPlus;

    // ECU:job-1:result,result,result
    QStringList logSpecs = parser->values("data-log");
    QMap<QString, DS2PlusPlus::ControlUnitPtr> ecus;
    QMap<QString, DataLogEntry> jobs;

    foreach (const QString &spec, logSpecs) {
        QStringList currentSpec = spec.split(":");

        if (currentSpec.length() != 3) {
            throw CommandlineArgumentException("Data log spec must follow the format ECU:job:result1,result2,resultn");
        }

        QString ecuName = currentSpec.at(0);
        QString jobName = currentSpec.at(1);
        QString key = QString("%1:%2").arg(ecuName, jobName);
        QStringList resultsList = currentSpec.at(2).split(",");

        if (!ecus.contains(ecuName)) {
            DS2PlusPlus::ControlUnitPtr ourEcu;
            QList<quint8> ecuAddresses = ControlUnit::addressForFamily(ecuName);
            foreach(quint8 address, ecuAddresses) {
                ourEcu = DS2PlusPlus::ControlUnitPtr(dbm->findModuleAtAddress(address));
                if (!ourEcu.isNull()) {
                    break;
                }
            }
            if (ourEcu.isNull()) {
                throw std::runtime_error(qPrintable(QString("Could not locate ECU at %1").arg(ecuName)));
            }
            ecus.insert(ecuName, ourEcu);
        }
        if (!jobs.contains(key)) {
            jobs[key].ecuName = ecuName;
            jobs[key].jobName = jobName;
        }
        jobs[key].results.append(resultsList);
    }


    QFile file(QString("dpp-%1.csv").arg(QDateTime::currentDateTime().toString()));
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    QTextStream StdOut(stdout);

    QStringList headers, formats;

    foreach (const DataLogEntry &entry, jobs.values()) {
        headers << QString("%1:%2 Time").arg(entry.ecuName).arg(entry.jobName);
        formats << "s";

        foreach (const QString &resultName, entry.results) {
            Result r = ecus[entry.ecuName]->operations()[entry.jobName]->results()[resultName];
            headers << QString("%1:%2:%3").arg(entry.ecuName).arg(entry.jobName).arg(resultName);
            formats << r.units();
        }
    }

    out << headers.join("\t") << endl;
    out << formats.join("\t") << endl;

    StdOut << headers.join("\t") << endl;
    StdOut << formats.join("\t") << endl;

    struct timeval tv;
    gettimeofday(&tv, NULL);
    double startTime = tv.tv_sec + (0.000001 * tv.tv_usec);

    while (true) {
        QStringList outValues;
        foreach (const DataLogEntry &entry, jobs.values()) {
            DS2PlusPlus::ControlUnitPtr ourEcu = ecus[entry.ecuName];

            QStringList ourResults = entry.results;

            gettimeofday(&tv, NULL);
            double execTime = tv.tv_sec + (0.000001 * tv.tv_usec);
            double curTime = execTime - startTime;

            DS2Response ourResponse = ourEcu->executeOperation(entry.jobName);

            outValues << QString::number(curTime, 'f', 5);

            foreach (const QString &resultName, ourResults) {
                QVariant ourResult = ourResponse.value(resultName);
                switch (ourResult.type()) {
                case QMetaType::Short:
                case QMetaType::Int:
                case QMetaType::Long:
                case QMetaType::LongLong:
                    outValues << QString::number(ourResult.toLongLong());
                    break;
                case QMetaType::UShort:
                case QMetaType::UInt:
                case QMetaType::ULong:
                case QMetaType::ULongLong:
                    outValues << QString::number(ourResult.toULongLong());
                    break;
                case QMetaType::Double:
                case QMetaType::Float:
                    outValues << QString::number(ourResult.toDouble(), 'f', 5);
                    break;
                case QMetaType::QString:
                    outValues << ourResult.toString();
                    break;
                default:
                    outValues << "";
                    break;
                }
            }

            usleep(200000); // 1/5th sec sleep
        }
        QString outputLine = outValues.join("\t");
        out << outputLine << endl;
        StdOut << outputLine << endl;

        usleep(750000); // 3/4th sec sleep
    }
    file.close();
}
