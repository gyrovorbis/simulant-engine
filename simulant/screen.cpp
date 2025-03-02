#include <chrono>

#include "window.h"
#include "screen.h"

#include "threads/future.h"

namespace smlt {


void Screen::render(const uint8_t *data) {
    assert(format_ == SCREEN_FORMAT_G1);

    std::copy(data, data + buffer_.size(), &buffer_[0]);

    time_till_next_refresh_ = 0.0f;
}

void Screen::update(float dt) {
    time_till_next_refresh_ -= dt;
    if(time_till_next_refresh_ <= 0.0f) {
        time_till_next_refresh_ = smlt::fast_divide(1.0f, refresh_rate_);

        /* Now if this blocks, it doesn't matter. The main thread
         * can continue to update buffer_ without waiting */
        window_->render_screen(this, &buffer_[0], row_stride_);
    }
}


Screen::Screen(Window *window, const std::string &name, uint16_t w, uint16_t h, ScreenFormat format, uint16_t refresh):
    window_(window),
    name_(name),
    width_(w),
    height_(h),
    format_(format),
    refresh_rate_(refresh) {

    buffer_.resize((w * h) / 8, 0);
}

std::string Screen::name() const {
    return name_;
}

}
