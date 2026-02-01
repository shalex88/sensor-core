#pragma once
#include "common/types/Result.h"

namespace service::infrastructure {
    class ICameraHw {
    public:
        virtual ~ICameraHw() = default;

        virtual Result<void> open() = 0;
        virtual Result<void> close() = 0;
    };
} // namespace service::infrastructure