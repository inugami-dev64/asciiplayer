#include <iostream>
#include <thread>

#include "utils/console_info.h"

int main() {
    const char *words[] = {
        "Welcome",
        "I'm happy to see you here :3",
        "Please wait, while we are getting ready",
        "Almost there...",
        "Just a little more...",
        "Done!",
        "❤️🏳️‍⚧️"
    };

    for (const char *word : words) {
        ap::Console::clear_console();
        ap::Console::output(word);
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    }

    ap::Rectangle<int> rect = ap::Console::get_console_dimensions();
    std::cout << "Console dimensions: " << rect.width << "x" << rect.height << std::endl;
    return 0;
}
