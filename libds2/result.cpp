#include <QDebug>

#include "result.h"

namespace DS2PlusPlus {
    void Result::setUuid(const QString &aUuid)
    {
        _uuid = aUuid;
    }

    const QString Result::uuid() const
    {
        return _uuid;
    }

    void Result::setName(const QString &aName)
    {
        _name = aName;
    }

    const QString Result::name() const
    {
        return _name;
    }

    void Result::setType(const QString &aType)
    {
        _type = aType;
    }

    const QString Result::type() const
    {
        return _type;
    }

    bool Result::isType(const QString &aType) const
    {
        return _type == aType;
    }

    void Result::setDisplayFormat(const QString &aDisplayFormat)
    {
        _displayFormat = aDisplayFormat;
    }

    const QString Result::displayFormat() const
    {
        return _displayFormat;
    }

    void Result::setStartPosition(int aStartPosition)
    {
        _startPosition = aStartPosition;
    }

    int Result::startPosition() const
    {
        return _startPosition;
    }

    void Result::setLength(int aLength)
    {
        _length = aLength;
    }

    int Result::length() const
    {
        return _length;
    }

    void Result::setMask(const QString &aMask)
    {
        if (aMask.isEmpty()) {
            _mask = 0xff;
        } else {
            bool ok;
            _mask = aMask.toUInt(&ok, 16);
            if (!ok) {
                qDebug() << "Problem setting result mask: " << ok << " for " << aMask;
            }
        }
    }

    int Result::mask() const
    {
        return _mask;
    }

    void Result::setFactorA(double aFactor)
    {
        _factorA = aFactor;
    }

    double Result::factorA() const
    {
        return _factorA;
    }

    void Result::setFactorB(double aFactor)
    {
        _factorB = aFactor;
    }

    double Result::factorB() const
    {
        return _factorB;
    }

    void Result::setYesValue(const QString &aString)
    {
        _yesValue = aString;
    }

    const QString Result::yesValue() const
    {
        return _yesValue;
    }

    void Result::setNoValue(const QString &aString)
    {
        _noValue = aString;
    }

    const QString Result::noValue() const
    {
        return _noValue;
    }

    const QString Result::stringForBoolean(bool aBoolean) const
    {
        return aBoolean ? _yesValue : _noValue;
    }
}
