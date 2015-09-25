#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdexcept>
#include <iostream>

namespace DS2PlusPlus {
    /*!
     * \brief An abstract class for DS2PlusPlus exceptions to inherit.
     */
    class Exception {};

    /*!
     * \brief An exception thrown when a timeout condition is encountered while reading or writing to the serial port.
     */
    class TimeoutException : public Exception, public std::ios_base::failure {
    public:
        TimeoutException() : std::ios_base::failure("Timeout") {}
    };

    /*!
     * \brief An exception thrown when a command line argument is inapproprite.
     */
    class CommandlineArgumentException : public Exception, public std::invalid_argument {
    public:
        CommandlineArgumentException(std::string s) : std::invalid_argument(s) {}
    };
}

#endif // EXCEPTIONS_H
