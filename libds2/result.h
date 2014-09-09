#ifndef RESULT_H
#define RESULT_H

#include <QString>

namespace DS2PlusPlus {
    class Result
    {
    public:
        Result () {}

        void setName(const QString &aName);
        const QString name() const;

        void setUuid(const QString &aUuid);
        const QString uuid() const;


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

        const QString stringForLevel(quint8 aLevel) const;
        void setLevels(QHash<QString, QString> someLevels);

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
        QHash<QString, QString> _levels;
    };
}

#endif // RESULT_H
