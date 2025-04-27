#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "src/Player.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: ./asciiplayer <filename>\n");
        exit(1);
    }

    try {
        ap::Player player(
            argv[1],
            ap::Logger(std::cout, ap::DEBUG),
            new ap::VideoPresenter());

        player.play();
    } catch (const std::exception &e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
    }
}
