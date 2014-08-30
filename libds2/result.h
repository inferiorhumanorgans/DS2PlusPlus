#ifndef RESULT_H
#define RESULT_H

#include <QString>

namespace DS2PlusPlus {
    class Result
    {
    public:
        Result () {}

        void setUuid(const QString &aUuid);
        const QString uuid() const;

        void setName(const QString &aName);
        const QString name() const;

        void setType(const QString &aType);
        const QString type() const;

        bool isType(const QString &aType) const;

        void setDisplayFormat(const QString &aDisplayFormat);
        const QString displayFormat() const;

        void setStartPosition(int aStartPosition);
        int startPosition() const;

        void setLength(int aLength);
        int length() const;

        void setMask(const QString &aMask);
        int mask() const;

        void setFactorA(double aFactor);
        double factorA() const;

        void setFactorB(double aFactor);
        double factorB() const;

        void setYesValue(const QString &aString);
        const QString yesValue() const;

        void setNoValue(const QString &aString);
        const QString noValue() const;

        const QString stringForBoolean(bool aBoolean) const;

    protected:
        QString _uuid;
        QString _name;
        QString _type;
        QString _displayFormat;
        int _startPosition;
        int _length;
        int _mask;
        float _factorA;
        float _factorB;
        QString _yesValue, _noValue;
    };
}

#endif // RESULT_H
