#include "app/Application.h"
#include "common/logger/Logger.h"

int main(int argc, char* argv[]) {
    service::app::Application app(argc, argv);

    if (const auto result = app.initialize(); result.isError()) {
        LOG_ERROR("Initialization failed: {}", result.error());
        return EXIT_FAILURE;
    }

    if (const auto result = app.start(); result.isError()) {
        LOG_ERROR("Failed to start: {}", result.error());
        return EXIT_FAILURE;
    }

    app.run(); // Blocking call

    if (const auto result = app.stop(); result.isError()) {
        LOG_ERROR("Shutdown error: {}", result.error());
        return EXIT_FAILURE;
    }

    LOG_INFO("Stopped");
    return EXIT_SUCCESS;
}
