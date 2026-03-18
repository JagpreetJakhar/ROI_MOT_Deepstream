#include "dwell.hpp"
#include <gst/gst.h>
gchar *format_dwell(guint64 ns)
{
    guint64 total_sec = ns / G_GUINT64_CONSTANT(1000000000);
    guint   mins      = static_cast<guint>(total_sec / 60);
    guint   secs      = static_cast<guint>(total_sec % 60);
    return g_strdup_printf("%02u:%02u", mins, secs);
}

void DwellTracker::update(guint64 tid, bool in_roi,
                          guint64 current_pts, bool pts_valid)
{
    auto it = map_.find(tid);

    if (in_roi) {
        if (it == map_.end()) {
            DwellEntry e;
            e.in_roi       = true;
            e.dwell_ns     = 0;
            e.entry_pts_ns = pts_valid ? current_pts : 0;
            map_[tid]      = e;

        } else if (!it->second.in_roi) {
            it->second.in_roi       = true;
            it->second.entry_pts_ns = pts_valid ? current_pts : 0;

        }
    } else {
        if (it != map_.end() && it->second.in_roi) {
            if (pts_valid && current_pts >= it->second.entry_pts_ns)
                it->second.dwell_ns += current_pts - it->second.entry_pts_ns;
            it->second.in_roi = false;
        }
    }
}

void DwellTracker::finalize(guint64 tid, guint64 current_pts, bool pts_valid)
{
    auto it = map_.find(tid);
    if (it == map_.end() || !it->second.in_roi) return;

    if (pts_valid && current_pts >= it->second.entry_pts_ns)
        it->second.dwell_ns += current_pts - it->second.entry_pts_ns;
    it->second.in_roi = false;


}

guint64 DwellTracker::get_display_dwell(guint64 tid,
                                        guint64 current_pts,
                                        bool    pts_valid) const
{
    auto it = map_.find(tid);
    if (it == map_.end()) return 0;

    guint64 d = it->second.dwell_ns;
    if (it->second.in_roi && pts_valid && current_pts >= it->second.entry_pts_ns)
        d += current_pts - it->second.entry_pts_ns;
    return d;
}

void DwellTracker::print_summary() const
{
    g_print("\n=== DWELL TIME SUMMARY ===\n");
    for (const auto &kv : map_) {
        gchar *t = format_dwell(kv.second.dwell_ns);
        g_print("  Vehicle ID %-6lu  total dwell: %s\n",
                static_cast<unsigned long>(kv.first), t);
        g_free(t);
    }
    g_print("==========================\n\n");
}
