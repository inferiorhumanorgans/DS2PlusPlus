#include <QDebug>
#include <QCommandLineParser>
#include <QSharedPointer>

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

    ManagerPtr dbm(new Manager(parser));

    QCommandLineOption setEcuOption(QStringList() << "e" << "ecu", "The ECU to operate on.", "ecu");
    parser->addOption(setEcuOption);

    QCommandLineOption detectEcuOption(QStringList() << "p" << "probe", "Probe an ECU for its identity.");
    parser->addOption(detectEcuOption);

    QCommandLineOption runJobOption(QStringList() << "j" << "run-job", "Probe an ECU at <address> for its identity.", "job");
    parser->addOption(runJobOption);

    QCommandLineOption iterateOption(QStringList() << "n" << "iterate", "Iterate <n> number of times over the job.", "n");
    parser->addOption(iterateOption);

    parser->process(*QCoreApplication::instance());
    dbm->initializeManager();

    // This will rescan all the JSON files, we should be smarter about doing this.
    dbm->initializeDatabase();

    quint8 ecuAddress;
    if (parser->isSet("ecu")) {
        QString ecuString = parser->value("ecu");
        bool ok;
        if (ecuString.startsWith("0x")) {
            ecuAddress = ecuString.toUShort(&ok, 16);
        } else {
            ecuAddress = ecuString.toUShort(&ok, 10);
        }
        if (!ok) {
            qErr << "Please specify a valid positive integer for the ECU address." << endl;
            emit finished();
            return;
        }
    } else {
        emit finished();
        return;
    }

    DS2PlusPlus::ControlUnitPtr autoDetect(dbm->findModuleAtAddress(ecuAddress));

    if (parser->isSet("run-job")) {
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
                DS2Response ourResponse = autoDetect->executeOperation(ourJob);
                qDebug() << ourJob << ": " << DS2ResponseToString(ourResponse);
                if ((i < iterations - 1) and (iterations > 1)) {
                    sleep(1);
                }
            }

        } else {
            qOut << "Couldn't find a match" << endl;
        }

    } else if (parser->isSet("probe")) {
        if (!autoDetect.isNull()) {
            qOut << QString("At 0x%1 we think we have: %2").arg(ecuAddress, 2, 16, QChar('0')).arg(autoDetect->name()) << endl;
            DS2Response ourResponse = autoDetect->executeOperation("identify");
            qOut << "Identity:" << endl << DS2ResponseToString(ourResponse) << endl;
        } else {
            DS2PacketPtr anIdentPacket(new DS2Packet(ecuAddress, QByteArray(1, 0)));
            DS2PacketPtr aResponsePacket = dbm->query(anIdentPacket);
            qDebug() << "Couldn't find a match, got this response: " << endl << *aResponsePacket << endl;
        }
    }

    emit finished();
}
