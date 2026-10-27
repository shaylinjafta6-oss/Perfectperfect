#include "DecoCore.hpp"

#include <algorithm>
#include <random>

namespace autodeco {

namespace {

constexpr char const* kStyleNames[kStyleCount] = {"Modern", "Neon", "Tech", "Hell", "Minimal"};
constexpr char const* kDensityNames[kDensityCount] = {"Low", "Medium", "High", "Dense"};
constexpr char const* kPaletteNames[kPaletteCount] = {"Auto", "Ocean", "Sunset", "Toxic", "Crimson", "Mono"};
constexpr char const* kModuleNames[kModuleCount] = {"Blocks", "Air", "Structures", "Background", "Pulses"};

constexpr std::size_t kMaxProbeMarkers = 150;
constexpr float kProbeGap = 90.f;  // distance above the highest object

float probeSpacing(Density d) {
    switch (d) {
        case Density::Low: return 600.f;
        case Density::Medium: return 300.f;
        case Density::High: return 150.f;
        case Density::Dense: return 90.f;
    }
    return 300.f;
}

}  // namespace

void Bounds::include(float x, float y) {
    if (!valid) {
        minX = maxX = x;
        minY = maxY = y;
        valid = true;
        return;
    }
    minX = std::min(minX, x);
    maxX = std::max(maxX, x);
    minY = std::min(minY, y);
    maxY = std::max(maxY, y);
}

char const* styleName(Style s) {
    int i = static_cast<int>(s);
    return (i >= 0 && i < kStyleCount) ? kStyleNames[i] : "?";
}

char const* densityName(Density d) {
    int i = static_cast<int>(d);
    return (i >= 0 && i < kDensityCount) ? kDensityNames[i] : "?";
}

char const* paletteName(Palette p) {
    int i = static_cast<int>(p);
    return (i >= 0 && i < kPaletteCount) ? kPaletteNames[i] : "?";
}

Style styleFromInt(int value) {
    return (value >= 0 && value < kStyleCount) ? static_cast<Style>(value) : Style::Modern;
}

Density densityFromInt(int value) {
    return (value >= 0 && value < kDensityCount) ? static_cast<Density>(value) : Density::Medium;
}

Palette paletteFromInt(int value) {
    return (value >= 0 && value < kPaletteCount) ? static_cast<Palette>(value) : Palette::Auto;
}

Style nextStyle(Style s) {
    return static_cast<Style>((static_cast<int>(s) + 1) % kStyleCount);
}

Density nextDensity(Density d) {
    return static_cast<Density>((static_cast<int>(d) + 1) % kDensityCount);
}

Palette nextPalette(Palette p) {
    return static_cast<Palette>((static_cast<int>(p) + 1) % kPaletteCount);
}

char const* moduleName(int index) {
    return (index >= 0 && index < kModuleCount) ? kModuleNames[index] : "?";
}

bool getModule(Modules const& m, int index) {
    switch (index) {
        case 0: return m.blocks;
        case 1: return m.air;
        case 2: return m.structures;
        case 3: return m.background;
        case 4: return m.pulses;
        default: return false;
    }
}

void setModule(Modules& m, int index, bool value) {
    switch (index) {
        case 0: m.blocks = value; break;
        case 1: m.air = value; break;
        case 2: m.structures = value; break;
        case 3: m.background = value; break;
        case 4: m.pulses = value; break;
        default: break;
    }
}

std::vector<Point> planProbeMarkers(Bounds const& bounds, Density density, std::uint32_t seed) {
    std::vector<Point> out;
    if (!bounds.valid) return out;

    float const width = std::max(0.f, bounds.maxX - bounds.minX);
    float const spacing = probeSpacing(density);

    std::size_t count = static_cast<std::size_t>(width / spacing) + 1;
    count = std::min(count, kMaxProbeMarkers);

    float const step = count > 1 ? width / static_cast<float>(count - 1) : 0.f;

    // mt19937 output is fully specified by the C++ standard, so this is deterministic.
    std::mt19937 rng(seed);
    out.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        float const jitter = static_cast<float>(static_cast<int>(rng() % 31u) - 15);
        out.push_back(Point{bounds.minX + step * static_cast<float>(i), bounds.maxY + kProbeGap + jitter});
    }
    return out;
}

}  // namespace autodeco
