#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdexcept>
#include <iostream>

namespace DS2PlusPlus {
    class TimeoutException : public std::ios_base::failure {
    public:
        TimeoutException() : std::ios_base::failure("Timeout") {}
    };

    class CommandlineArgumentException : public std::invalid_argument {
    public:
        CommandlineArgumentException(std::string s) : std::invalid_argument(s) {}
    };
}

#endif // EXCEPTIONS_H
