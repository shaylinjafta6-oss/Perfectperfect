#include "EditorBridge.hpp"

#include <unordered_set>
#include <utility>
#include <vector>

using namespace geode::prelude;

namespace autodeco {

namespace {

// Object ID 914 is the Text object (a non-solid decoration). Confirmed by two third-party
// GD libraries (DashTools, gdlib), not by official docs. Stage 2 only uses it for test markers.
constexpr int kTextObjectID = 914;

// Generated objects are tracked here for the current editor session.
// Ref<> keeps each tracked object alive, so its address can never be reused by a different
// (original) object. That is what makes "is this pointer one of ours?" a safe question.
// The vector is deliberately leaked so nothing is destroyed during game shutdown.
std::vector<Ref<GameObject>>& registry() {
    static auto* r = new std::vector<Ref<GameObject>>();
    return *r;
}

LevelEditorLayer*& registryOwner() {
    static LevelEditorLayer* owner = nullptr;  // only compared, never dereferenced
    return owner;
}

void syncOwner(LevelEditorLayer* layer) {
    if (registryOwner() != layer) {
        registry().clear();
        registryOwner() = layer;
    }
}

std::unordered_set<GameObject*> collectLive(LevelEditorLayer* layer) {
    std::unordered_set<GameObject*> live;
    CCArray* all = layer->getAllObjects();
    if (!all) return live;
    for (unsigned int i = 0; i < all->count(); ++i) {
        auto* obj = static_cast<GameObject*>(all->objectAtIndex(i));
        if (obj) live.insert(obj);
    }
    return live;
}

std::unordered_set<GameObject*> trackedSet() {
    std::unordered_set<GameObject*> tracked;
    for (auto& ref : registry()) {
        if (ref.data()) tracked.insert(ref.data());
    }
    return tracked;
}

}  // namespace

GenerateResult EditorBridge::generate(LevelEditorLayer* layer, Settings const& settings) {
    GenerateResult result;
    if (!layer) {
        result.message = "The editor is not available.";
        return result;
    }
    syncOwner(layer);

    // 1. Scan: bounds of the ORIGINAL objects (anything we generated earlier is ignored).
    auto const tracked = trackedSet();
    Bounds bounds;
    for (GameObject* obj : collectLive(layer)) {
        if (tracked.contains(obj)) continue;
        CCPoint const pos = obj->getPosition();
        bounds.include(pos.x, pos.y);
    }
    if (!bounds.valid) {
        result.message = "Nothing to decorate: the level has no objects.";
        return result;
    }

    // 2. Plan (pure C++, see DecoCore.cpp).
    auto const points = planProbeMarkers(bounds, settings.density, settings.seed);

    // 3. Create real editor objects and remember them.
    for (auto const& p : points) {
        GameObject* obj = layer->createObject(kTextObjectID, ccp(p.x, p.y), true);
        if (!obj) continue;
        registry().emplace_back(obj);
        ++result.created;
    }

    log::info("AutoDecorator: placed {} test markers (seed {})", result.created, settings.seed);

    result.ok = result.created > 0;
    result.message = result.ok
        ? "Placed " + std::to_string(result.created) + " test markers above the layout."
        : "No objects could be created.";
    return result;
}

RemoveResult EditorBridge::removeGenerated(LevelEditorLayer* layer) {
    RemoveResult result;
    if (!layer) {
        result.message = "The editor is not available.";
        return result;
    }
    syncOwner(layer);

    if (registry().empty()) {
        result.message = "Nothing to remove: no generated objects tracked in this editor session.";
        return result;
    }

    auto const live = collectLive(layer);
    EditorUI* ui = layer->m_editorUI;
    auto isSelected = [ui](GameObject* obj) {
        if (!ui) return false;
        if (ui->m_selectedObject == obj) return true;
        return ui->m_selectedObjects && ui->m_selectedObjects->containsObject(obj);
    };

    std::vector<Ref<GameObject>> keep;
    for (auto& ref : registry()) {
        GameObject* obj = ref.data();
        // Not in the level any more (the user deleted it themselves): just forget it.
        if (!obj || !live.contains(obj)) continue;
        if (isSelected(obj)) {
            keep.push_back(ref);
            ++result.skipped;
            continue;
        }
        layer->removeObject(obj, true);
        ++result.removed;
    }
    registry().swap(keep);

    result.message = "Removed " + std::to_string(result.removed) + " generated objects.";
    if (result.skipped > 0) {
        result.message += " " + std::to_string(result.skipped) + " selected ones were kept: deselect and try again.";
    }
    return result;
}

GenerateResult EditorBridge::regenerate(LevelEditorLayer* layer, Settings const& settings) {
    RemoveResult const removed = removeGenerated(layer);
    if (removed.skipped > 0) {
        GenerateResult cancelled;
        cancelled.message = "Regenerate cancelled: " + std::to_string(removed.skipped) +
                            " generated objects are selected. Deselect them first.";
        return cancelled;
    }

    GenerateResult result = generate(layer, settings);
    if (result.ok) {
        result.message = "Replaced " + std::to_string(removed.removed) + " markers with " +
                         std::to_string(result.created) + " new ones.";
    }
    return result;
}

}  // namespace autodeco
