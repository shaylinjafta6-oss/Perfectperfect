#pragma once

// Pure C++ (no Geode / Cocos2d includes) so it can be unit tested on any machine.

#include <cstddef>
#include <cstdint>
#include <vector>

namespace autodeco {

enum class Style : int { Modern = 0, Neon, Tech, Hell, Minimal };
enum class Density : int { Low = 0, Medium, High, Dense };
enum class Palette : int { Auto = 0, Ocean, Sunset, Toxic, Crimson, Mono };

inline constexpr int kStyleCount = 5;
inline constexpr int kDensityCount = 4;
inline constexpr int kPaletteCount = 6;
inline constexpr int kModuleCount = 5;

struct Modules {
    bool blocks = true;
    bool air = true;
    bool structures = true;
    bool background = false;  // off by default: never touch the user's background unasked
    bool pulses = false;
};

struct Settings {
    Style style = Style::Modern;
    Density density = Density::Medium;
    Palette palette = Palette::Auto;
    std::uint32_t seed = 12345;
    Modules modules;
};

struct Point {
    float x = 0.f;
    float y = 0.f;
};

struct Bounds {
    float minX = 0.f;
    float minY = 0.f;
    float maxX = 0.f;
    float maxY = 0.f;
    bool valid = false;

    void include(float x, float y);
};

char const* styleName(Style s);
char const* densityName(Density d);
char const* paletteName(Palette p);

// Out-of-range values (for example from a corrupted save) fall back to the default.
Style styleFromInt(int value);
Density densityFromInt(int value);
Palette paletteFromInt(int value);

Style nextStyle(Style s);
Density nextDensity(Density d);
Palette nextPalette(Palette p);

char const* moduleName(int index);
bool getModule(Modules const& m, int index);
void setModule(Modules& m, int index, bool value);

// Stage 2 "pipeline probe": a row of marker positions above the top of the layout.
// Deterministic: the same bounds, density and seed always give the same points.
std::vector<Point> planProbeMarkers(Bounds const& bounds, Density density, std::uint32_t seed);

}  // namespace autodeco
