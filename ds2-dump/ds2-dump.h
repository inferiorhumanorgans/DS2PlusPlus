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

#ifndef DATACOLLECTION_H
#define DATACOLLECTION_H

#include <QObject>
#include <QSharedPointer>
#include <QCommandLineParser>

#include <ds2/manager.h>

class DataCollection : public QObject
{
    Q_OBJECT
public:
    explicit DataCollection(QObject *aParent = 0);

signals:
    void finished();

public slots:
    void run();
    void probe();
    void probeAll();
    void listFamilies();
    void listEcus();
    void listOperations();
    void runOperation();
    void dataLog();
    void rawQuery();

protected:
    void serialSetup(QSharedPointer<QCommandLineParser> parser);
    DS2PlusPlus::ManagerPtr dbm;
    QTextStream qOut, qErr;
    QSharedPointer<QCommandLineParser> parser;
    QString ecuUuid;
    QList<quint8> ecuAddressList;
};

#endif // DATACOLLECTION_H
