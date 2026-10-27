#pragma once

#include <Geode/Geode.hpp>

#include <cstddef>
#include <string>

#include "DecoCore.hpp"

namespace autodeco {

struct GenerateResult {
    bool ok = false;
    std::size_t created = 0;
    std::string message;
};

struct RemoveResult {
    std::size_t removed = 0;
    std::size_t skipped = 0;  // generated objects left alone because they are selected
    std::string message;
};

// All functions must be called on the main (cocos) thread, which is where button callbacks run.
class EditorBridge {
public:
    static GenerateResult generate(LevelEditorLayer* layer, Settings const& settings);
    static GenerateResult regenerate(LevelEditorLayer* layer, Settings const& settings);
    static RemoveResult removeGenerated(LevelEditorLayer* layer);
};

}  // namespace autodeco
