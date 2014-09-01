#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QObject>
#include <QSharedPointer>
#include <QCommandLineParser>

#include <QSqlDatabase>
#include <QSqlTableModel>

#include "controlunit.h"

class QSerialPort;

namespace DS2PlusPlus
{
    /*!
     * \brief The Manager class
     */
    class Manager : public QObject
    {
        Q_OBJECT
    public:
        /*!
         * \brief Manager
         * \param aParser A QSharedPointer to a QCommandLineParser.  This is used to add additional switches.  If this is specified, Manager::initializeManager() must be called before this object can be used.
         * \param aParent
         */
        explicit Manager(QSharedPointer<QCommandLineParser> aParser=QSharedPointer<QCommandLineParser>(), QObject *aParent = 0);
        virtual ~Manager();

        /*!
         * \brief initializeManager
         *
         * initializeManager will setup all the various paths and such (serial port, database, etc).  It is called explicitly by the constructor if no QCommandLineParser is pased in.
         */
        void initializeManager();

        /*!
         * \brief initializeDatabase
         */
        void initializeDatabase();

        /*!
         * \brief DPP_DB_PATH
         */
        static const QString DPP_DB_PATH;

        /*!
         * \brief DPP_JSON_PATH
         */
        static const QString DPP_JSON_PATH;

        /*!
         * \brief DPP_DIR
         */
        static const QString DPP_DIR;

        QSqlTableModel *modulesTable();
        QSqlTableModel *operationsTable();
        QSqlTableModel *resultsTable();
        QSqlTableModel *stringTable();

        QString dppDir();
        QString jsonDir();
        QString serialPortPath() const {return _serialPortPath;}

        DS2PacketPtr query(DS2PacketPtr aPacket);

        ControlUnitPtr findModuleAtAddress(quint8 anAddress);
        ControlUnitPtr findModuleByMatchingIdentPacket(const DS2PacketPtr packet);

        /*!
         * \brief findModuleRecordByUuid
         * \param aUuid
         * \return a hash that can be used by the ControlUnit class to walk the hierarchy
         */
        QHash<QString, QVariant> findModuleRecordByUuid(const QString &aUuid);

        QHash<QString, ControlUnitPtr> findAllModulesByAddress(quint8 anAddress);

        QHash<QString, ControlUnitPtr> findAllModulesByFamily(const QString &aFamily);

        QHash<QString, ControlUnitPtr> findAllModules();

        QHash<QString, QVariant> findOperationByUuid(const QString &aUuid);

        QHash<QString, QVariant> findResultByUuid(const QString &aUuid);

        /*!
         * \brief findStringByTableAndNumber
         * \param aStringTable
         * \param aNumber
         * \return
         */
        QString findStringByTableAndNumber(const QString &aStringTable, int aNumber);

        /*!
         * \brief removeModuleByUuid
         * \param aUuid
         * \return
         */
        bool removeModuleByUuid(const QString &aUuid);

        /*!
         * \brief removeOperationByUuid
         * \param aUuid
         * \return
         */
        bool removeOperationByUuid(const QString &aUuid);

        /*!
         * \brief removeResultByUuid
         * \param aUuid
         * \return
         */
        bool removeResultByUuid(const QString &aUuid);

        bool removeStringTableByUuid(const QString &aUuid);

    protected:
        QSqlDatabase _db;
        QString _dppDir;
        QString _serialPortPath;
        QSerialPort *_serialPort;
        QSharedPointer<QCommandLineParser> _cliParser;
    };

    typedef QSharedPointer<Manager> ManagerPtr;
}


#endif // DBMANAGER_H
