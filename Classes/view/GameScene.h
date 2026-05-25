#pragma once
#include "cocos2d.h"
#include <vector>
#include <unordered_map>
#include "data/LayoutDef.h"

class CardView;
class CardModel;
class GameModel;
class GameController;

// 主场景：View 完全由 GameModel 驱动，每次操作后重建牌堆归属并重算布局
class GameScene : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();
    bool init() override;
    CREATE_FUNC(GameScene);

    static const float DESIGN_WIDTH;
    static const float DESIGN_HEIGHT;
    static const float PILE_AREA_H;
    static const float TABLEAU_AREA_H;

private:
    GameModel*      _model      = nullptr;
    GameController* _controller = nullptr;

    LayoutDef _layout;

    // CardModel* → CardView*（全生命周期不变）
    std::unordered_map<CardModel*, CardView*> _cardViewMap;
    // 发牌时槽位索引，用于桌面牌定位（不随 remove/insert 改变）
    std::unordered_map<CardModel*, int> _cardSlotIndex;

    // 由 rebuildViewStacksFromModel() 从 Model 重建，不做手动增删
    std::vector<CardView*> _tableauViews;
    std::vector<CardView*> _handViewStack;
    std::vector<CardView*> _reserveViewStack;

    cocos2d::Vec2 _handPilePos;
    cocos2d::Vec2 _reservePos;

    bool      _inputLocked   = false;
    CardView* _touchTarget   = nullptr;

    static const float ANIM_DURATION;
    static const float RESERVE_STACK_OFFSET;
    static const int   FLYING_Z_ORDER;

    void createAllCardViews();
    void setupUndoButton();
    void setupSceneTouch();

    void onTableauCardClicked(CardView* view);
    void onReserveClicked();
    void onUndoClicked();

    void rebuildViewStacksFromModel();
    void applyAllLayouts(bool animate, CardView* flyingView = nullptr);
    void syncAllInteractable();

    cocos2d::Vec2 handPositionFor(int index) const;
    cocos2d::Vec2 reservePositionFor(int index, int total) const;
    cocos2d::Vec2 slotPositionFor(CardModel* card) const;
    int           slotZOrderFor(CardModel* card) const;

    CardView* hitTestTableau(const cocos2d::Vec2& scenePos) const;
    CardView* hitTestReserveTop(const cocos2d::Vec2& scenePos) const;

    void lockInput();
    void unlockInput();
};
