#include "DecoPopup.hpp"

#include <algorithm>
#include <chrono>
#include <random>

#include "EditorBridge.hpp"

using namespace geode::prelude;

namespace autodeco {

namespace {

constexpr char const* kKeyStyle = "ad-style";
constexpr char const* kKeyDensity = "ad-density";
constexpr char const* kKeyPalette = "ad-palette";
constexpr char const* kKeySeed = "ad-seed";
constexpr char const* kKeyModulePrefix = "ad-module-";

Settings loadSettings() {
    Mod* mod = Mod::get();
    Settings s;
    s.style = styleFromInt(mod->getSavedValue<int>(kKeyStyle, 0));
    s.density = densityFromInt(mod->getSavedValue<int>(kKeyDensity, 1));
    s.palette = paletteFromInt(mod->getSavedValue<int>(kKeyPalette, 0));
    s.seed = static_cast<std::uint32_t>(std::max(1, mod->getSavedValue<int>(kKeySeed, 12345)));
    for (int i = 0; i < kModuleCount; ++i) {
        std::string const key = std::string(kKeyModulePrefix) + std::to_string(i);
        setModule(s.modules, i, mod->getSavedValue<bool>(key, getModule(s.modules, i)));
    }
    return s;
}

void saveSettings(Settings const& s) {
    Mod* mod = Mod::get();
    mod->setSavedValue<int>(kKeyStyle, static_cast<int>(s.style));
    mod->setSavedValue<int>(kKeyDensity, static_cast<int>(s.density));
    mod->setSavedValue<int>(kKeyPalette, static_cast<int>(s.palette));
    mod->setSavedValue<int>(kKeySeed, static_cast<int>(s.seed));
    for (int i = 0; i < kModuleCount; ++i) {
        std::string const key = std::string(kKeyModulePrefix) + std::to_string(i);
        mod->setSavedValue<bool>(key, getModule(s.modules, i));
    }
}

}  // namespace

DecoPopup* DecoPopup::create(LevelEditorLayer* layer) {
    auto ret = new DecoPopup();
    if (ret->init(layer)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool DecoPopup::init(LevelEditorLayer* layer) {
    if (!Popup::init(400.f, 290.f)) return false;

    m_targetLayer = layer;
    m_settings = loadSettings();

    this->setTitle("Auto-Decorate");

    // A row = [tappable button on the left] [current value on the right].
    auto addCycleRow = [this](char const* caption, SEL_MenuHandler callback, float y, char const* value) {
        auto sprite = ButtonSprite::create(caption);
        sprite->setScale(0.7f);
        auto button = CCMenuItemSpriteExtra::create(sprite, this, callback);
        m_buttonMenu->addChildAtPosition(button, Anchor::Center, ccp(-105.f, y));

        auto label = CCLabelBMFont::create(value, "bigFont.fnt");
        label->setScale(0.55f);
        m_mainLayer->addChildAtPosition(label, Anchor::Center, ccp(45.f, y));
        return label;
    };

    m_styleLabel = addCycleRow("Style", menu_selector(DecoPopup::onStyle), 95.f, styleName(m_settings.style));
    m_densityLabel = addCycleRow("Density", menu_selector(DecoPopup::onDensity), 65.f, densityName(m_settings.density));
    m_paletteLabel = addCycleRow("Palette", menu_selector(DecoPopup::onPalette), 35.f, paletteName(m_settings.palette));
    m_seedLabel = addCycleRow(
        "Reroll", menu_selector(DecoPopup::onSeed), 5.f,
        ("Seed: " + std::to_string(m_settings.seed)).c_str()
    );

    // Module toggles: [checkbox] name
    auto addToggle = [this](int index, float centerX, float y) {
        auto toggler = CCMenuItemToggler::createWithStandardSprites(this, menu_selector(DecoPopup::onModule), 0.6f);
        toggler->setTag(index);
        toggler->toggle(getModule(m_settings.modules, index));
        m_buttonMenu->addChildAtPosition(toggler, Anchor::Center, ccp(centerX - 40.f, y));

        auto label = CCLabelBMFont::create(moduleName(index), "bigFont.fnt");
        label->setScale(0.4f);
        label->setAnchorPoint(ccp(0.f, 0.5f));
        m_mainLayer->addChildAtPosition(label, Anchor::Center, ccp(centerX - 22.f, y));
    };
    addToggle(0, -125.f, -25.f);
    addToggle(1, 0.f, -25.f);
    addToggle(2, 125.f, -25.f);
    addToggle(3, -60.f, -52.f);
    addToggle(4, 60.f, -52.f);

    auto note = CCLabelBMFont::create(
        "Style, palette and modules are saved now; they take effect from Stage 4.", "bigFont.fnt"
    );
    note->setScale(0.28f);
    m_mainLayer->addChildAtPosition(note, Anchor::Center, ccp(0.f, -75.f));

    m_statusLabel = CCLabelBMFont::create("Ready.", "bigFont.fnt");
    m_mainLayer->addChildAtPosition(m_statusLabel, Anchor::Center, ccp(0.f, -95.f));
    this->setStatus("Ready.");

    // Action buttons.
    auto addAction = [this](char const* caption, SEL_MenuHandler callback, float x) {
        auto sprite = ButtonSprite::create(caption);
        sprite->setScale(0.7f);
        auto button = CCMenuItemSpriteExtra::create(sprite, this, callback);
        m_buttonMenu->addChildAtPosition(button, Anchor::Center, ccp(x, -122.f));
    };
    addAction("Generate", menu_selector(DecoPopup::onGenerate), -125.f);
    addAction("Regenerate", menu_selector(DecoPopup::onRegenerate), 0.f);
    addAction("Remove", menu_selector(DecoPopup::onRemove), 125.f);

    return true;
}

void DecoPopup::persist() {
    saveSettings(m_settings);
}

void DecoPopup::setStatus(std::string const& text) {
    if (!m_statusLabel) return;
    m_statusLabel->setString(text.c_str());
    constexpr float kBaseScale = 0.4f;
    constexpr float kMaxWidth = 360.f;
    float const width = m_statusLabel->getContentSize().width * kBaseScale;
    m_statusLabel->setScale(width > kMaxWidth ? kBaseScale * kMaxWidth / width : kBaseScale);
}

void DecoPopup::onStyle(CCObject*) {
    m_settings.style = nextStyle(m_settings.style);
    m_styleLabel->setString(styleName(m_settings.style));
    this->persist();
}

void DecoPopup::onDensity(CCObject*) {
    m_settings.density = nextDensity(m_settings.density);
    m_densityLabel->setString(densityName(m_settings.density));
    this->persist();
}

void DecoPopup::onPalette(CCObject*) {
    m_settings.palette = nextPalette(m_settings.palette);
    m_paletteLabel->setString(paletteName(m_settings.palette));
    this->persist();
}

void DecoPopup::onSeed(CCObject*) {
    std::mt19937 rng(static_cast<std::uint32_t>(std::chrono::steady_clock::now().time_since_epoch().count()));
    m_settings.seed = rng() % 999999u + 1u;
    m_seedLabel->setString(("Seed: " + std::to_string(m_settings.seed)).c_str());
    this->persist();
}

void DecoPopup::onModule(CCObject* sender) {
    // The toggle's own on/off picture flips by itself; we mirror it in our settings.
    int const index = static_cast<CCNode*>(sender)->getTag();
    if (index < 0 || index >= kModuleCount) return;
    setModule(m_settings.modules, index, !getModule(m_settings.modules, index));
    this->persist();
}

void DecoPopup::onGenerate(CCObject*) {
    this->persist();
    this->setStatus(EditorBridge::generate(m_targetLayer, m_settings).message);
}

void DecoPopup::onRegenerate(CCObject*) {
    this->persist();
    this->setStatus(EditorBridge::regenerate(m_targetLayer, m_settings).message);
}

void DecoPopup::onRemove(CCObject*) {
    this->setStatus(EditorBridge::removeGenerated(m_targetLayer).message);
}

}  // namespace autodeco
