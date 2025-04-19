//
// Created by user on 25/04/20.
//

#include "Logger.h"

namespace ap {
    void Logger::log(LogLevel level, const char *message) {
        if (m_level >= level) {
            time_t now = time(0);
            tm* timeinfo = localtime(&now);
            char timestamp[32];
            strftime(timestamp, sizeof(timestamp),
                "%Y-%m-%d %H:%M:%S", timeinfo);

            m_stream << "[" << timestamp << "] " << level2string(level) << ": " << message << std::endl;
            m_stream.flush();
        }
    }

} // ap