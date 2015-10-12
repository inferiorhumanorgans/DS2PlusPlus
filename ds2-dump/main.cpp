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

#include <QCoreApplication>
#include <QTimer>
#include <QCommandLineParser>

#include "ds2-dump.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationVersion(QString("0.2.5 (libds2 %1)").arg(DS2PlusPlus::Manager::version()));
    app.setOrganizationDomain("inferiorhumanorgans.com");
    app.setOrganizationName("Inferior Human Organs, Inc.");

    DataCollection *dc = new DataCollection(&app);
    QObject::connect(dc, &DataCollection::finished, &app, &QCoreApplication::quit);

    QTimer::singleShot(0, dc, SLOT(run()));

    return app.exec();
}
