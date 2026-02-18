#pragma once

#include <memory>
#include <string>

#include "api/ITransport.h"
#include "common/types/Result.h"

namespace httplib {
    class Server;
}

namespace service::api {
    class IRequestHandler;

    class RestTransport final : public ITransport {
    public:
        explicit RestTransport(IRequestHandler& request_handler);
        ~RestTransport() override;

        Result<void> start(const std::string& server_address) override;
        Result<void> stop() override;
        Result<void> runLoop() override;

    private:
        IRequestHandler& request_handler_;
        std::unique_ptr<httplib::Server> server_;
        std::string server_address_;
        bool is_running_{false};

        void setupRoutes() const;
    };
} // namespace service::api
