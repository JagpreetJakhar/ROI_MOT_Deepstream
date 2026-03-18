#include "probe.hpp"
#include "config.hpp"
#include "roi.hpp"
#include "overlay.hpp"
#include "dwell.hpp"
#include <gstnvdsmeta.h>
#include <nvdsmeta.h>
#include <gst/gst.h>
#include <algorithm>
#include <vector>
GstPadProbeReturn osd_sink_pad_buffer_probe(GstPad * ,
                                            GstPadProbeInfo *info,
                                            gpointer         data)
{
    auto *ctx = static_cast<ProbeContext *>(data);
    ctx->fps_counter.tick();
    GstBuffer     *buf        = static_cast<GstBuffer *>(info->data);
    NvDsBatchMeta *batch_meta = gst_buffer_get_nvds_batch_meta(buf);
    if (!batch_meta) return GST_PAD_PROBE_OK;
    
    guint64 current_pts = GST_BUFFER_PTS(buf);
    bool    pts_valid   = GST_CLOCK_TIME_IS_VALID(current_pts);
    double  fps         = ctx->fps_counter.get_fps();

    for (NvDsMetaList *l_frame = batch_meta->frame_meta_list;
         l_frame != nullptr; l_frame = l_frame->next)
    {
        auto *frame_meta = static_cast<NvDsFrameMeta *>(l_frame->data);
        
        draw_roi_overlay(batch_meta, frame_meta, ctx->roi_polygon);

        int total_vehicles = 0;
        int roi_vehicles   = 0;

        std::vector<guint64> alive_ids;

        for (NvDsMetaList *l_obj = frame_meta->obj_meta_list;
             l_obj != nullptr; l_obj = l_obj->next)
        {
            auto *obj = static_cast<NvDsObjectMeta *>(l_obj->data);
            bool is_vehicle = std::find(VEHICLE_CLASSES.begin(),
                                        VEHICLE_CLASSES.end(),
                                        obj->class_id) != VEHICLE_CLASSES.end();
            if (!is_vehicle) {
                obj->rect_params.border_width = 0;
                obj->text_params.display_text = nullptr;
                continue;
            }

            ++total_vehicles;
            guint64 tid = obj->object_id;
            alive_ids.push_back(tid);

            float cx = obj->rect_params.left + obj->rect_params.width  * 0.5f;
            float cy = obj->rect_params.top  + obj->rect_params.height * 0.5f;
            bool  in_roi = point_in_polygon(cx, cy, ctx->roi_polygon);

            if (in_roi) ++roi_vehicles;
            ctx->dwell_tracker.update(tid, in_roi, current_pts, pts_valid);
            guint64 display_dwell = ctx->dwell_tracker.get_display_dwell(
                                        tid, current_pts, pts_valid);
            obj->rect_params.has_bg_color = 0;
            if (in_roi) {
                obj->rect_params.border_color = COLOR_BOX_IN_ROI;
                obj->rect_params.border_width = BOX_WIDTH_IN_ROI;
                obj->rect_params.has_bg_color = 1;
                obj->rect_params.bg_color     = {1.0, 0.0, 0.0, 0.12};
            } else {
                obj->rect_params.border_color = COLOR_BOX_DEFAULT;
                obj->rect_params.border_width = BOX_WIDTH_DEFAULT;
            }
            if (in_roi || display_dwell > 0) {
                draw_dwell_label(batch_meta, frame_meta, obj,
                                 tid, display_dwell, in_roi);
            }
            if (obj->text_params.display_text) {
                obj->text_params.font_params.font_color =
                    in_roi ? COLOR_TEXT_RED : COLOR_TEXT_WHITE;
                obj->text_params.set_bg_clr  = 1;
                obj->text_params.text_bg_clr = COLOR_BG_DARK;
            }
        }
        for (const auto &kv : ctx->dwell_tracker.entries()) {
            guint64 tid   = kv.first;
            bool    alive = std::find(alive_ids.begin(), alive_ids.end(), tid)
                            != alive_ids.end();
            if (!alive)
                ctx->dwell_tracker.finalize(tid, current_pts, pts_valid);
        }
        draw_hud(batch_meta, frame_meta, total_vehicles, roi_vehicles, fps);
    }
    return GST_PAD_PROBE_OK;
}
