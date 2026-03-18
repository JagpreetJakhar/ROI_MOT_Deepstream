#include "overlay.hpp"
#include "config.hpp"
#include "dwell.hpp"  
#include <algorithm>   


void draw_roi_overlay(NvDsBatchMeta *batch_meta, NvDsFrameMeta *frame_meta,
                      const std::vector<Point2f> &roi_polygon)
{
    NvDsDisplayMeta *dm = nvds_acquire_display_meta_from_pool(batch_meta);
    dm->num_lines  = 0;
    dm->num_labels = 0;

    int n = static_cast<int>(roi_polygon.size());
    for (int i = 0; i < n; ++i) {
        if (dm->num_lines >= MAX_ELEMENTS_IN_DISPLAY_META) {
            nvds_add_display_meta_to_frame(frame_meta, dm);
            dm = nvds_acquire_display_meta_from_pool(batch_meta);
            dm->num_lines  = 0;
            dm->num_labels = 0;
        }
        int j = (i + 1) % n;
        NvOSD_LineParams &lp = dm->line_params[dm->num_lines++];
        lp.x1         = static_cast<int>(roi_polygon[i].x);
        lp.y1         = static_cast<int>(roi_polygon[i].y);
        lp.x2         = static_cast<int>(roi_polygon[j].x);
        lp.y2         = static_cast<int>(roi_polygon[j].y);
        lp.line_width = ROI_LINE_WIDTH;
        lp.line_color = COLOR_ROI_OUTLINE;
    }
    if (dm->num_labels < MAX_ELEMENTS_IN_DISPLAY_META) {
        NvOSD_TextParams &tp      = dm->text_params[dm->num_labels++];
        tp.display_text           = g_strdup("ROI");
        tp.x_offset               = static_cast<int>(roi_polygon[0].x);
        tp.y_offset               = static_cast<int>(roi_polygon[0].y) - 32;
        tp.font_params.font_name  = const_cast<char *>("Serif");
        tp.font_params.font_size  = 20;
        tp.font_params.font_color = COLOR_ROI_OUTLINE;
        tp.set_bg_clr             = 1;
        tp.text_bg_clr            = COLOR_BG_DARK;
    }

    nvds_add_display_meta_to_frame(frame_meta, dm);
}

void draw_hud(NvDsBatchMeta *batch_meta, NvDsFrameMeta *frame_meta,
              int total, int in_roi_count, double fps)
{
    NvDsDisplayMeta *dm = nvds_acquire_display_meta_from_pool(batch_meta);
    dm->num_labels = 0;

    auto add_label = [&](const char *text, int y, NvOSD_ColorParams fc) {
        if (dm->num_labels >= MAX_ELEMENTS_IN_DISPLAY_META) return;
        NvOSD_TextParams &tp      = dm->text_params[dm->num_labels++];
        tp.display_text           = g_strdup(text);
        tp.x_offset               = 20;
        tp.y_offset               = y;
        tp.font_params.font_name  = const_cast<char *>("Serif");
        tp.font_params.font_size  = 26;
        tp.font_params.font_color = fc;
        tp.set_bg_clr             = 1;
        tp.text_bg_clr            = COLOR_BG_DARK;
    };

    gchar *s1 = g_strdup_printf("Vehicles : %d", total);
    gchar *s2 = g_strdup_printf("In ROI   : %d", in_roi_count);
    gchar *s3 = g_strdup_printf("FPS      : %.1f", fps);

    add_label(s1, 20,  COLOR_TEXT_WHITE);
    add_label(s2, 62,  COLOR_TEXT_RED);
    add_label(s3, 104, COLOR_TEXT_GREEN);

    g_free(s1); g_free(s2); g_free(s3);

    nvds_add_display_meta_to_frame(frame_meta, dm);
}
void draw_dwell_label(NvDsBatchMeta  *batch_meta,
                      NvDsFrameMeta  *frame_meta,
                      NvDsObjectMeta *obj,
                      guint64         tid,
                      guint64         display_dwell_ns,
                      bool            in_roi)
{
    NvDsDisplayMeta *tdm = nvds_acquire_display_meta_from_pool(batch_meta);
    tdm->num_labels = 0;

    gchar *dwell_str = format_dwell(display_dwell_ns);

    NvOSD_TextParams &tp      = tdm->text_params[tdm->num_labels++];
    tp.display_text           = g_strdup_printf("ID:%lu %s",
                                    static_cast<unsigned long>(tid), dwell_str);
    tp.x_offset               = static_cast<int>(obj->rect_params.left);
    tp.y_offset               = std::max(0, static_cast<int>(obj->rect_params.top) - 36);
    tp.font_params.font_name  = const_cast<char *>("Serif");
    tp.font_params.font_size  = in_roi ? 20 : 16;
    tp.font_params.font_color = in_roi ? COLOR_TEXT_RED : COLOR_TEXT_WHITE;
    tp.set_bg_clr             = 1;
    tp.text_bg_clr            = COLOR_BG_DARK;

    g_free(dwell_str);

    nvds_add_display_meta_to_frame(frame_meta, tdm);
}
