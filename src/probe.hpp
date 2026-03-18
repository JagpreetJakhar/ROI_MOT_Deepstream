#pragma once
#include <gst/gst.h>
#include "config.hpp"
#include "dwell.hpp"
#include "fps_counter.hpp"
struct ProbeContext {
    std::vector<Point2f> roi_polygon;   
    DwellTracker         dwell_tracker;
    FpsCounter           fps_counter{30};  // 30-frame sliding window
};
GstPadProbeReturn osd_sink_pad_buffer_probe(GstPad          *pad,
                                            GstPadProbeInfo *info,
                                            gpointer         data);
