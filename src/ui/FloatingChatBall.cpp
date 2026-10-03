#include "FloatingChatBall.hpp"
#include "QuickChatPopup.hpp"
#include "../SessionManager.hpp"
#include <Geode/binding/LevelEditorLayer.hpp>

using namespace geode::prelude;

namespace mpedit {

    FloatingChatBall* FloatingChatBall::create() {
        auto* ret = new FloatingChatBall();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    bool FloatingChatBall::init() {
        if (!CCLayer::init()) return false;

        m_root = CCNode::create();
        m_root->setContentSize({m_radius * 2, m_radius * 2});
        m_root->setAnchorPoint({0.5f, 0.5f});
        this->addChild(m_root);

        m_bg = CCDrawNode::create();
        m_bg->drawDot({0,0}, m_radius, {0.f, 0.f, 0.f, 0.55f});
        m_bg->setPosition({m_radius, m_radius});
        m_root->addChild(m_bg, -1);

        m_border = CCDrawNode::create();
        m_border->drawCircle({0,0}, m_radius - 1.f, {0,0,0,0}, 1.5f, {1.f, 1.f, 1.f, 0.9f}, 32);
        m_border->setPosition({m_radius, m_radius});
        m_root->addChild(m_border, 0);

        CCSprite* icon = nullptr;
        if (auto* s = CCSprite::createWithSpriteFrameName("GJ_chatBtn_001.png")) {
            icon = s;
        } else if (auto* s = CCSprite::createWithSpriteFrameName("gj_chatBtn_001.png")) {
            icon = s;
        } else if (auto* s = CCSprite::createWithSpriteFrameName("GJ_commentBtn_001.png")) {
            icon = s;
        }
        if (icon) {
            icon->setScale(0.55f);
            icon->setPosition({m_radius, m_radius + 1.f});
            m_root->addChild(icon, 1);
            m_icon = icon;
        } else {
            auto* lbl = CCLabelBMFont::create("Chat", "bigFont.fnt");
            lbl->setScale(0.35f);
            lbl->setPosition({m_radius, m_radius});
            m_root->addChild(lbl, 1);
        }

        this->setContentSize({m_radius * 2, m_radius * 2});
        this->setAnchorPoint({0.5f, 0.5f});

        loadPosition();

        this->setTouchEnabled(true);

        this->scheduleUpdate();
        updateOpacity(true);

        this->setID("floating-chat-ball"_spr);
        m_root->setID("ball-root"_spr);

        return true;
    }

    void FloatingChatBall::registerWithTouchDispatcher() {
        CCTouchDispatcher::get()->addTargetedDelegate(this, -499, true);
    }

    void FloatingChatBall::onExit() {
        CCTouchDispatcher::get()->removeDelegate(this);
        CCLayer::onExit();
    }

    void FloatingChatBall::loadPosition() {
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        float defX = winSize.width - 70.f;
        float defY = winSize.height - 95.f;
        float x = Mod::get()->getSavedValue<float>("floating-ball-x", defX);
        float y = Mod::get()->getSavedValue<float>("floating-ball-y", defY);
        this->setPosition({x, y});
        clampPosition();
    }

    void FloatingChatBall::savePosition() {
        auto pos = this->getPosition();
        Mod::get()->setSavedValue("floating-ball-x", pos.x);
        Mod::get()->setSavedValue("floating-ball-y", pos.y);
    }

    void FloatingChatBall::clampPosition() {
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        auto pos = this->getPosition();
        pos.x = std::clamp(pos.x, m_radius + 4.f, winSize.width - m_radius - 4.f);
        pos.y = std::clamp(pos.y, m_radius + 4.f, winSize.height - m_radius - 4.f);
        this->setPosition(pos);
    }

    void FloatingChatBall::updateOpacity(bool immediate) {
        float targetOpacity = m_touching ? 255.f : 110.f;
        if (!m_touching && m_idleTime < 0.7f) targetOpacity = 200.f;

        if (immediate) {
            if (m_bg) m_bg->setOpacity(static_cast<GLubyte>(m_touching ? 140 : 85));
            if (m_border) m_border->setOpacity(static_cast<GLubyte>(targetOpacity));
            if (m_icon) m_icon->setOpacity(static_cast<GLubyte>(targetOpacity));
        } else {
            float cur = m_border ? m_border->getOpacity() : targetOpacity;
            float next = cur + (targetOpacity - cur) * 0.18f;
            if (std::abs(next - targetOpacity) < 1.f) next = targetOpacity;
            GLubyte b = static_cast<GLubyte>(next);
            GLubyte bgO = static_cast<GLubyte>(m_touching ? 140 : 70 + (next/255.f)*40.f);
            if (m_bg) m_bg->setOpacity(bgO);
            if (m_border) m_border->setOpacity(b);
            if (m_icon) m_icon->setOpacity(b);
        }
    }

    bool FloatingChatBall::ccTouchBegan(CCTouch* touch, CCEvent* event) {
        if (!this->isVisible()) return false;
        if (auto* editor = LevelEditorLayer::get()) {
            if (editor->m_playbackMode != PlaybackMode::Not) return false;
        }
        auto loc = this->convertToNodeSpace(touch->getLocation());
        CCRect rect{0, 0, m_radius*2, m_radius*2};
        rect.origin.x -= 4; rect.origin.y -= 4;
        rect.size.width += 8; rect.size.height += 8;
        if (!rect.containsPoint(loc)) return false;

        m_touching = true;
        m_dragging = false;
        m_dragDist = 0.f;
        m_startPos = this->getPosition();
        m_touchOffset = touch->getLocation() - this->getPosition();
        m_idleTime = 0.f;
        updateOpacity(true);
        m_root->stopAllActions();
        m_root->runAction(CCScaleTo::create(0.08f, 1.08f));
        return true;
    }

    void FloatingChatBall::ccTouchMoved(CCTouch* touch, CCEvent* event) {
        if (!m_touching) return;
        auto cur = touch->getLocation();
        auto newPos = cur - m_touchOffset;
        auto oldPos = this->getPosition();
        float dx = newPos.x - oldPos.x;
        float dy = newPos.y - oldPos.y;
        m_dragDist += std::sqrt(dx*dx + dy*dy);
        if (m_dragDist > 6.f) m_dragging = true;
        this->setPosition(newPos);
        clampPosition();
    }

    void FloatingChatBall::ccTouchEnded(CCTouch* touch, CCEvent* event) {
        if (!m_touching) return;
        m_touching = false;
        m_root->stopAllActions();
        m_root->runAction(CCScaleTo::create(0.12f, 1.f));
        m_idleTime = 0.f;

        bool wasDrag = m_dragging && m_dragDist > 8.f;
        if (wasDrag) {
            savePosition();
        } else {
            if (m_dragDist > 2.f) {
                savePosition();
            }
            auto& session = SessionManager::get();
            if (!session.isInSession()) {
                Notification::create("Not in a multiplayer session", NotificationIcon::Info)->show();
            } else {
                if (auto* popup = QuickChatPopup::create()) {
                    popup->show();
                }
            }
        }
        m_dragging = false;
        updateOpacity(true);
    }

    void FloatingChatBall::ccTouchCancelled(CCTouch* touch, CCEvent* event) {
        ccTouchEnded(touch, event);
    }

    void FloatingChatBall::update(float dt) {
        if (auto* editor = LevelEditorLayer::get()) {
            bool isPlaytesting = editor->m_playbackMode != PlaybackMode::Not;
            if (isPlaytesting) {
                if (this->isVisible()) this->setVisible(false);
                return;
            } else {
                if (!this->isVisible()) this->setVisible(true);
            }
        }
        if (!m_touching) {
            m_idleTime += dt;
            updateOpacity(false);
        }
        clampPosition();
    }

}
