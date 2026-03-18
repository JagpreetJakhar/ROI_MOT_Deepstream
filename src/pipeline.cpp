#include "pipeline.hpp"
#include "config.hpp"

#include <gst/gst.h>
#include <cstdlib>
#include <cerrno>
#include <cstring>


gboolean bus_call(GstBus * , GstMessage *msg, gpointer data)
{
    auto *loop = static_cast<GMainLoop *>(data);

    switch (GST_MESSAGE_TYPE(msg)) {
        case GST_MESSAGE_EOS:
            g_print("[INFO] End of stream.\n");
            g_main_loop_quit(loop);
            break;

        case GST_MESSAGE_ERROR: {
            GError *err   = nullptr;
            gchar  *debug = nullptr;
            gst_message_parse_error(msg, &err, &debug);
            g_printerr("[ERROR] '%s': %s\n",
                       GST_OBJECT_NAME(GST_MESSAGE_SRC(msg)), err->message);
            g_printerr("[DEBUG] %s\n", debug ? debug : "none");
            g_error_free(err);
            g_free(debug);
            g_main_loop_quit(loop);
            break;
        }

        case GST_MESSAGE_WARNING: {
            GError *err   = nullptr;
            gchar  *debug = nullptr;
            gst_message_parse_warning(msg, &err, &debug);
            g_printerr("[WARNING] '%s': %s\n",
                       GST_OBJECT_NAME(GST_MESSAGE_SRC(msg)), err->message);
            g_printerr("[DEBUG]   %s\n", debug ? debug : "none");
            g_error_free(err);
            g_free(debug);
            break;
        }

        default:
            break;
    }
    return TRUE;
}

Pipeline::Pipeline(const char *input_path, const char *output_path)
    : input_path_(input_path), output_path_(output_path)
{}

Pipeline::~Pipeline()
{
    if (pipeline_) {
        gst_element_set_state(pipeline_, GST_STATE_NULL);
        gst_object_unref(GST_OBJECT(pipeline_));
    }
}

GstElement *Pipeline::make_element(const char *factory, const char *name)
{
    GstElement *el = gst_element_factory_make(factory, name);
    if (!el)
        g_printerr("[ERROR] Could not create '%s' (factory: '%s').\n",
                   name, factory);
    return el;
}
void Pipeline::on_pad_added(GstElement * , GstPad *new_pad, gpointer user_data)
{
    auto *streammux = static_cast<GstElement *>(user_data);

    GstCaps *caps = gst_pad_get_current_caps(new_pad);
    if (!caps)
        caps = gst_pad_query_caps(new_pad, nullptr);
    if (!caps) {
        g_printerr("[ERROR] on_pad_added: could not get caps for new pad\n");
        return;
    }

    GstStructure *st   = gst_caps_get_structure(caps, 0);
    const gchar  *name = gst_structure_get_name(st);
    g_print("[INFO] nvurisrcbin pad caps: %s\n", name);

    if (!g_str_has_prefix(name, "video/")) {
        gst_caps_unref(caps);
        return;
    }
    gst_caps_unref(caps);
    GstPad *mux_sink = gst_element_get_static_pad(streammux, "sink_0");
    if (!mux_sink)
        mux_sink = gst_element_request_pad_simple(streammux, "sink_0");

    if (!mux_sink) {
        g_printerr("[ERROR] Could not get nvstreammux sink_0 pad\n");
        return;
    }

    if (gst_pad_is_linked(mux_sink)) {
        g_print("[INFO] nvstreammux sink_0 already linked, skipping\n");
        gst_object_unref(mux_sink);
        return;
    }

    if (gst_pad_link(new_pad, mux_sink) == GST_PAD_LINK_OK)
        g_print("[INFO] Linked nvurisrcbin → nvstreammux\n");
    else
        g_printerr("[ERROR] nvurisrcbin → nvstreammux link failed\n");

    gst_object_unref(mux_sink);
}

bool Pipeline::build(ProbeContext *probe_ctx)
{
    pipeline_    = gst_pipeline_new("deepstream-pipeline");
    urisrcbin_   = make_element("nvurisrcbin",    "uri-src-bin");  
    streammux_   = make_element("nvstreammux",    "stream-muxer");
    pgie_        = make_element("nvinfer",        "primary-infer");
    tracker_     = make_element("nvtracker",      "tracker");
    nvvidconv_   = make_element("nvvideoconvert", "nvvid-converter");
    nvosd_       = make_element("nvdsosd",        "onscreendisplay");
    encoder_     = make_element("nvv4l2h264enc",  "h264-encoder");
    parser_out_  = make_element("h264parse",      "h264-parser-out");
    muxer_       = make_element("qtmux",          "muxer");
    sink_        = make_element("filesink",       "filesink");

    if (!pipeline_ || !urisrcbin_  || !streammux_ || !pgie_      || !tracker_ ||
        !nvvidconv_|| !nvosd_      || !encoder_   || !parser_out_|| !muxer_   || !sink_)
    {
        g_printerr("[FATAL] One or more elements could not be created.\n");
        return false;
    }

    gchar *abs_path = realpath(input_path_, nullptr);
    if (!abs_path) {
        g_printerr("[FATAL] Could not resolve input path '%s': %s\n",
                   input_path_, strerror(errno));
        return false;
    }
    gchar *uri = g_strdup_printf("file://%s", abs_path);
    free(abs_path);
    g_print("[INFO] URI: %s\n", uri);
    g_object_set(G_OBJECT(urisrcbin_),
                 "uri",             uri,
                 "gpu-id",          0,
                 "cudadec-memtype", 0,   
                 NULL);
    g_free(uri);

    g_object_set(G_OBJECT(streammux_),
                 "width",            FRAME_WIDTH,
                 "height",           FRAME_HEIGHT,
                 "batch-size",       1,
                 "live-source",      0,
                 "nvbuf-memory-type", 0,
                 "gpu-id",           0,
                 NULL);
    g_object_set(G_OBJECT(pgie_),    "config-file-path", PGIE_CONFIG_PATH, NULL);
    g_object_set(G_OBJECT(tracker_),
                 "tracker-width",  TRACKER_WIDTH,
                 "tracker-height", TRACKER_HEIGHT,
                 "ll-lib-file",    TRACKER_LIB_PATH,
                 "ll-config-file", TRACKER_CONFIG_PATH,
                 NULL);
    g_object_set(G_OBJECT(encoder_),
                 "bitrate", ENCODER_BITRATE,
                 NULL);
    g_object_set(G_OBJECT(sink_),
                 "location", output_path_,
                 "sync",     FALSE,
                 NULL);

    gst_bin_add_many(GST_BIN(pipeline_),
                     urisrcbin_, streammux_,
                     pgie_, tracker_, nvvidconv_, nvosd_,
                     encoder_, parser_out_, muxer_, sink_, NULL);

    g_signal_connect(urisrcbin_, "pad-added",
                     G_CALLBACK(Pipeline::on_pad_added),
                     static_cast<gpointer>(streammux_));

    if (!gst_element_link_many(streammux_, pgie_, tracker_,
                               nvvidconv_, nvosd_,
                               encoder_, parser_out_, muxer_, sink_, NULL)) {
        g_printerr("[FATAL] streammux chain link failed\n"); return false;
    }

    GstPad *osd_pad = gst_element_get_static_pad(nvosd_, "sink");
    gst_pad_add_probe(osd_pad,
                      GST_PAD_PROBE_TYPE_BUFFER,
                      osd_sink_pad_buffer_probe,
                      static_cast<gpointer>(probe_ctx),
                      nullptr);
    gst_object_unref(osd_pad);

    return true;
}

bool Pipeline::play()
{
    if (gst_element_set_state(pipeline_, GST_STATE_PLAYING)
        == GST_STATE_CHANGE_FAILURE) {
        g_printerr("[FATAL] Cannot set pipeline to PLAYING\n");
        return false;
    }
    return true;
}

void Pipeline::stop()
{
    if (pipeline_)
        gst_element_set_state(pipeline_, GST_STATE_NULL);
}

guint Pipeline::attach_bus_watch(GMainLoop *loop)
{
    GstBus *bus        = gst_pipeline_get_bus(GST_PIPELINE(pipeline_));
    guint   watch_id   = gst_bus_add_watch(bus, bus_call, loop);
    gst_object_unref(bus);
    return watch_id;
}
