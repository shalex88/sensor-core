#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <string>

#include <microhttpd.h>

#include "api/ITransport.h"
#include "common/types/Result.h"

namespace service::api {
    class IRequestHandler;

    class RestTransport final : public ITransport {
    public:
        explicit RestTransport(IRequestHandler& request_handler);
        ~RestTransport() override;

        Result<void> start(const std::string& server, uint16_t port) override;
        Result<void> stop() override;
        Result<void> runLoop() override;

    private:
        static MHD_Result accessHandler(void* cls,
                                         MHD_Connection* connection,
                                         const char* url,
                                         const char* method,
                                         const char* version,
                                         const char* upload_data,
                                         size_t* upload_data_size,
                                         void** con_cls);

        static void requestCompleted(void* cls,
                                      MHD_Connection* connection,
                                      void** con_cls,
                                      MHD_RequestTerminationCode toe);

        MHD_Result handleRequest(MHD_Connection* connection,
                                  const std::string& method,
                                  const std::string& path,
                                  const std::string& body) const;

        IRequestHandler& request_handler_;
        std::string host_{};
        uint16_t port_{};
        std::atomic<bool> is_running_{false};
        std::mutex run_mutex_{};
        std::condition_variable run_cv_{};
    };
} // namespace service::api
