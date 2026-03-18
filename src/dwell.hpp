#pragma once
#include <glib.h>
#include <unordered_map>
struct DwellEntry {
    guint64 entry_pts_ns; 
    guint64 dwell_ns;      
    bool    in_roi;        
};
class DwellTracker {
public:

    void update(guint64 tid, bool in_roi, guint64 current_pts, bool pts_valid);
    void finalize(guint64 tid, guint64 current_pts, bool pts_valid);
    guint64 get_display_dwell(guint64 tid, guint64 current_pts, bool pts_valid) const;
    void print_summary() const;
    const std::unordered_map<guint64, DwellEntry> &entries() const { return map_; }

private:
    std::unordered_map<guint64, DwellEntry> map_;
};
gchar *format_dwell(guint64 ns);
