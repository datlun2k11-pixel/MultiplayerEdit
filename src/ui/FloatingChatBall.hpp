#pragma once
#include <Geode/Geode.hpp>

namespace mpedit {

    class FloatingChatBall : public cocos2d::CCLayer {
    public:
        static FloatingChatBall* create();
        bool init() override;
        void registerWithTouchDispatcher() override;
        bool ccTouchBegan(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;
        void ccTouchMoved(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;
        void ccTouchEnded(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;
        void ccTouchCancelled(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;
        void update(float dt) override;
        void onExit() override;
    private:
        void clampPosition();
        void updateOpacity(bool immediate = false);
        void savePosition();
        void loadPosition();

        cocos2d::CCDrawNode* m_bg = nullptr;
        cocos2d::CCDrawNode* m_border = nullptr;
        cocos2d::CCSprite* m_icon = nullptr;
        cocos2d::CCNode* m_root = nullptr;

        bool m_dragging = false;
        bool m_touching = false;
        cocos2d::CCPoint m_touchOffset{0.f, 0.f};
        cocos2d::CCPoint m_startPos{0.f, 0.f};
        float m_idleTime = 0.f;
        float m_dragDist = 0.f;
        float m_radius = 22.f;
    };

}
