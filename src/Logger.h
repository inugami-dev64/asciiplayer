//
// Created by user on 25/04/20.
//

#ifndef LOGGER_H
#define LOGGER_H
#include <fstream>

namespace ap {

    enum LogLevel {
        DEBUG,
        INFO,
        WARNING,
        ERROR,
        CRITICAL
    };

    class Logger {
    public:
        Logger(std::ostream& stream, LogLevel level) :
            m_stream(stream),
            m_level(level) {}

        void log(LogLevel level, const char* message);
    private:
        std::ostream& m_stream;
        LogLevel m_level;

        const char* level2string(LogLevel level) {
            switch (level) {
                case DEBUG:
                    return "DEBUG";
                case INFO:
                    return "INFO";
                case WARNING:
                    return "WARNING";
                case ERROR:
                    return "ERROR";
                case CRITICAL:
                    return "CRITICAL";
                default:
                    return "UNKNOWN";
            }
        }
    };

} // ap

#endif //LOGGER_H
