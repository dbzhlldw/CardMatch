#include "GameScene.h"
#include "CardView.h"
#include "model/GameModel.h"
#include "model/CardModel.h"
#include "controller/GameController.h"
#include "controller/command/MatchCommand.h"
#include "controller/command/DrawCommand.h"
#include <algorithm>

USING_NS_CC;

const float GameScene::ANIM_DURATION = 0.25f;

Scene* GameScene::createScene() {
    return GameScene::create();
}

bool GameScene::init() {
    if (!Scene::init()) return false;

    _model      = GameModel::getInstance();
    _controller = GameController::getInstance();

    _model->setupGame();
    _controller->init();

    // 固定区域位置（分辨率 1080×2080，底部 580px 为手牌区）
    _handPilePos = Vec2(300, 290);
    _reservePos  = Vec2(780, 290);

    setupHandPile();
    setupReserve();
    setupTableau();
    setupUndoButton();

    return true;
}

// ---------------------------------------------------------------------------
// 初始布局
// ---------------------------------------------------------------------------

void GameScene::setupHandPile() {
    for (auto card : _model->getHand()) {
        auto view = CardView::create(card);
        view->setAnchorPoint(Vec2(0.5f, 0.5f));
        view->setPosition(_handPilePos);
        addChild(view, 10);
        _handViewStack.push_back(view);
    }
    refreshHandPileDisplay();

    auto label = Label::createWithSystemFont("手牌堆", "Arial", 36);
    label->setPosition(_handPilePos + Vec2(0, -CardView::CARD_SIZE.height / 2 - 30));
    addChild(label, 20);
}

void GameScene::setupReserve() {
    // 备用牌堆
    // 底部→顶部依次创建，轻微错位形成叠牌效果
    const auto& reserve = _model->getReserve();
    for (int i = 0; i < (int)reserve.size(); i++) {
        auto view = CardView::create(reserve[i]);
        view->setAnchorPoint(Vec2(0.5f, 0.5f));
        view->setPosition(_reservePos + Vec2(i * 2.f, i * 2.f));
        addChild(view, 10 + i);
        _reserveViewStack.push_back(view);
    }
    // 只有顶部牌可以点击
    if (!_reserveViewStack.empty()) {
        _reserveViewStack.back()->setClickCallback(
            [this](CardView* v){ onReserveClicked(v); });
    }
    refreshReserveDisplay();

    auto label = Label::createWithSystemFont("备用牌", "Arial", 36);
    label->setPosition(_reservePos + Vec2(0, -CardView::CARD_SIZE.height / 2 - 30));
    addChild(label, 20);
}

void GameScene::setupTableau() {
    const auto& tableau = _model->getTableau();
    int   count   = (int)tableau.size();
    float spacing = 280.f;
    float totalW  = spacing * (count - 1);
    float startX  = (1080.f - totalW) / 2.f;
    float y       = 1350.f;

    for (int i = 0; i < count; i++) {
        auto view = CardView::create(tableau[i]);
        view->setAnchorPoint(Vec2(0.5f, 0.5f));
        view->setPosition(startX + i * spacing, y);
        view->setClickCallback([this](CardView* v){ onTableauCardClicked(v); });
        addChild(view, 5);
        _tableauViews.push_back(view);
    }
}

void GameScene::setupUndoButton() {
    auto btn = Label::createWithSystemFont("[撤回]", "Arial", 60);
    btn->setColor(Color3B(255, 200, 50));
    btn->setPosition(Vec2(540, 100));
    addChild(btn, 20);

    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [btn](Touch* t, Event*) -> bool {
        return btn->getBoundingBox().containsPoint(t->getLocation());
    };
    listener->onTouchEnded = [this, btn](Touch* t, Event*) {
        if (btn->getBoundingBox().containsPoint(t->getLocation()))
            onUndoClicked();
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, btn);
}

// ---------------------------------------------------------------------------
// 用户交互
// ---------------------------------------------------------------------------

void GameScene::onTableauCardClicked(CardView* view) {
    CardView* prevHandTop = _handViewStack.empty() ? nullptr : _handViewStack.back();
    Vec2      returnPos   = view->getPosition();

    MatchCommand* cmd = _controller->tryMatch(view->getModel());
    if (!cmd) return;

    _animHistory.push_back({ view, returnPos, true, cmd->getTableauIndex(), prevHandTop });

    _tableauViews.erase(
        std::remove(_tableauViews.begin(), _tableauViews.end(), view),
        _tableauViews.end());

    view->setLocalZOrder(50);
    view->runAction(MoveTo::create(ANIM_DURATION, _handPilePos));

    _handViewStack.push_back(view);
    refreshHandPileDisplay();
}

void GameScene::onReserveClicked(CardView* view) {
    CardView* prevHandTop = _handViewStack.empty() ? nullptr : _handViewStack.back();
    Vec2      returnPos   = view->getPosition();

    DrawCommand* cmd = _controller->tryDraw();
    if (!cmd) return;

    _animHistory.push_back({ view, returnPos, false, -1, prevHandTop });

    _reserveViewStack.erase(
        std::remove(_reserveViewStack.begin(), _reserveViewStack.end(), view),
        _reserveViewStack.end());

    // 让下一张备用牌可点击
    if (!_reserveViewStack.empty()) {
        _reserveViewStack.back()->setClickCallback(
            [this](CardView* v){ onReserveClicked(v); });
    }

    view->setFaceUp(true);
    view->setLocalZOrder(50);
    view->runAction(MoveTo::create(ANIM_DURATION, _handPilePos));

    _handViewStack.push_back(view);
    refreshHandPileDisplay();
}

void GameScene::onUndoClicked() {
    if (_animHistory.empty() || !_controller->canUndo()) return;

    MoveRecord record = _animHistory.back();
    _animHistory.pop_back();

    _controller->tryUndo();

    _handViewStack.pop_back();

    CardView* view = record.view;

    if (record.returnToTableau) {
        view->setFaceUp(true);
        view->setLocalZOrder(5);
        view->runAction(MoveTo::create(ANIM_DURATION, record.returnPos));

        int idx = record.tableauIndex;
        if (idx >= (int)_tableauViews.size())
            _tableauViews.push_back(view);
        else
            _tableauViews.insert(_tableauViews.begin() + idx, view);

        view->setClickCallback([this](CardView* v){ onTableauCardClicked(v); });
    } else {
        view->setFaceUp(false);
        view->setLocalZOrder(10 + (int)_reserveViewStack.size());
        view->runAction(MoveTo::create(ANIM_DURATION, record.returnPos));

        _reserveViewStack.push_back(view);
        view->setClickCallback([this](CardView* v){ onReserveClicked(v); });
    }

    refreshHandPileDisplay();
    refreshReserveDisplay();
}

// ---------------------------------------------------------------------------
// 刷新显示
// ---------------------------------------------------------------------------

void GameScene::refreshHandPileDisplay() {
    for (int i = 0; i < (int)_handViewStack.size(); i++) {
        _handViewStack[i]->setVisible(i == (int)_handViewStack.size() - 1);
    }
}

void GameScene::refreshReserveDisplay() {
    for (int i = 0; i < (int)_reserveViewStack.size(); i++) {
        _reserveViewStack[i]->setVisible(i == (int)_reserveViewStack.size() - 1);
    }
}
