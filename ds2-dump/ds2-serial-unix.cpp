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

#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <stdio.h>

#include "ds2-dump.h"

void DataCollection::serialSetup(QSharedPointer<QCommandLineParser> parser)
{
    QString serialPortName = parser->value("device");

    int fd = open(qPrintable(serialPortName), O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd < 0) {
        throw std::ios_base::failure(strerror(errno));
    }
    if (!isatty(fd)) {
        close(fd);
        throw std::ios_base::failure("This is not a tty");
    }
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) & ~O_NONBLOCK);

    struct termios tty;
    memset(&tty, 0, sizeof tty);

    if (tcgetattr (fd, &tty) != 0) {
        throw std::ios_base::failure(strerror(errno));
    }

    tty.c_oflag = 0;
    tty.c_lflag = 0;
    tty.c_iflag &= (IXON | IXOFF | IXANY);
    tty.c_cflag |= CLOCAL | CREAD;
    tty.c_cflag &= ~HUPCL;

    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;

    tty.c_cflag |= PARENB;
    tty.c_cflag &= ~PARODD;

    tty.c_cflag &= ~CSTOPB;

    if (tcsetattr (fd, TCSANOW, &tty) != 0) {
        throw std::ios_base::failure(strerror(errno));
    }

    int data_rate = B9600;
    cfsetispeed(&tty, data_rate);
    cfsetospeed(&tty, data_rate);

    dbm->setFd(fd);
}
