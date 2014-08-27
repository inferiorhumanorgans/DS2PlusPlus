#include "exception.h"

#include <QString>
#include <iostream>

namespace DS2PlusPlus {
    Exception::Exception(const QString &aMessage, const QString &aClassName) :
        _message(aMessage), _who(aClassName)
    {
    }

    const QString Exception::message() const
    {
        return _message;
    }

    const QString Exception::who() const
    {
        return _who;
    }

    InvalidData::InvalidData(const QString &aMessage) :
        Exception(aMessage, "InvalidData")
    {
    }

    SQLError::SQLError(const QString &aMessage) :
        Exception(aMessage, "SQLError")
    {
    }

    IOError::IOError(const QString &aMessage, const QString &aClassName) :
        Exception(aMessage, aClassName)
    {
    }

    SerialIOError::SerialIOError(const QString &aMessage) :
        IOError(aMessage, "SerialIOError")
    {
    }
}

QDebug operator << (QDebug d, const DS2PlusPlus::Exception &anException)
{
    QString formattedMessage = QString("%1: %2").arg(anException.who()).arg(anException.message());
    d << formattedMessage;
    return d;
}

QDebug operator << (QDebug d, const DS2PlusPlus::Exception *anException)
{
    d << *anException;
    return d;
}

std::ostream& operator<< (std::ostream& stream, const DS2PlusPlus::Exception &anException)
{
    QString formattedMessage = QString("%1: %2").arg(anException.who()).arg(anException.message());
    stream << qPrintable(formattedMessage);
    return stream;
}


std::ostream& operator<< (std::ostream& stream, const DS2PlusPlus::Exception *anException)
{
    stream << *anException;
    return stream;
}
