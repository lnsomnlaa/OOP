#include "Solid3D.h"
#include <array>
#include <cmath>

POINT Projection3D::map(double x, double y, double z) {
    const double k = 0.45;
    POINT p;
    p.x = static_cast<LONG>(x + k * z + 0.5);
    p.y = static_cast<LONG>(y - k * z + 0.5);
    return p;
}

CubeSolid3D::CubeSolid3D(double centerX, double centerY, double centerZ, double edgeLength) {
    cx = centerX;
    cy = centerY;
    cz = centerZ;
    half = edgeLength > 0 ? edgeLength * 0.5 : 0.5;
}

std::vector<std::pair<POINT, POINT>> CubeSolid3D::projectedWireframe() const {
    std::array<double, 3> corners[8];
    int i = 0;
    for (int dx : {-1, 1}) {
        for (int dy : {-1, 1}) {
            for (int dz : {-1, 1}) {
                corners[i][0] = cx + dx * half;
                corners[i][1] = cy + dy * half;
                corners[i][2] = cz + dz * half;
                ++i;
            }
        }
    }
    POINT pv[8];
    for (i = 0; i < 8; ++i) {
        pv[i] = Projection3D::map(corners[i][0], corners[i][1], corners[i][2]);
    }
    static const int ed[12][2] = {{0, 1}, {2, 3}, {4, 5}, {6, 7}, {0, 2}, {1, 3}, {4, 6}, {5, 7},
                                  {0, 4}, {1, 5}, {2, 6}, {3, 7}};
    std::vector<std::pair<POINT, POINT>> out;
    out.reserve(12);
    for (const auto& e : ed) {
        out.push_back({pv[e[0]], pv[e[1]]});
    }
    return out;
}

namespace {

constexpr int kSeg = 20;

void appendCircle(std::vector<std::pair<POINT, POINT>>& out, double cx, double cy, double cz, double r,
                  int plane) {
    POINT first{};
    POINT prev{};
    for (int i = 0; i < kSeg; ++i) {
        const double t = i * (6.28318530717958647692 / kSeg);
        double px = cx, py = cy, pz = cz;
        if (plane == 0) {
            px += r * std::cos(t);
            py += r * std::sin(t);
        } else if (plane == 1) {
            px += r * std::cos(t);
            pz += r * std::sin(t);
        } else {
            py += r * std::cos(t);
            pz += r * std::sin(t);
        }
        const POINT cur = Projection3D::map(px, py, pz);
        if (i == 0) {
            first = cur;
            prev = cur;
        } else {
            out.push_back({prev, cur});
            prev = cur;
        }
    }
    out.push_back({prev, first});
}

} // namespace

SphereSolid3D::SphereSolid3D(double centerX, double centerY, double centerZ, double rad) {
    cx = centerX;
    cy = centerY;
    cz = centerZ;
    radius = rad > 0 ? rad : 1;
}

std::vector<std::pair<POINT, POINT>> SphereSolid3D::projectedWireframe() const {
    std::vector<std::pair<POINT, POINT>> out;
    appendCircle(out, cx, cy, cz, radius, 0);
    appendCircle(out, cx, cy, cz, radius, 1);
    appendCircle(out, cx, cy, cz, radius, 2);
    return out;
}

TetraSolid3D::TetraSolid3D(double centerX, double centerY, double centerZ, double edgeLength) {
    cx = centerX;
    cy = centerY;
    cz = centerZ;
    edge = edgeLength > 0 ? edgeLength : 1;
}

std::vector<std::pair<POINT, POINT>> TetraSolid3D::projectedWireframe() const {
    const double k = edge / std::sqrt(8.0);
    const std::array<double, 3> raw[4] = {{k, k, k}, {k, -k, -k}, {-k, k, -k}, {-k, -k, k}};
    POINT pv[4];
    for (int i = 0; i < 4; ++i) {
        pv[i] = Projection3D::map(cx + raw[i][0], cy + raw[i][1], cz + raw[i][2]);
    }
    static const int ed[6][2] = {{0, 1}, {0, 2}, {0, 3}, {1, 2}, {1, 3}, {2, 3}};
    std::vector<std::pair<POINT, POINT>> out;
    out.reserve(6);
    for (const auto& e : ed) {
        out.push_back({pv[e[0]], pv[e[1]]});
    }
    return out;
}
