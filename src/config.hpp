#pragma once
#include <gstnvdsmeta.h>
#include <vector>
static constexpr int FRAME_WIDTH  = 1920;
static constexpr int FRAME_HEIGHT = 1080;
static const std::vector<int> VEHICLE_CLASSES = {2, 5, 7};

struct Point2f { float x, y; };

static const NvOSD_ColorParams COLOR_BOX_DEFAULT  = {0.0, 1.0, 0.0, 1.0};
static const NvOSD_ColorParams COLOR_BOX_IN_ROI   = {1.0, 0.0, 0.0, 1.0}; 
static const NvOSD_ColorParams COLOR_ROI_OUTLINE  = {1.0, 1.0, 0.0, 1.0}; 
static const NvOSD_ColorParams COLOR_TEXT_WHITE   = {1.0, 1.0, 1.0, 1.0};
static const NvOSD_ColorParams COLOR_TEXT_RED     = {1.0, 0.3, 0.3, 1.0};
static const NvOSD_ColorParams COLOR_TEXT_GREEN   = {0.2, 1.0, 0.2, 1.0};
static const NvOSD_ColorParams COLOR_BG_DARK      = {0.0, 0.0, 0.0, 0.6};


static constexpr int BOX_WIDTH_DEFAULT = 2;
static constexpr int BOX_WIDTH_IN_ROI  = 4;
static constexpr int ROI_LINE_WIDTH    = 4;

static constexpr const char *PGIE_CONFIG_PATH =
    "../configs/config_infer_primary_yolo.txt";

static constexpr const char *TRACKER_LIB_PATH =
    "/opt/nvidia/deepstream/deepstream-8.0/lib/libnvds_nvmultiobjecttracker.so";

static constexpr const char *TRACKER_CONFIG_PATH =
    "../configs/config_tracker.txt";

static constexpr int TRACKER_WIDTH  = 640;
static constexpr int TRACKER_HEIGHT = 640;
static constexpr int ENCODER_BITRATE = 4000000;

static constexpr int FPS_UPDATE_INTERVAL_FRAMES = 30;

static constexpr const char *ROI_CONFIG_PATH = "../configs/roi_config.json";
