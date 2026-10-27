#pragma once

#include <Geode/Geode.hpp>

#include <string>

#include "DecoCore.hpp"

namespace autodeco {

class DecoPopup : public geode::Popup {
public:
    static DecoPopup* create(LevelEditorLayer* layer);

protected:
    bool init(LevelEditorLayer* layer);

    void onStyle(cocos2d::CCObject* sender);
    void onDensity(cocos2d::CCObject* sender);
    void onPalette(cocos2d::CCObject* sender);
    void onSeed(cocos2d::CCObject* sender);
    void onModule(cocos2d::CCObject* sender);
    void onGenerate(cocos2d::CCObject* sender);
    void onRegenerate(cocos2d::CCObject* sender);
    void onRemove(cocos2d::CCObject* sender);

    void persist();
    void setStatus(std::string const& text);

    LevelEditorLayer* m_targetLayer = nullptr;
    Settings m_settings;

    cocos2d::CCLabelBMFont* m_styleLabel = nullptr;
    cocos2d::CCLabelBMFont* m_densityLabel = nullptr;
    cocos2d::CCLabelBMFont* m_paletteLabel = nullptr;
    cocos2d::CCLabelBMFont* m_seedLabel = nullptr;
    cocos2d::CCLabelBMFont* m_statusLabel = nullptr;
};

}  // namespace autodeco
