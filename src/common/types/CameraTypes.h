#pragma once

#include <cstdint>
#include <string>

namespace service::common::types {
    using zoom = uint32_t;
    using focus = uint32_t;
    using info = std::string;

    inline constexpr zoom MIN_NORMALIZED_ZOOM = 0;
    inline constexpr zoom MAX_NORMALIZED_ZOOM = 100;

    inline constexpr focus MIN_NORMALIZED_FOCUS = 0;
    inline constexpr focus MAX_NORMALIZED_FOCUS = 100;

    struct ZoomRange {
        zoom min;
        zoom max;
    };

    struct FocusRange {
        focus min;
        focus max;
    };
} // namespace service::common::types
