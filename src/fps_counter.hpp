#pragma once
#include<vector>
#include <glib.h>
#include <string>
class FpsCounter {
public:
    explicit FpsCounter(int window_size = 30);
    void tick();
    double get_fps() const;
    std::string fps_string() const;
    guint64 total_frames() const { return total_frames_; }

private:
    int     window_size_;
    double  fps_;
    guint64 total_frames_;
    std::vector<gint64> timestamps_;
    int                 head_;      
    int                 count_;    
};
