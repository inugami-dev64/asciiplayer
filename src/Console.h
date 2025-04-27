//
// Created by user on 25/04/19.
//

#ifndef CONSOLE_INFO_H
#define CONSOLE_INFO_H

namespace ap {

    template <typename T>
    struct Rectangle {
        T width;
        T height;
    };

    /**
     * Utility class for doing console specific operations platform-independently
     */
    class Console {
    public:
        /**
         * Returns the current console dimensions
         * @return a Rectangle object containing the current console dimensions
         */
        static Rectangle<int> get_console_dimensions();

        /**
         * Outputs data to stdout
         * @param data specifies the string to output
         */
        static void output(const char* data);

        /**
         * Clears the console
         */
        static void clear_console();
    };

} // ap

#endif //CONSOLE_INFO_H
