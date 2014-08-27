#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <QString>
#include <QDebug>

#include <ostream>

namespace DS2PlusPlus {
    class Exception
    {
    public:
        explicit Exception(const QString &aMessage = QString::null, const QString &aClassName = "Exception");

        virtual const QString message() const;
        virtual const QString who() const;

    protected:
        QString _message, _who;
    };

    class InvalidData : public Exception
    {
    public:
        explicit InvalidData(const QString &aMessage = QString::null);
    };

    class SQLError : public Exception
    {
    public:
        explicit SQLError(const QString &aMessage = QString::null);
    };

    class IOError : public Exception
    {
    public:
        explicit IOError(const QString &aMessage = QString::null, const QString &aClassName = "IOError");
    };

    class SerialIOError : public IOError
    {
    public:
        explicit SerialIOError(const QString &aMessage = QString::null);
    };
}

QDebug operator << (QDebug d, const DS2PlusPlus::Exception &anException);
QDebug operator << (QDebug d, const DS2PlusPlus::Exception *anException);

std::ostream& operator<< (std::ostream& stream, const DS2PlusPlus::Exception &anException);
std::ostream& operator<< (std::ostream& stream, const DS2PlusPlus::Exception *anException);

#endif // EXCEPTION_H
