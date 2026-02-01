#pragma once

namespace service::infrastructure {
    class VideoChannel {
    public:
        explicit VideoChannel(int channel_num);
        ~VideoChannel() noexcept;
    };
} // namespace service::infrastructure