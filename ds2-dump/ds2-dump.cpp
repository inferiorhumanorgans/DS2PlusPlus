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
#include <QVariant>
#include <QList>

#include <ds2packet.h>
#include <kwppacket.h>
#include <manager.h>
#include <controlunit.h>
#include <exceptions.h>

#include "ds2-dump.h"

void PrettyFormat(const QList<QStringList> &rows)
{
    QTextStream qOut(stdout);

    int numCols = rows.at(0).length();
    QList<int> colWidth;

    for (int i=0; i < numCols; i++) {
        colWidth.append(0);
    }

    foreach (const QStringList &row, rows) {
        for (int i=0; i < numCols; i++) {
            colWidth[i] = qMax(colWidth[i], row.at(i).length()+2);
        }
    }

    bool isHeader = true;
    foreach (const QStringList &row, rows) {
        for (int i=0; i < numCols; i++) {
            qOut << qSetFieldWidth(colWidth[i]) << qSetPadChar(QChar(' ')) << left << row.at(i);
        }
        qOut << qSetFieldWidth(0) << endl;

        if (isHeader == true) {
            for (int i=0; i < numCols; i++) {
                qOut << qSetFieldWidth(colWidth[i]) << qSetPadChar(QChar('-')) << left << "-";
            }
            qOut << qSetFieldWidth(0) << endl;
            isHeader = false;
        }
    }

    return;
}

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

    QCommandLineOption setFamilyOption(QStringList() << "f" << "family", "The ECU family to operate on (family name).", "family");
    parser->addOption(setFamilyOption);

    QCommandLineOption setOperationOption(QStringList() << "j" << "operation", "The operation to run.", "operation");
    parser->addOption(setOperationOption);

    QCommandLineOption listEcuOption(QStringList() << "E" << "list-ecus", "Print the known ECUs for a given family.");
    parser->addOption(listEcuOption);

    QCommandLineOption listFamiliesOption(QStringList() << "F" << "list-families", "Print the known ECU families.");
    parser->addOption(listFamiliesOption);

    QCommandLineOption listOperationsOption(QStringList() << "J" << "list-operations", "Print the known operations for a given ECU.  ECU UUID must be specified.");
    parser->addOption(listOperationsOption);

    QCommandLineOption detectEcuOption(QStringList() << "P" << "probe", "Probe an ECU for its identity.");
    parser->addOption(detectEcuOption);

    QCommandLineOption autoDiscoverOption(QStringList() << "A" << "probe-all", "Probe all known ECU addresses and print the results.");
    parser->addOption(autoDiscoverOption);

    QCommandLineOption runJobOption(QStringList() << "R" << "run-operation", "Run an operation on an ECU, prints results as JSON to stdout.");
    parser->addOption(runJobOption);

    QCommandLineOption datalogOption(QStringList() << "D" << "data-log", "Create a CSV log, write until interrupted.", "ecu-jobs-and-results");
    parser->addOption(datalogOption);

    QCommandLineOption rawQueryOption(QStringList() << "Q" << "query", "Send packet to ECU, print raw output.", "query");
    parser->addOption(rawQueryOption);

    QCommandLineOption iterateOption(QStringList() << "n" << "iterate", "Iterate <n> number of times.", "n");
    parser->addOption(iterateOption);

    QCommandLineOption textPacketOption(QStringList() << "i" << "input-packet", "Treat this argument as a packet instead of reading from the serial port.  Base 16, space delimited.", "input-packet");
    parser->addOption(textPacketOption);

    QCommandLineOption outputFormatOption(QStringList() << "o" << "format", "Output format.  Either text, verbose, or json.", "format", "text");
    parser->addOption(outputFormatOption);

    parser->process(*QCoreApplication::instance());

    try {
        if (parser->isSet("format")) {
            QStringList validFormats = QStringList() << "text" << "verbose" << "json";
            if (!validFormats.contains(parser->value("format"))) {
                throw CommandlineArgumentException(qPrintable(QString("Format must be one of: %1").arg(validFormats.join(", "))));
            }
        }

        if (!parser->isSet("reload") && !parser->isSet("list-families") && !parser->isSet("list-ecus") && !parser->isSet("list-operations")) {
            if (!parser->isSet("input-packet")) {
                if (!parser->isSet("device")) {
                    throw CommandlineArgumentException("A serial port is required to complete this operation.");
                }
                this->serialSetup(parser);
            }
        }

        dbm->initializeManager();

        // This will rescan all the JSON files, we should be smarter about doing this.
        if (parser->isSet("reload")) {
            dbm->initializeDatabase();
            emit finished();
            return;
        }

        if (parser->isSet("list-families")) {
            listFamilies();
            emit finished();
            return;
        }

        if (parser->isSet("list-ecus")) {
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

        if (parser->isSet("probe-all")) {
            this->probeAll();
            emit finished();
            return;
        }

        if (!parser->isSet("ecu") && !parser->isSet("data-log") && !parser->isSet("query")) {
            throw CommandlineArgumentException("A valid ECU family or address is required to proceed further.");
        }

        if (parser->isSet("list-operations")) {
            listOperations();
            emit finished();
            return;
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
            probe();
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

    if (parser->value("format") == "text") {
        qOut << "Known ECU families: " << families.join(", ") << endl << endl;
    } else if (parser->value("format") == "verbose") {
        qOut.setFieldAlignment(QTextStream::AlignLeft);
        qOut << qSetFieldWidth(8) << "Family" << qSetFieldWidth(25) << "Addresses" << qSetFieldWidth(1) << endl;

        qOut.setPadChar('-');
        qOut << qSetFieldWidth(33) << "-" << qSetFieldWidth(1) << endl;
        qOut.setPadChar(' ');

        foreach(const QString &family, families) {
            qOut << qSetFieldWidth(8) << family;
            QStringList textList;
            QString stringVal;
            QList<quint8> sortedAddresses(ControlUnit::addressForFamily(family));
            qSort(sortedAddresses);
            foreach(quint8 address, sortedAddresses) {
                stringVal.sprintf(BasePacket::HEX_CHAR_FORMAT, address);
                textList << stringVal;
            }
            qOut << qSetFieldWidth(25) << textList.join(", ");
            qOut << qSetFieldWidth(1) << endl;
        }
    } else if (parser->value("format") == "json") {
        QHash<QString, QVariant> ourHash;
        foreach(const QString &family, families) {
            QList<quint8> sortedAddresses(ControlUnit::addressForFamily(family));
            qSort(sortedAddresses);
            QVariant variant;
            variant.setValue<QList<quint8> >(sortedAddresses);
            ourHash.insert(family, variant);
        }
        qOut << HashToJsonString(ourHash, "families") << endl;
    }

    return;
}

void DataCollection::listEcus()
{
    using namespace DS2PlusPlus;

    QHash<QString, ControlUnitPtr> ourEcus;

    QString aFamily = parser->value("family");
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

    QList<QStringList> output;
    QStringList headerRow;
    headerRow << "UUID" << "Name" << "Version" << "Last Modified";

    if (aFamily == "ALL") {
        headerRow << "Family";
    }

    output.append(headerRow);

    foreach (const QString &ourUuid, ecuSortMap.values()) {
        ControlUnitPtr ourEcu = ourEcus[ourUuid];
        QStringList row;
        row << ourEcu->uuid() << ourEcu->name() << QString::number(ourEcu->fileVersion()) << ourEcu->fileLastModified().toString(Qt::ISODate);

        if (aFamily == "ALL") {
            row << ourEcu->family();
        }

        output.append(row);
    }
    PrettyFormat(output);
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
    qOut << "Control Unit: " << ourEcu->name() << " (" << ourEcu->uuid() << ")" << endl;

    QStringList partStrings;
    QList<quint64> partNumbers = ourEcu->partNumbers().toList();
    qSort(partNumbers);
    foreach (quint64 partNumber, partNumbers) {
        partStrings << QString::number(partNumber, 10);
    }

    if (partStrings.count() == 1) {
        qOut << "Part number:  " << partStrings.first() << endl;
    } else if (!partStrings.isEmpty()){
        qOut << "Part numbers: " << partStrings.join(", ") << endl;
    }

    qOut << "Protocol:     ";
    switch (ourEcu->protocol()) {
    case BasePacket::ProtocolDS2:
        qOut << "DS2";
        break;
    case BasePacket::ProtocolKWP:
        qOut << "KWP-2000";
        break;
    default:
        qOut << "Unknown";
        break;
    }
    qOut << endl;
    qOut << endl;

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

    if (parser->value("format") == "json") {
        qOut << "JSON";
    } else if (parser->value("format") == "text") {
        foreach (const QString &ourOpName, opNames) {
            qOut << qSetFieldWidth(14) << left << "Operation:";
            qOut << qSetFieldWidth(1)  << ourOpName << endl;

            qOut << qSetFieldWidth(14) << left << "Command:";
            qOut << ourEcu->operations()[ourOpName]->command().join(", ");
            qOut << qSetFieldWidth(1) << endl;

            qOut << qSetFieldWidth(14) << "Returns:";
            qOut << qSetFieldWidth(1)  << opsHash[ourOpName].join(", ") << endl;
            qOut << endl;
        }
    } else if (parser->value("format") == "verbose") {
        foreach (const QString &ourOpName, opNames) {
            qOut << qSetFieldWidth(14) << left << "Operation:";
            qOut << qSetFieldWidth(1)  << ourOpName << endl;

            qOut << qSetFieldWidth(14) << left << "Command:";
            qOut << ourEcu->operations()[ourOpName]->command().join(", ");
            qOut << qSetFieldWidth(1) << endl;

            qOut << qSetFieldWidth(14) << "Returns:" << left << "Data Type" << "Name";
            qOut << qSetFieldWidth(1)  << endl;

            qOut << qSetFieldWidth(14) << " " << qSetPadChar('-') << "-" << qSetFieldWidth(50) << "-" << qSetPadChar(' ') << qSetFieldWidth(1) << endl;

            foreach (const QString &aResultName, opsHash[ourOpName]) {
                Result result = ourEcu->operations()[ourOpName]->results()[aResultName];

                qOut << qSetFieldWidth(14) << left << " ";
                qOut << qSetFieldWidth(14) << left;

                if (result.type() == "byte") {
                    if (result.displayFormat().startsWith("string_table:")) {
                        qOut << "string";
                    } else if (result.displayFormat() == "raw") {
                        qOut << "integer (dec)";
                    } else if (result.displayFormat() == "hex_string") {
                        qOut << "integer (hex)";
                    } else if (result.displayFormat() == "hex_int") {
                        qOut << "integer (dec)";
                    } else if (result.displayFormat() == "float") {
                        qOut << "decimal";
                    } else {
                        qOut << "byte " << result.displayFormat();
                    }
                } else if (result.type() == "boolean") {
                    if (result.displayFormat() == "string") {
                        qOut << "string (bool)";
                    } else {
                        qOut << "boolean";
                    }
                } else if (result.type() == "hex_string") {
                    if (result.displayFormat() == "int") {
                        qOut << "integer (dec)";
                    } else {
                        qOut << result.displayFormat();
                    }
                } else if (result.type() == "signed_byte") {
                    qOut << "int (signed)";
                } else if (result.type() == "6bit-string") {
                    qOut << "string";
                } else {
                    qOut << result.type();
                }

                qOut << qSetFieldWidth(14) << left << aResultName;
                qOut << qSetFieldWidth(1)  << endl;
            }

            qOut << qSetFieldWidth(1)  << endl;
        }
    }
}

void DataCollection::probe()
{
    using namespace DS2PlusPlus;

    ControlUnitPtr autoDetect;

    foreach (quint8 ecuAddress, ecuAddressList) {
        try {
            // Progress info
            qOut << qSetFieldWidth(80) << qSetPadChar(' ') << left << QString(">> Probing ECU at 0x%1 (%2)")
                    .arg(ecuAddress, 2, 16, QChar('0'))
                    .arg(ControlUnit::familyForAddress(ecuAddress)) << qSetFieldWidth(0) << endl;

            autoDetect = ControlUnitPtr(dbm->findModuleAtAddress(ecuAddress));
        } catch(DS2PlusPlus::TimeoutException) {
            continue;
        }
        if (!autoDetect.isNull()) {
            break;
        }
        usleep(5000);
    }
    usleep(5000);

    if (!autoDetect.isNull()) {
        qOut << QString("At 0x%1 we think we have: %2").arg(autoDetect->address(), 2, 16, QChar('0')).arg(autoDetect->name()) << endl;
        PacketResponse ourResponse = autoDetect->executeOperation("identify");
        qOut << "Identity:" << endl << ResponseToJsonString(ourResponse) << endl;
    } else {
#if 0
        try {
            DS2PacketPtr anIdentPacket(new DS2Packet(autoDetect->address(), QByteArray(1, 0)));
            DS2PacketPtr aResponsePacket = dbm->query(anIdentPacket);

            qOut << "Couldn't find a match, got this response: " << endl << aResponsePacket << endl;
        } catch(DS2PlusPlus::TimeoutException exception) {
            qDebug() << "Another timeout here at: " <<
        }
#endif
    }
    emit finished();
    return;
}

void DataCollection::probeAll()
{
    using namespace DS2PlusPlus;

    qOut << "-- Probing all known ECUs" << endl;

    QList<quint8> addresses = ControlUnit::knownAddresses();

    QList<QStringList> output;
    QList<quint32> colWidths = QList<quint32>() << 0 << 0 << 0 << 0 << 0;

    output << (QStringList() << "Family" << "Name" << "Part Number" << "Manufacturer" << "Notes");
    for (int i=0; i < 5; i++) {
        colWidths[i] = qMax(colWidths[i], static_cast<quint32>(output[0][i].length()));
    }

    int ecuPosition = 0;
    foreach (quint8 address, addresses) {
        QStringList currentOutput;
        currentOutput << "" << "" << "" << "" << "";

        usleep(100000);

        DS2PlusPlus::ControlUnitPtr autoDetect;
        try {
            if (parser->value("format") == "text") {
                qOut << qSetFieldWidth(0) << "\r";
            }
            qOut << qSetFieldWidth(80) << qSetPadChar(' ') << left << QString(">> Probing ECU at 0x%1 (%2), %3/%4 %5%")
                    .arg(address, 2, 16, QChar('0'))
                    .arg(ControlUnit::familyForAddress(address))
                    .arg(ecuPosition)
                    .arg(addresses.count())
                    .arg(
                        static_cast<quint32>((ecuPosition++ / static_cast<double>(addresses.length())) * 100)
                    );

            if (parser->value("format") == "text") {
                qOut << qSetFieldWidth(0) << "\r";
                qOut.flush();
            } else if (parser->value("format") == "text") {
                qOut << endl;
            }

            autoDetect = DS2PlusPlus::ControlUnitPtr(dbm->findModuleAtAddress(address));
        } catch(DS2PlusPlus::TimeoutException) {
            if (parser->value("format") == "verbose") {
                qOut << QString("-- Uncaught timeout for ECU at 0x%1").arg(address, 2, 16, QChar('0')) << endl;
            }

            continue;
        }

        if (autoDetect.isNull()) {
            if (parser->value("format") == "verbose") {
                qOut << "-- Unable to identify ECU, trying identify based on root ECU definition." << endl;
            }

            autoDetect = ControlUnitPtr(new ControlUnit(ControlUnit::ROOT_UUID, &(*dbm)));
            autoDetect->setAddress(address);
        }

        usleep(100000);
        PacketResponse ourResponse;
        try {
            if (parser->value("format") == "verbose") {
                qOut << QString(">> Interrogating ECU at 0x%1 for additional details").arg(address, 2, 16, QChar('0')) << endl;
            }
            ourResponse = autoDetect->executeOperation("identify");
        } catch(DS2PlusPlus::TimeoutException) {
            if (parser->value("format") == "verbose") {
                qOut << QString("-- Uncaught timeout for ECU at 0x%1").arg(address, 2, 16, QChar('0')) << endl;
            }
            continue;
        }

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

        PacketResponse ourVin;
        if (autoDetect->operations().contains("vehicle_id")) {
            usleep(250000);
            try {
                ourVin = autoDetect->executeOperation("vehicle_id");
                if (ourVin.contains("vin")) {
                    notes.append(QString("vin=%1").arg(ourVin.value("vin").toString()));
                }
            } catch(DS2PlusPlus::TimeoutException) {
            }
        } else if (autoDetect->operations().contains("vehicle_id_short")) {
            usleep(250000);
            try {
                PacketResponse ourVin = autoDetect->executeOperation("vehicle_id_short");
                if (ourVin.contains("short_vin")) {
                    notes.append(QString("vin=%1").arg(ourVin.value("short_vin").toString()));
                }
            } catch(DS2PlusPlus::TimeoutException) {
            }
        }

        if (autoDetect->operations().contains("dtc_count")) {
            usleep(250000);
            try {
                PacketResponse ourFaultCountResponse;
                ourFaultCountResponse = autoDetect->executeOperation("dtc_count");
                if (ourFaultCountResponse.contains("error_code.count")) {
                    quint64 ourFaultCount = ourFaultCountResponse.value("error_code.count").toULongLong();
                    if (ourFaultCount > 0) {
                        notes.append(QString("faults=%1").arg(ourFaultCount));
                    }
                }
            } catch(DS2PlusPlus::TimeoutException) {
            }
        }

        currentOutput[0] = (autoDetect->isRoot() ? ControlUnit::familyForAddress(address) : autoDetect->family());
        currentOutput[1] = (autoDetect->isRoot() ? "Unknown" : autoDetect->name());
        currentOutput[2] = BasePacket::prettyPrintPartNumber(ourResponse.value("part_number").toString());
        currentOutput[3] = ourResponse.value("supplier").toString();
        currentOutput[4] = notes.join(", ");

        for (int i=0; i < 5; i++) {
            colWidths[i] = qMax(colWidths[i], static_cast<quint32>(currentOutput[i].length()));
        }

        output << currentOutput;
    }

    for (int i=0; i < 5; i++) {
        qOut << qSetFieldWidth(colWidths[i] + 2) << left << qSetPadChar(' ') << output[0][i];
    }
    qOut << qSetFieldWidth(0) << qSetPadChar(' ') << endl;

    for (int i=0; i < 5; i++) {
        qOut << qSetFieldWidth(colWidths[i] + 2) << left << qSetPadChar('-') << '-';
    }
    qOut << qSetFieldWidth(0) << qSetPadChar(' ') << endl;

    output.takeFirst();

    foreach (const QStringList &row, output) {
        for (int i=0; i < 5; i++) {
            qOut << qSetFieldWidth(colWidths[i] + 2) << left << qSetPadChar(' ') << row[i];
        }
        qOut << qSetFieldWidth(0) << qSetPadChar(' ') << endl;
    }

    return;
}

void DataCollection::rawQuery()
{
    using namespace DS2PlusPlus;

    BasePacketPtr queryPacket(new DS2Packet(parser->value("query")));
    BasePacketPtr responsePacket =  dbm->query(queryPacket);

    qOut << "<< REPLY: " << responsePacket << endl;
}

void DataCollection::runOperation()
{
    using namespace DS2PlusPlus;

    ControlUnitPtr autoDetect;
    BasePacketPtr ourPacket;

    if (ecuUuid.isEmpty()) {
        if (parser->isSet("input-packet")) {
            qOut << "-- Auto detection from a command line packet is not supported." << endl << "-- Please specify an ECU UUID when specifying a packet on the command line." << endl;
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
            QString packetString = parser->value("input-packet");
            try {
                ourPacket = BasePacketPtr(new KWPPacket(packetString));
            } catch (std::domain_error) {
                ourPacket = BasePacketPtr(new DS2Packet(packetString));
            }
        } else {
            qDebug() << "-- No Input packet specified";
        }
    }

    if (!autoDetect.isNull()) {
        QString ourJob = parser->value("operation");
        qOut << QString("%1: %3 at 0x%2")
                .arg(ecuUuid.isEmpty() ? "Detected" : "Using")
                .arg(autoDetect->address(), 2, 16, QChar('0')).arg(autoDetect->name()) << endl;

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
            PacketResponse ourResponse;

            if (!ourPacket.isNull()) {
                ourResponse = autoDetect->parseOperation(ourJob, ourPacket);
            } else {
                ourResponse = autoDetect->executeOperation(ourJob);
            }

            if (parser->value("format") == "text") {
                QStringList resultNames = ourResponse.keys();
                resultNames.sort();

                int longestResultName = 0;
                foreach (const QString &result, resultNames) {
                    longestResultName = qMax(result.length(), longestResultName);
                }

                foreach (const QString &result, resultNames) {
                    qOut << left << qSetFieldWidth(longestResultName + 2) << result << qSetFieldWidth(0);
                    QVariant variantValue = ourResponse[result];

                    if (variantValue.type() == static_cast<QVariant::Type>(QMetaType::QString)) {
                        QString ourString = variantValue.toString();

                        if (ourString.isEmpty()) {
                            qOut << "null";
                        } else {
                            qOut << variantValue.toString();
                        }
                    } else if (
                               (variantValue.type() == static_cast<QVariant::Type>(QMetaType::Int)) ||
                               (variantValue.type() == static_cast<QVariant::Type>(QMetaType::Long)) ||
                               (variantValue.type() == static_cast<QVariant::Type>(QMetaType::LongLong))
                               ) {
                        qOut << variantValue.toLongLong();
                    } else if (
                               (variantValue.type() == static_cast<QVariant::Type>(QMetaType::UInt)) ||
                               (variantValue.type() == static_cast<QVariant::Type>(QMetaType::ULong)) ||
                               (variantValue.type() == static_cast<QVariant::Type>(QMetaType::ULongLong))
                              ) {
                        qOut << variantValue.toULongLong();
                    } else if ((variantValue.type() == static_cast<QVariant::Type>(QMetaType::Double)) || (variantValue.type() == static_cast<QVariant::Type>(QMetaType::Float))) {
                        qOut << variantValue.toDouble();
                    } else {
                        qDebug() << "Uknown variant type: " << variantValue.typeName();
                    }

                    qOut << endl;
                }
            } else if (parser->value("format") == "json") {
                qOut << "\"" << ourJob << "\"" << ": " << ResponseToJsonString(ourResponse) << endl;
            }

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

            PacketResponse ourResponse = ourEcu->executeOperation(entry.jobName);

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
