#ifndef DPP_V1_PARSER_H
#define DPP_V1_PARSER_H

#include <QObject>
#include <QIODevice>
#include <QSet>
#include <QSharedPointer>
#include <QSqlTableModel>
#include <QTextStream>

namespace Json {
    class Value;
    class ValueIterator;
}

namespace DS2PlusPlus {
    class Manager;

    class DPP_V1_Parser : public QObject
    {
        Q_OBJECT
    public:
        explicit DPP_V1_Parser(Manager *parent = 0);
        void parseFile(const QString &aLabel, QIODevice *anInput);
        void reset();

    protected:
        /*!
         * \brief parseEcuFile
         * \param aJsonObject
         */
        void parseEcuFile(const Json::Value &aJsonObject);

        /*!
         * \brief parseStringTableFile
         * \param aJsonObject
         */
        void parseStringTableFile(const Json::Value &aJsonObject);

        bool parseOperationJson(Json::ValueIterator &operationIt, Json::Value &moduleJSON, QSharedPointer<QSqlTableModel> &operationsModel);
        bool parseResultJson(Json::ValueIterator &aResultIterator, Json::Value &operationJSON, QSharedPointer<QSqlTableModel> &aResultsModel);

    protected:
        Manager *_manager;
        const QIODevice *_input;
        QSet<QString> _knownUuids;
        QTextStream qOut, qErr;
    };
}
#endif // DPP_V1_PARSER_H
