#ifndef OPERATION_H
#define OPERATION_H

#include <QString>
#include <QStringList>
#include <QHash>

#include <result.h>
#include <ds2packet.h>

namespace DS2PlusPlus {
    class Operation
    {
    public:
        Operation (const QString &aUuid, quint8 aControlUnitAddress, const QString &aName, const QByteArray &aCommand);

        const QString uuid() const;
        const QString moduleId() const;
        const QString name() const;
        const QStringList command() const;
        void setCommand(const QStringList &aCommand);

        const QHash<QString, Result> results() const;
        void insertResult(const QString &aName, const Result aResult);

        DS2Packet *queryPacket() const;

    protected:
        QString _uuid, _name;
        quint8 _controlUnitAddress;
        QByteArray _command;
        QHash<QString, Result> _results;
    };

    typedef QSharedPointer<Operation> OperationPtr;
}

#endif // OPERATION_H
