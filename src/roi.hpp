#pragma once
#include "config.hpp"
#include <vector>
// Test whether point (px, py) is inside the given polygon.
// Uses the ray-casting algorithm
bool point_in_polygon(float px, float py, const std::vector<Point2f> &poly);
