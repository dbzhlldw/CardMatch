// GameScene — 主场景 View：cocos 节点、动画、触摸
#pragma once
#include "cocos2d.h"
#include <unordered_map>
#include <memory>
#include "presenter/GamePresenter.h"
#include "presenter/ViewTypes.h"

class CardView;

class GameScene : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();
    bool init() override;
    CREATE_FUNC(GameScene);

    using Layout = GamePresenter::Layout;

private:
    std::unique_ptr<GamePresenter> _presenter;

    std::unordered_map<int, CardView*> _cardViewMap;
    std::unordered_map<int, CardPile>  _cardPileById;

    ViewState _viewState;
    int       _reserveTopCardId = -1;

    bool _inputLocked       = false;
    int  _touchTargetCardId = -1;

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

    void onTableauTapped(int cardId);
    void onReserveTapped();
    void onUndoTapped();
    void onRestartTapped();

    void applyViewState(const ViewState& state, bool animate, int flyingCardId = -1);
    void syncPileCache(const ViewState& state);
    void updateRestartButtonVisibility();

    void handlePresenterOutcome(const PresenterOutcome& outcome);

    int hitTestTableauCardId(const cocos2d::Vec2& scenePos) const;
    int hitTestReserveTopCardId(const cocos2d::Vec2& scenePos) const;
    CardView* viewForCardId(int cardId) const;

    void lockInput();
    void unlockInput();
    void playRejectShake(int cardId);
    void playBlockedFeedback(int cardId, const std::vector<int>& blockerIds);
    void playBlockerHighlight(int cardId);
};
