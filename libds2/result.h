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

        void setRpn(const QString &aRpnString);
        QStringList rpn() const;

        const QString stringForLevel(quint8 aLevel) const;
        void setLevels(QHash<QString, QString> someLevels);

        const QString units() const;
        void setUnits(const QString &aUnit);

    protected:
        QString _uuid;
        QString _name;
        QString _type;
        QString _displayFormat;
        int _startPosition;
        int _length;
        int _mask;
        QStringList _rpn;
        QHash<QString, QString> _levels;
        QString _units;
    };
}

#endif // RESULT_H
