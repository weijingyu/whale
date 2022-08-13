#include "window.h"

int main(int argc, char **argv) {
    

    // Main window
    {
        Whale::Window window;
        window.loop();
    }

    return EXIT_SUCCESS;
}
