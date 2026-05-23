#include "GameScene.h"
#include "CardView.h"
#include "model/GameModel.h"
#include "model/CardModel.h"
#include "controller/GameController.h"
#include "controller/command/MatchCommand.h"
#include "controller/command/DrawCommand.h"
#include <algorithm>

USING_NS_CC;

const float GameScene::DESIGN_WIDTH   = 1080.f;
const float GameScene::DESIGN_HEIGHT  = 2080.f;
const float GameScene::PILE_AREA_H    = 580.f;
const float GameScene::TABLEAU_AREA_H = GameScene::DESIGN_HEIGHT - GameScene::PILE_AREA_H; // 1500
const float GameScene::ANIM_DURATION  = 0.25f;

Scene* GameScene::createScene() {
    return GameScene::create();
}

bool GameScene::init() {
    if (!Scene::init()) return false;

    _model      = GameModel::getInstance();
    _controller = GameController::getInstance();

    _model->setupGame();
    _controller->init();

    // 堆牌区三列横排：手牌堆 | 备用牌 | 回退按钮
    float pileAreaCenterY = PILE_AREA_H / 2.f; // 290
    _handPilePos = Vec2(DESIGN_WIDTH * 0.22f, pileAreaCenterY);
    _reservePos  = Vec2(DESIGN_WIDTH * 0.50f, pileAreaCenterY);

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
}

void GameScene::setupTableau() {
    const auto& tableau = _model->getTableau();
    int   count   = (int)tableau.size();
    float spacing = 280.f;
    float totalW  = spacing * (count - 1);
    float startX  = (DESIGN_WIDTH - totalW) / 2.f;
    float y       = PILE_AREA_H + TABLEAU_AREA_H * 0.51f; // 主牌区约51%高度处

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
    const Vec2  btnPos(DESIGN_WIDTH * 0.78f, PILE_AREA_H / 2.f);
    const float TARGET_SIZE  = 160.f; // 按钮显示尺寸（设计单位）
    const float SHADOW_OFFSET = 8.f;  // 阴影偏移量

    // 阴影：同一张图染黑、半透明、往右下偏移
    auto shadow = Sprite::create("res/other/undo_blue.png");
    if (!shadow) { CCLOG("undo_blue.png not found"); return; }
    float scale = TARGET_SIZE / shadow->getContentSize().width;
    shadow->setScale(scale);
    shadow->setAnchorPoint(Vec2(0.5f, 0.5f));
    shadow->setPosition(btnPos + Vec2(SHADOW_OFFSET, -SHADOW_OFFSET));
    shadow->setColor(Color3B(0, 0, 0));
    shadow->setOpacity(90);
    addChild(shadow, 19);

    // 主按钮
    auto btn = Sprite::create("res/other/undo_blue.png");
    btn->setScale(scale);
    btn->setAnchorPoint(Vec2(0.5f, 0.5f));
    btn->setPosition(btnPos);
    addChild(btn, 20);

    // 触摸事件
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
