#pragma once

#include <gst/gst.h>
#include "probe.hpp"
class Pipeline {
public:
    Pipeline(const char *input_path, const char *output_path);
    ~Pipeline();
    bool build(ProbeContext *probe_ctx);
    bool play();
    void stop();
    guint attach_bus_watch(GMainLoop *loop);

private:
    static GstElement *make_element(const char *factory, const char *name);
    static void on_pad_added(GstElement *demux, GstPad *new_pad, gpointer user_data);

    const char *input_path_;
    const char *output_path_;

    GstElement *pipeline_   = nullptr;
    GstElement *urisrcbin_  = nullptr;  
    GstElement *streammux_  = nullptr;
    GstElement *pgie_       = nullptr;
    GstElement *tracker_    = nullptr;
    GstElement *nvvidconv_  = nullptr;
    GstElement *nvosd_      = nullptr;
    GstElement *encoder_    = nullptr;
    GstElement *parser_out_ = nullptr;
    GstElement *muxer_      = nullptr;
    GstElement *sink_       = nullptr;
};
gboolean bus_call(GstBus *bus, GstMessage *msg, gpointer data);
