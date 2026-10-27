#include <Geode/Geode.hpp>
#include <Geode/modify/EditorPauseLayer.hpp>

#include "DecoPopup.hpp"

using namespace geode::prelude;

// Adds an "Auto-Deco" button to the editor's pause menu.
class $modify(AutoDecoPauseLayer, EditorPauseLayer) {
    void customSetup() {
        EditorPauseLayer::customSetup();

        auto menu = CCMenu::create();
        menu->setPosition(CCPointZero);  // CCMenu defaults to the screen centre

        auto sprite = ButtonSprite::create("Auto-Deco");
        sprite->setScale(0.7f);
        auto button = CCMenuItemSpriteExtra::create(
            sprite, this, menu_selector(AutoDecoPauseLayer::onAutoDecorate)
        );

        auto const winSize = CCDirector::sharedDirector()->getWinSize();
        button->setPosition(ccp(winSize.width - 62.f, 28.f));
        menu->addChild(button);
        this->addChild(menu);
    }

    void onAutoDecorate(CCObject*) {
        if (auto popup = autodeco::DecoPopup::create(m_editorLayer)) {
            popup->show();
        }
    }
};
