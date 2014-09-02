#include <stdexcept>

#include "controlunit.h"
#include "operation.h"

namespace DS2PlusPlus {

    Operation::Operation (const QString &aUuid, quint8 aControlUnitAddress, const QString &aName, const QByteArray &aCommand)
        : _uuid(aUuid), _name(aName), _controlUnitAddress(aControlUnitAddress), _command(aCommand)
    {
    }

    const QString Operation::uuid() const
    {
        return _uuid;
    }

    const QString Operation::name() const
    {
        return _name;
    }

    const QStringList Operation::command() const
    {
        QStringList ret;
        for (int i=0; i < _command.length(); i++) {
            ret.append(QString("0x%1").arg(_command.at(i), 2, 16, QChar('0')));
        }

        return ret;
    }

    const QHash<QString, Result> Operation::results() const
    {
        return _results;
    }

    void Operation::insertResult(const QString &aName, const Result aResult)
    {
        _results.insert(aName, aResult);
    }

    DS2Packet *Operation::queryPacket() const
    {
       return new DS2Packet(_controlUnitAddress, _command);
    }
}
