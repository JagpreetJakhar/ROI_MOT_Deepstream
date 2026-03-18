#include "fps_counter.hpp"
#include <ios>
#include <iomanip>
#include <sstream>

FpsCounter::FpsCounter(int window_size)
    : window_size_(window_size > 1 ? window_size : 2)
    , fps_(0.0)
    , total_frames_(0)
    , timestamps_(window_size_, 0)
    , head_(0)
    , count_(0)
{}

void FpsCounter::tick()
{
    gint64 now = g_get_monotonic_time(); // microseconds
    timestamps_[head_] = now;
    head_ = (head_ + 1) % window_size_;
    if (count_ < window_size_) ++count_;
    ++total_frames_;
    if (count_ < 2) {
        fps_ = 0.0;
        return;
    }
    int oldest_idx = (head_) % window_size_;   
                                               
                                               
    gint64 oldest_ts = timestamps_[oldest_idx];
    gint64 newest_ts = timestamps_[(head_ + window_size_ - 1) % window_size_];

    double elapsed_sec = static_cast<double>(newest_ts - oldest_ts) / 1e6;
    if (elapsed_sec > 0.0)
        fps_ = static_cast<double>(count_ - 1) / elapsed_sec;
}

double FpsCounter::get_fps() const
{
    return fps_;
}

std::string FpsCounter::fps_string() const
{
    std::ostringstream oss;
    oss << "FPS: " << std::fixed << std::setprecision(1) << fps_;
    return oss.str();
}
