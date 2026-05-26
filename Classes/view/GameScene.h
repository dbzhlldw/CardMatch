// GameScene — 主场景 View：cocos 节点、动画、触摸；布局与交互决策交给 GamePresenter
#pragma once
#include "cocos2d.h"
#include <vector>
#include <unordered_map>
#include <memory>
#include "presenter/GamePresenter.h"

class CardView;
class CardModel;

class GameScene : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();
    bool init() override;
    CREATE_FUNC(GameScene);

    using Layout = GamePresenter::Layout;

private:
    std::unique_ptr<GamePresenter> _presenter;

    std::unordered_map<CardModel*, CardView*> _cardViewMap;

    std::vector<CardView*> _tableauViews;
    std::vector<CardView*> _handViewStack;
    std::vector<CardView*> _reserveViewStack;

    bool      _inputLocked = false;
    CardView* _touchTarget = nullptr;

    cocos2d::Node* _restartButton   = nullptr;
    float          _restartHitHalfW = 0.f;
    float          _restartHitHalfH = 0.f;

    static const float ANIM_DURATION;
    static const int   FLYING_Z_ORDER;

    void createAllCardViews();
    void destroyAllCardViews();
    void setupPileButtons();
    void setupRestartButton();
    void setupSceneTouch();

    void onTableauCardClicked(CardView* view);
    void onReserveClicked();
    void onUndoClicked();
    void onRestartClicked();

    void rebuildViewStacksFromModel();
    void applyAllLayouts(bool animate, CardView* flyingView = nullptr);
    void syncAllInteractable();
    void updateRestartButtonVisibility();
    void refreshViewAfterAction(const PresenterOutcome& outcome);

    CardView* hitTestTableau(const cocos2d::Vec2& scenePos) const;
    CardView* hitTestReserveTop(const cocos2d::Vec2& scenePos) const;
    CardView* viewForCard(CardModel* card) const;

    void lockInput();
    void unlockInput();
};
