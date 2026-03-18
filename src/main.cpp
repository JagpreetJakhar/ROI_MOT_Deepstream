#include <gst/gst.h>
#include <glib.h>
#include <iostream>
#include <fstream>
#include <string>
#include "config.hpp"
#include "pipeline.hpp"
#include "probe.hpp"
#include "nlohmann/json.hpp"

static std::string basename_of(const std::string &path)
{
    size_t pos = path.find_last_of("/\\");
    return (pos == std::string::npos) ? path : path.substr(pos + 1);
}


static std::vector<Point2f> load_roi(const std::string &config_path,
                                     const std::string &filename)
{
    std::ifstream f(config_path);
    if (!f.is_open()) {
        g_printerr("[ERROR] Cannot open ROI config: %s\n", config_path.c_str());
        return {};
    }

    nlohmann::json j;
    try {
        f >> j;
    } catch (const nlohmann::json::parse_error &e) {
        g_printerr("[ERROR] JSON parse error in %s: %s\n",
                   config_path.c_str(), e.what());
        return {};
    }

    if (j.contains(filename)) {
        std::vector<Point2f> poly;
        for (auto &pt : j[filename])
            poly.push_back({pt[0].get<float>(), pt[1].get<float>()});
        g_print("[INFO] ROI loaded for '%s' from %s\n",
                filename.c_str(), config_path.c_str());
        return poly;
    }

    g_printerr("[WARN] No ROI entry for '%s' in %s — using first entry as default.\n",
               filename.c_str(), config_path.c_str());
    if (!j.empty()) {
        auto it = j.begin();
        std::vector<Point2f> poly;
        for (auto &pt : it.value())
            poly.push_back({pt[0].get<float>(), pt[1].get<float>()});
        g_print("[INFO] Using ROI for key '%s'\n", it.key().c_str());
        return poly;
    }

    g_printerr("[ERROR] ROI config is empty.\n");
    return {};
}

int main(int argc, char *argv[])
{
    const char *input_path   = (argc > 1) ? argv[1] : "../input/input.mp4";
    const char *output_path  = (argc > 2) ? argv[2] : "../output/result.mp4";
    const char *roi_cfg_path = (argc > 3) ? argv[3] : ROI_CONFIG_PATH;

    g_print("[INFO] Input      : %s\n", input_path);
    g_print("[INFO] Output     : %s\n", output_path);
    g_print("[INFO] ROI config : %s\n", roi_cfg_path);

    gst_init(&argc, &argv);

    std::string fname = basename_of(input_path);
    std::vector<Point2f> roi = load_roi(roi_cfg_path, fname);

    if (roi.size() < 3) {
        g_printerr("[FATAL] ROI must have at least 3 vertices (got %zu).\n",
                   roi.size());
        return EXIT_FAILURE;
    }

    g_print("[INFO] ROI vertices (%zu points):\n", roi.size());
    for (size_t i = 0; i < roi.size(); ++i)
        g_print("  [%zu] (%.0f, %.0f)\n", i, roi[i].x, roi[i].y);

    g_print("[INFO] Dwell time  : guint64 nanoseconds (integer, no drift)\n");
    g_print("[INFO] FPS display : %d-frame sliding-window average\n", 30);
    g_print("[INFO] Display fmt : MM:SS\n\n");

    ProbeContext probe_ctx;
    probe_ctx.roi_polygon = std::move(roi);

    Pipeline pipeline(input_path, output_path);
    GMainLoop *loop = g_main_loop_new(nullptr, FALSE);

    if (!pipeline.build(&probe_ctx)) return EXIT_FAILURE;

    guint bus_watch_id = pipeline.attach_bus_watch(loop);

    if (!pipeline.play()) return EXIT_FAILURE;

    g_print("[INFO] Pipeline running.  Press Ctrl+C to stop.\n");
    g_main_loop_run(loop);
    pipeline.stop();

    probe_ctx.dwell_tracker.print_summary();

    g_source_remove(bus_watch_id);
    g_main_loop_unref(loop);

    g_print("[INFO] Done. Total frames processed: %" G_GUINT64_FORMAT "\n",
            probe_ctx.fps_counter.total_frames());

    return EXIT_SUCCESS;
}
