#include "Application.h"
#include <iostream>
#include <csignal>

static Application* g_app = nullptr;

void signalHandler(int signum) {
    if (g_app) {
        g_app->requestShutdown();
    }
}

int main(int argc, char* argv[]) {
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    try {
        Application app;
        g_app = &app;

        if (!app.initialize(argc, argv)) {
            std::cerr << "Failed to initialize application" << std::endl;
            return 1;
        }

        app.run();
        app.shutdown();
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}