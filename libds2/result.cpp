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

#include <stdexcept>

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

    void Result::setRpn(const QString &aRpnString)
    {
        if (!aRpnString.isEmpty()) {
            _rpn = aRpnString.split(" ");
        }
    }

    QStringList Result::rpn() const
    {
        return _rpn;
    }

    void Result::setLevels(QHash<QString, QString> someLevels)
    {
        _levels = someLevels;
    }

    const QString Result::stringForLevel(quint8 aLevel) const
    {
        // Boolean
        if (_levels.contains("yes") and _levels.contains("no") and _levels.count() == 2) {
                return (aLevel == true) ? _levels.value("yes") : _levels.value("no");
        }

        qint32 ourLevelCount = 0;
        QStringList ourLevels;
        for (QHash<QString, QString>::const_iterator it = _levels.begin(); it != _levels.end(); ++it) {
            if ((it.key() == "else") or (it.key() == "all")) {
                continue;
            }

            ourLevelCount++;

            bool ok;
            quint8 mask = it.key().toUShort(&ok, 16);

            if (!ok) {
                throw std::invalid_argument("Invalid mask found");
            }

            if ((aLevel & mask) > 0) {
                ourLevels.append(it.value());
            }
        }

        if ((ourLevels.isEmpty()) and (_levels.contains("else"))) {
            return _levels.value("else");
        }

        if (ourLevels.isEmpty()) {
            return QString::null;
        }

        if ((ourLevels.count() == ourLevelCount) and _levels.contains("all")) {
            return _levels.value("all");
        }

        return ourLevels.join(",");
    }
}
