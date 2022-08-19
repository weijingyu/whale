#include "window.h"

#include <log.h>

int main(int argc, char **argv) {

    whale::Log::init();
	whale::Log::getCoreLogger()->info("From whale!");   

    // Main window
    {
        whale::Window window;
        window.loop();
    }

    return EXIT_SUCCESS;
}
