#include <QCoreApplication>
#include <QTimer>
#include <QCommandLineParser>

#include "datacollection.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationVersion("0.1");
    app.setOrganizationDomain("inferiorhumanorgans.com");
    app.setOrganizationName("Inferior Human Organs, Inc.");

    DataCollection *dc = new DataCollection(&app);
    QObject::connect(dc, &DataCollection::finished, &app, &QCoreApplication::quit);

    QTimer::singleShot(0, dc, SLOT(run()));

    return app.exec();
}
