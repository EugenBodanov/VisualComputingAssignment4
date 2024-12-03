#pragma once

#include "base.h"
#include "camera.h"

#include <vector>

struct DebugVertex
{
    Vector3D position;
    Vector3D color;
};

void debugDrawPoints(const std::vector<DebugVertex>& points);
void debugDrawLines(const std::vector<DebugVertex>& lines);
void debugDrawTriangles(const std::vector<DebugVertex>& triangles);

void debugInit();
void debugDraw(const Camera& camera);
void debugShutdown();
