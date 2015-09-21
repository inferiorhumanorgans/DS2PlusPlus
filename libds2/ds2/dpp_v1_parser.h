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

        static QString rawUuidToString(const QByteArray &aRawUuid);
        static QVariant stringToUuidVariant(const QString &aUuid);
        static QString stringToUuidSQL(const QString &aRawUuid);

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

        bool parseOperationJson(Json::ValueIterator &operationIt, Json::Value &moduleJSON);
        bool parseResultJson(Json::ValueIterator &aResultIterator, Json::Value &operationJSON);

    protected:
        Manager *_manager;
        const QIODevice *_input;
        QSet<QString> _knownUuids;
        QTextStream qOut, qErr;
    };
}
#endif // DPP_V1_PARSER_H
