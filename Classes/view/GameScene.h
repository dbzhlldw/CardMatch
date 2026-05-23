#pragma once
#include "cocos2d.h"
#include <vector>
#include <functional>

class CardView;
class CardModel;
class GameModel;
class GameController;

class GameScene : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();
    bool init() override;
    CREATE_FUNC(GameScene);

    // 设计分辨率与区域划分
    static const float DESIGN_WIDTH;
    static const float DESIGN_HEIGHT;
    static const float PILE_AREA_H;
    static const float TABLEAU_AREA_H;

private:
    GameModel*      _model      = nullptr;
    GameController* _controller = nullptr;

    // 桌面牌视图列表
    std::vector<CardView*> _tableauViews;

    // 手牌堆和备用牌堆各自的视图栈
    std::vector<CardView*> _handViewStack;
    std::vector<CardView*> _reserveViewStack;

    // 固定位置
    cocos2d::Vec2 _handPilePos;
    cocos2d::Vec2 _reservePos;

    // 动画记录，用于撤回
    struct MoveRecord {
        CardView*      view;
        cocos2d::Vec2  returnPos;    // 撤回时动画回到此处
        bool           returnToTableau;
        int            tableauIndex; // returnToTableau 为 true 时有效
        CardView*      prevHandTop;  // 此次移动前的手牌堆顶视图
    };
    std::vector<MoveRecord> _animHistory;

    static const float ANIM_DURATION;

    void setupLayout();
    void setupTableau();
    void setupHandPile();
    void setupReserve();
    void setupUndoButton();

    void onTableauCardClicked(CardView* view);
    void onReserveClicked(CardView* view);
    void onUndoClicked();

    // 让手牌堆顶视图可见，其余隐藏
    void refreshHandPileDisplay();
    // 让备用牌堆顶视图可见，其余隐藏
    void refreshReserveDisplay();
};
