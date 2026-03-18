#pragma once
#include <gstnvdsmeta.h>
#include <nvdsmeta.h>
#include <glib.h>
#include <vector>
#include "config.hpp"

void draw_roi_overlay(NvDsBatchMeta *batch_meta, NvDsFrameMeta *frame_meta,
                      const std::vector<Point2f> &roi_polygon);

void draw_hud(NvDsBatchMeta *batch_meta, NvDsFrameMeta *frame_meta,
              int total, int in_roi_count, double fps);

void draw_dwell_label(NvDsBatchMeta     *batch_meta,
                      NvDsFrameMeta     *frame_meta,
                      NvDsObjectMeta    *obj,
                      guint64            tid,
                      guint64            display_dwell_ns,
                      bool               in_roi);
