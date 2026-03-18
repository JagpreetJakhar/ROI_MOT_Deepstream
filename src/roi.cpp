#include "roi.hpp"
bool point_in_polygon(float px, float py, const std::vector<Point2f> &poly)
{
    int  n      = static_cast<int>(poly.size());
    bool inside = false;

    for (int i = 0, j = n - 1; i < n; j = i++) {
        float xi = poly[i].x, yi = poly[i].y;
        float xj = poly[j].x, yj = poly[j].y;

        bool cross = ((yi > py) != (yj > py)) &&
                     (px < (xj - xi) * (py - yi) / (yj - yi) + xi);
        if (cross) inside = !inside;
    }
    return inside;
}
