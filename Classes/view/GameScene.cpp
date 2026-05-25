#include "GameScene.h"
#include "CardView.h"
#include "model/GameModel.h"
#include "model/CardModel.h"
#include "controller/GameController.h"
#include "controller/command/MatchCommand.h"
#include "controller/command/DrawCommand.h"
#include "data/LayoutDef.h"
#include <algorithm>

USING_NS_CC;

const float GameScene::DESIGN_WIDTH   = 1080.f;
const float GameScene::DESIGN_HEIGHT  = 2080.f;
const float GameScene::PILE_AREA_H    = 580.f;
const float GameScene::TABLEAU_AREA_H = GameScene::DESIGN_HEIGHT - GameScene::PILE_AREA_H;
const float GameScene::ANIM_DURATION  = 0.25f;

// 备用牌堆每张牌的水平偏移
static const float RESERVE_STACK_OFFSET = 15.f;

Scene* GameScene::createScene() {
    return GameScene::create();
}

bool GameScene::init() {
    if (!Scene::init()) return false;

    _model      = GameModel::getInstance();
    _controller = GameController::getInstance();

    _model->setupGame(Layouts::PYRAMID);
    _controller->init();

    float pileAreaCenterY = PILE_AREA_H / 2.f;
    _reservePos  = Vec2(DESIGN_WIDTH * 0.35f, pileAreaCenterY);
    _handPilePos = Vec2(DESIGN_WIDTH * 0.65f, pileAreaCenterY);

    // 堆牌区灰色背景
    auto bg = LayerColor::create(Color4B(110, 110, 110, 255), DESIGN_WIDTH, PILE_AREA_H);
    bg->setPosition(Vec2::ZERO);
    addChild(bg, -1);

    setupHandPile();
    setupReserve();
    setupTableau(Layouts::PYRAMID);
    setupUndoButton();

    return true;
}

// ---------------------------------------------------------------------------
// 初始布局
// ---------------------------------------------------------------------------

void GameScene::setupHandPile() {
    const auto& hand = _model->getHand();
    for (int i = 0; i < (int)hand.size(); i++) {
        auto view = CardView::create(hand[i]);
        view->setAnchorPoint(Vec2(0.5f, 0.5f));
        view->setPosition(_handPilePos);
        addChild(view, 10 + i);
        _handViewStack.push_back(view);
    }
}

void GameScene::setupReserve() {
    const auto& reserve = _model->getReserve();
    int n = (int)reserve.size();
    for (int i = 0; i < n; i++) {
        int fromTop = n - 1 - i;
        auto view = CardView::create(reserve[i]);
        view->setFaceUp(true); // 备用牌堆显示正面
        view->setAnchorPoint(Vec2(0.5f, 0.5f));
        view->setPosition(_reservePos - Vec2(fromTop * RESERVE_STACK_OFFSET, 0));
        addChild(view, 10 + i);
        _reserveViewStack.push_back(view);
    }
    if (!_reserveViewStack.empty()) {
        _reserveViewStack.back()->setClickCallback(
            [this](CardView* v){ onReserveClicked(v); });
    }
}

void GameScene::setupTableau(const LayoutDef& layout) {
    const auto& tableau = _model->getTableau();
    int count = (int)tableau.size();

    for (int i = 0; i < count; i++) {
        const SlotDef& slot = layout[i];
        auto view = CardView::create(tableau[i]);
        view->setAnchorPoint(Vec2(0.5f, 0.5f));
        view->setPosition(slot.x, slot.y);
        view->setLocalZOrder(slot.zOrder);
        // 全部正面朝上；只有可操作（无遮挡）的牌注册点击回调
        if (tableau[i]->isAccessible()) {
            view->setClickCallback([this](CardView* v){ onTableauCardClicked(v); });
        }
        addChild(view, slot.zOrder);
        _tableauViews.push_back(view);
        _cardViewMap[tableau[i]] = view;
    }
}

void GameScene::setupUndoButton() {
    const Vec2  btnPos(DESIGN_WIDTH * 0.88f, PILE_AREA_H / 2.f);
    const float TARGET_SIZE   = 160.f;
    const float SHADOW_OFFSET = 8.f;

    auto shadow = Sprite::create("res/other/undo_blue.png");
    if (!shadow) { CCLOG("undo_blue.png not found"); return; }
    float scale = TARGET_SIZE / shadow->getContentSize().width;
    shadow->setScale(scale);
    shadow->setAnchorPoint(Vec2(0.5f, 0.5f));
    shadow->setPosition(btnPos + Vec2(SHADOW_OFFSET, -SHADOW_OFFSET));
    shadow->setColor(Color3B(0, 0, 0));
    shadow->setOpacity(90);
    addChild(shadow, 40);

    auto btn = Sprite::create("res/other/undo_blue.png");
    btn->setScale(scale);
    btn->setAnchorPoint(Vec2(0.5f, 0.5f));
    btn->setPosition(btnPos);
    addChild(btn, 50);

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
    int       returnZ     = view->getLocalZOrder();

    MatchCommand* cmd = _controller->tryMatch(view->getModel());
    if (!cmd) return;

    int newIdx = (int)_handViewStack.size();

    _animHistory.push_back({ view, returnPos, true,
                              cmd->getTableauIndex(), prevHandTop, returnZ });

    _tableauViews.erase(
        std::remove(_tableauViews.begin(), _tableauViews.end(), view),
        _tableauViews.end());

    // 注册因此次匹配而解锁的桌面牌的点击回调
    for (auto* newCard : cmd->getNewlyUnblocked()) {
        auto it = _cardViewMap.find(newCard);
        if (it != _cardViewMap.end()) {
            it->second->setClickCallback([this](CardView* cv){ onTableauCardClicked(cv); });
        }
    }

    _handViewStack.push_back(view);
    refreshHandPileDisplay();
    view->setLocalZOrder(60); // 动画期间置于最顶层

    auto seq = Sequence::create(
        MoveTo::create(ANIM_DURATION, _handPilePos),
        CallFunc::create([view, newIdx]() { view->setLocalZOrder(10 + newIdx); }),
        nullptr);
    view->runAction(seq);
}

void GameScene::onReserveClicked(CardView* view) {
    CardView* prevHandTop = _handViewStack.empty() ? nullptr : _handViewStack.back();

    DrawCommand* cmd = _controller->tryDraw();
    if (!cmd) return;

    Vec2 returnPos = _reservePos;

    _animHistory.push_back({ view, returnPos, false, -1, prevHandTop, 0 });

    _reserveViewStack.erase(
        std::remove(_reserveViewStack.begin(), _reserveViewStack.end(), view),
        _reserveViewStack.end());

    // 剩余备用牌整体右移，保持新顶牌在 _reservePos
    {
        int n = (int)_reserveViewStack.size();
        for (int i = 0; i < n; i++) {
            int fromTop = n - 1 - i;
            Vec2 newPos = _reservePos - Vec2(fromTop * RESERVE_STACK_OFFSET, 0);
            _reserveViewStack[i]->stopAllActions();
            _reserveViewStack[i]->runAction(MoveTo::create(ANIM_DURATION, newPos));
        }
    }

    if (!_reserveViewStack.empty()) {
        _reserveViewStack.back()->setClickCallback(
            [this](CardView* v){ onReserveClicked(v); });
    }

    int newHandIdx = (int)_handViewStack.size();

    _handViewStack.push_back(view);
    refreshHandPileDisplay();
    refreshReserveDisplay();
    view->setLocalZOrder(60);

    auto seq = Sequence::create(
        MoveTo::create(ANIM_DURATION, _handPilePos),
        CallFunc::create([view, newHandIdx]() { view->setLocalZOrder(10 + newHandIdx); }),
        nullptr);
    view->runAction(seq);
}

void GameScene::onUndoClicked() {
    if (_animHistory.empty() || !_controller->canUndo()) return;

    MoveRecord record = _animHistory.back();
    _animHistory.pop_back();

    ICommand* undoneCmd = _controller->tryUndo();

    _handViewStack.pop_back();

    CardView* view = record.view;

    if (record.returnToTableau) {
        // 将因此次匹配被解锁的牌重新清除点击回调（它们又被遮挡了）
        MatchCommand* matchCmd = dynamic_cast<MatchCommand*>(undoneCmd);
        if (matchCmd) {
            for (auto* reblocked : matchCmd->getNewlyUnblocked()) {
                auto it = _cardViewMap.find(reblocked);
                if (it != _cardViewMap.end()) {
                    it->second->setClickCallback(nullptr);
                }
            }
        }

        view->setFaceUp(true); // 保证正面（本就应为正面）
        view->setLocalZOrder(60); // 动画期间保持最顶，避免遮挡瞬变

        int idx = record.tableauIndex;
        if (idx >= (int)_tableauViews.size())
            _tableauViews.push_back(view);
        else
            _tableauViews.insert(_tableauViews.begin() + idx, view);

        view->setClickCallback([this](CardView* v){ onTableauCardClicked(v); });

        int returnZ = record.returnZOrder;
        auto seq = Sequence::create(
            MoveTo::create(ANIM_DURATION, record.returnPos),
            CallFunc::create([view, returnZ]() { view->setLocalZOrder(returnZ); }),
            nullptr);
        view->runAction(seq);
    } else {
        // 现有备用牌整体左移，为归回的顶牌腾出锚点位置
        int n = (int)_reserveViewStack.size();
        for (int i = 0; i < n; i++) {
            int fromTop = n - i;
            Vec2 newPos = _reservePos - Vec2(fromTop * RESERVE_STACK_OFFSET, 0);
            _reserveViewStack[i]->stopAllActions();
            _reserveViewStack[i]->runAction(MoveTo::create(ANIM_DURATION, newPos));
        }

        view->setLocalZOrder(60);
        int finalZ = 10 + n;

        _reserveViewStack.push_back(view);
        view->setClickCallback([this](CardView* v){ onReserveClicked(v); });

        auto seq = Sequence::create(
            MoveTo::create(ANIM_DURATION, record.returnPos),
            CallFunc::create([view, finalZ]() { view->setLocalZOrder(finalZ); }),
            nullptr);
        view->runAction(seq);
    }

    refreshHandPileDisplay();
    refreshReserveDisplay();
}

// ---------------------------------------------------------------------------
// 刷新显示
// ---------------------------------------------------------------------------

void GameScene::refreshHandPileDisplay() {
    for (int i = 0; i < (int)_handViewStack.size(); i++) {
        _handViewStack[i]->setLocalZOrder(10 + i);
        _handViewStack[i]->setVisible(true);
    }
}

void GameScene::refreshReserveDisplay() {
    int n = (int)_reserveViewStack.size();
    for (int i = 0; i < n; i++) {
        _reserveViewStack[i]->setLocalZOrder(10 + i);
        _reserveViewStack[i]->setVisible(true);
    }
    // 非顶牌清除回调，防止误触露出的边缘
    for (int i = 0; i + 1 < n; i++) {
        _reserveViewStack[i]->setClickCallback(nullptr);
    }
}
