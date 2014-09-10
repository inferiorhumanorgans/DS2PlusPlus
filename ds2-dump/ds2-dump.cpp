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
#include <iostream>
#include <stdexcept>

#include <QDebug>
#include <QCommandLineParser>
#include <QSharedPointer>
#include <QFile>
#include <QDate>

#include <ds2packet.h>
#include <manager.h>
#include <controlunit.h>

#include "ds2-dump.h"

DataCollection::DataCollection(QObject *parent) :
    QObject(parent)
{
}

void DataCollection::run()
{
    using namespace DS2PlusPlus;

    QTextStream qOut(stdout);
    QTextStream qErr(stderr);

    QSharedPointer<QCommandLineParser> parser(new QCommandLineParser);
    parser->setApplicationDescription("Dumps a DS2 packet from an ECU");
    parser->addHelpOption();
    parser->addVersionOption();

    dbm = ManagerPtr(new Manager(parser));

    QCommandLineOption reloadJsonOption(QStringList() << "r" << "reload" << "load", "Load JSON data into SQL db");
    parser->addOption(reloadJsonOption);

    QCommandLineOption setEcuOption(QStringList() << "e" << "ecu", "The ECU to operate on (family name or numerical address).", "ecu");
    parser->addOption(setEcuOption);

    QCommandLineOption detectEcuOption(QStringList() << "p" << "probe", "Probe an ECU at <ecu> for its identity.");
    parser->addOption(detectEcuOption);

    QCommandLineOption runJobOption(QStringList() << "j" << "run-job", "Probe an ECU at <ecu> for its identity.", "job");
    parser->addOption(runJobOption);

    QCommandLineOption iterateOption(QStringList() << "n" << "iterate", "Iterate <n> number of times.", "n");
    parser->addOption(iterateOption);

    QCommandLineOption datalogOption(QStringList() << "d" << "data-log", "Create a CSV log, write until interrupted.", "ecu-jobs-and-results");
    parser->addOption(datalogOption);

    QCommandLineOption textPacketOption(QStringList() << "i" << "input-packet", "packet", "input-packet");
    parser->addOption(textPacketOption);

    QCommandLineOption listFamiliesOption("list-families", "Print the known ECU families.");
    parser->addOption(listFamiliesOption);

    QCommandLineOption listEcuOption("list-ecus", "Print the known ECUs for a given family.", "family");
    parser->addOption(listEcuOption);

    QCommandLineOption listOperationsOption("list-operations", "Print the known operations for a given ECU.");
    parser->addOption(listOperationsOption);

    parser->process(*QCoreApplication::instance());

    if (!parser->isSet("reload") && !parser->isSet("list-families") && !parser->isSet("list-ecus") && !parser->isSet("list-operations")) {
        if (!parser->isSet("input-packet")) {
            this->serialSetup(parser);
        }
    }

    dbm->initializeManager();

    // This will rescan all the JSON files, we should be smarter about doing this.
    if (parser->isSet("reload")) {
        dbm->initializeDatabase();
    }

    if (parser->isSet("list-families")) {
        // Gross but this will initialize the hash for us.
        ControlUnit::addressForFamily("");

        QStringList families = ControlUnit::knownFamilies();
        families.sort();
        qOut << "Known ECU families: " << families.join(", ") << endl;

        emit finished();
        return;
    }

    if (parser->isSet("list-ecus")) {
        QHash<QString, ControlUnitPtr> ourEcus;

        QString aFamily = parser->value("list-ecus");
        if (aFamily == "ALL") {
            ourEcus = dbm->findAllModules();
        } else {
            ourEcus = dbm->findAllModulesByFamily(aFamily);
        }

        int totalLen = 38+40+9+21;
        qOut << qSetFieldWidth(38) << left << "UUID";
        qOut << qSetFieldWidth(40) << left << "Name";
        qOut << qSetFieldWidth(9)  << left << "Version";
        qOut << qSetFieldWidth(21)  << left << "Last Modified";

        if (aFamily == "ALL") {
            qOut << qSetFieldWidth(8) << left << "Family";
            totalLen += 8;
        }

        qOut << qSetFieldWidth(1) << endl;


        qOut << qSetFieldWidth(totalLen) << qSetPadChar('-') << "" << qSetFieldWidth(1) << qSetPadChar(' ') << endl;

        foreach (const ControlUnitPtr ourEcu, ourEcus) {
            qOut << qSetFieldWidth(38) << left << ourEcu->uuid();
            qOut << qSetFieldWidth(40) << left << ourEcu->name();
            qOut << qSetFieldWidth(9)  << left << ourEcu->fileVersion();
            qOut << qSetFieldWidth(21)  << left << ourEcu->fileLastModified().toString(Qt::ISODate);
            if (aFamily == "ALL") {
                qOut << qSetFieldWidth(8) << left << ControlUnit::familyForAddress(ourEcu->address());
            }
            qOut << qSetFieldWidth(1) << endl;
        }
        qOut << endl;

        emit finished();
        return;
    }

    QString ecuUuid;
    quint8 ecuAddress = 0;
    if (parser->isSet("ecu")) {
        QString ecuString = parser->value("ecu");

        if (ecuString.length() == 36) {
            ecuUuid = ecuString;
        } else {
            bool ok;
            if (ecuString.startsWith("0x")) {
                ecuAddress = ecuString.toUShort(&ok, 16);
            } else {
                ecuAddress = ecuString.toUShort(&ok, 10);
            }
            if (!ok) {
                ecuAddress = ControlUnit::addressForFamily(ecuString.toUpper());
                if (ecuAddress == static_cast<quint8>(-99)) {
                    qErr << "Please specify a valid positive integer or an ECU family name for the ECU address." << endl;
                    emit finished();
                    return;
                }
            }
        }
    }

    if (parser->isSet("list-operations")) {
        if  (ecuUuid.isEmpty()) {
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
            qOut << qSetFieldWidth(1) << ourOpName << endl;
            qOut << qSetFieldWidth(11) << "Returns:";
            qOut << qSetFieldWidth(1) << opsHash[ourOpName].join(", ") << endl;
            qOut << endl;
        }

        emit finished();
        return;
    }

    if (!parser->isSet("ecu") && !parser->isSet("data-log")) {
        if (!parser->isSet("reload")) {
            qErr << "A valid ECU family or address is required to proceed further." << endl;
        }
        emit finished();
        return;
    }

    if (parser->isSet("run-job")) {
        if (parser->isSet("input-packet"))
            qOut << "Reading from packet specified on command line." << endl << endl;

        DS2PlusPlus::ControlUnitPtr autoDetect;
        DS2PacketPtr ourPacket;

        if (ecuUuid.isEmpty()) {
            autoDetect = (dbm->findModuleAtAddress(ecuAddress));
        } else {
            autoDetect = ControlUnitPtr(new ControlUnit(ecuUuid));
            ecuAddress = autoDetect->address();

            ourPacket = DS2PacketPtr(new DS2Packet(parser->value("input-packet")));
        }

        if (!autoDetect.isNull()) {
            QString ourJob = parser->value("run-job");
            qOut << QString("At 0x%1 we think we have: %2").arg(ecuAddress, 2, 16, QChar('0')).arg(autoDetect->name()) << endl;

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
                qOut << "\"" << ourJob << "\"" << ": " << DS2ResponseToString(ourResponse) << endl;
                if ((i < iterations - 1) and (iterations > 1)) {
                    sleep(1);
                }
            }

        } else {
            qOut << "Couldn't find a match" << endl;
        }

    } else if (parser->isSet("probe")) {
        DS2PlusPlus::ControlUnitPtr autoDetect(dbm->findModuleAtAddress(ecuAddress));
        if (!autoDetect.isNull()) {
            qOut << QString("At 0x%1 we think we have: %2").arg(ecuAddress, 2, 16, QChar('0')).arg(autoDetect->name()) << endl;
            DS2Response ourResponse = autoDetect->executeOperation("identify");
            qOut << "Identity:" << endl << DS2ResponseToString(ourResponse) << endl;
        } else {
            DS2PacketPtr anIdentPacket(new DS2Packet(ecuAddress, QByteArray(1, 0)));
            DS2PacketPtr aResponsePacket = dbm->query(anIdentPacket);
            qDebug() << "Couldn't find a match, got this response: " << endl << *aResponsePacket << endl;
        }
    } else if (parser->isSet("data-log")) {
        // ECU:job-1:result,result,result
        QStringList logSpecs = parser->values("data-log");
        QMap<QString, DS2PlusPlus::ControlUnitPtr> ecus;
        QMap<QString, QMap<QString, QStringList> > jobs;

        foreach (const QString &spec, logSpecs) {
            QStringList currentSpec = spec.split(":");

            if (currentSpec.length() != 3) {
                throw std::invalid_argument("Data log spec must follow the format ECU:job:result1,result2,resultn");
            }

            QString ecuName = currentSpec.at(0);
            QString jobName = currentSpec.at(1);
            QStringList resultsList = currentSpec.at(2).split(",");

            if (!ecus.contains(ecuName)) {
                DS2PlusPlus::ControlUnitPtr ourEcu(dbm->findModuleAtAddress(ControlUnit::addressForFamily(ecuName)));
                if (ourEcu.isNull()) {
                    throw std::invalid_argument(qPrintable(QString("Could not locate ECU at %1").arg(ecuName)));
                }
                ecus.insert(ecuName, ourEcu);
            }


            jobs[ecuName][jobName].append(resultsList);
        }


        QFile file(QString("dpp-%1.csv").arg(QDateTime::currentDateTime().toString()));
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&file);

        QStringList headers, formats;

        headers << "Time";
        formats << "s";

        foreach (const QString &ecuName, ecus.keys()) {
            QMap<QString, QStringList> ourJobs = jobs[ecuName];
            foreach (const QString &jobName, ourJobs.keys()) {
                QStringList ourResults = ourJobs[jobName];
                foreach (const QString &resultName, ourResults) {
                    headers << QString("%1:%2:%3").arg(ecuName).arg(jobName).arg(resultName);
                    formats << "int";
                }
            }
        }

        out << headers.join("\t") << endl;
        out << formats.join("\t") << endl;
        out.setRealNumberNotation(QTextStream::FixedNotation);
        out.setRealNumberPrecision(5);

        while (true) {
            QStringList outValues;
            foreach (const QString &ecuName, ecus.keys()) {
                DS2PlusPlus::ControlUnitPtr ourEcu = ecus[ecuName];
                QMap<QString, QStringList> ourJobs = jobs[ecuName];

                foreach (const QString &jobName, ourJobs.keys()) {
                    QStringList ourResults = ourJobs[jobName];

                    DS2Response ourResponse = ourEcu->executeOperation(jobName);

                    foreach (const QString &resultName, ourResults) {
                        QVariant ourResult = ourResponse.value(resultName);
                        switch (ourResult.type()) {
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
                }
            }

            struct timeval tv;
            gettimeofday(&tv, NULL);
            double curTime = tv.tv_sec + (0.000001 * tv.tv_usec);
            out << curTime  << "\t" << outValues.join("\t") << endl;
            usleep(62500000); // 1/16th sec sleep
        }
        file.close();
    }

    emit finished();
}
