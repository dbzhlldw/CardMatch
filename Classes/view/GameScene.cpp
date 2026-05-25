#include "GameScene.h"
#include "CardView.h"
#include "model/GameModel.h"
#include "model/CardModel.h"
#include "controller/GameController.h"
#include "data/LayoutDef.h"
#include <algorithm>

USING_NS_CC;

const float GameScene::DESIGN_WIDTH        = 1080.f;
const float GameScene::DESIGN_HEIGHT       = 2080.f;
const float GameScene::PILE_AREA_H         = 580.f;
const float GameScene::TABLEAU_AREA_H        = GameScene::DESIGN_HEIGHT - GameScene::PILE_AREA_H;
const float GameScene::ANIM_DURATION       = 0.25f;
const float GameScene::RESERVE_STACK_OFFSET = 15.f;
const int   GameScene::FLYING_Z_ORDER      = 200;

Scene* GameScene::createScene() {
    return GameScene::create();
}

bool GameScene::init() {
    if (!Scene::init()) return false;

    _model      = GameModel::getInstance();
    _controller = GameController::getInstance();

    _layout = Layouts::PYRAMID;
    _model->setupGame(_layout);
    _controller->init();

    float pileAreaCenterY = PILE_AREA_H / 2.f;
    _reservePos  = Vec2(DESIGN_WIDTH * 0.35f, pileAreaCenterY);
    _handPilePos = Vec2(DESIGN_WIDTH * 0.65f, pileAreaCenterY);

    auto bg = LayerColor::create(Color4B(110, 110, 110, 255), DESIGN_WIDTH, PILE_AREA_H);
    bg->setPosition(Vec2::ZERO);
    addChild(bg, -1);

    createAllCardViews();
    rebuildViewStacksFromModel();
    applyAllLayouts(false);
    syncAllInteractable();
    setupUndoButton();
    setupSceneTouch();

    return true;
}

// ---------------------------------------------------------------------------
// 初始化
// ---------------------------------------------------------------------------

void GameScene::createAllCardViews() {
    const auto& tableau = _model->getTableau();
    for (int i = 0; i < (int)tableau.size(); i++) {
        CardModel* card = tableau[i];
        _cardSlotIndex[card] = i;

        auto view = CardView::create(card);
        view->setAnchorPoint(Vec2(0.5f, 0.5f));
        addChild(view, _layout[i].zOrder);
        _cardViewMap[card] = view;
    }

    for (auto* card : _model->getHand()) {
        auto view = CardView::create(card);
        view->setAnchorPoint(Vec2(0.5f, 0.5f));
        addChild(view, 10);
        _cardViewMap[card] = view;
    }

    for (auto* card : _model->getReserve()) {
        auto view = CardView::create(card);
        view->setAnchorPoint(Vec2(0.5f, 0.5f));
        addChild(view, 10);
        _cardViewMap[card] = view;
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

void GameScene::setupSceneTouch() {
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);

    listener->onTouchBegan = [this](Touch* t, Event*) -> bool {
        if (_inputLocked) return false;

        Vec2 pos = t->getLocation();
        if (auto* reserveTop = hitTestReserveTop(pos)) {
            _touchTarget = reserveTop;
            return true;
        }
        if (auto* tableau = hitTestTableau(pos)) {
            _touchTarget = tableau;
            return true;
        }
        return false;
    };

    listener->onTouchEnded = [this](Touch* t, Event*) {
        if (!_touchTarget || _inputLocked) {
            _touchTarget = nullptr;
            return;
        }
        Vec2 pos = t->getLocation();
        CardView* hit = hitTestReserveTop(pos);
        if (!hit) hit = hitTestTableau(pos);
        if (hit == _touchTarget) {
            if (std::find(_reserveViewStack.begin(), _reserveViewStack.end(), hit)
                != _reserveViewStack.end()) {
                onReserveClicked();
            } else {
                onTableauCardClicked(hit);
            }
        }
        _touchTarget = nullptr;
    };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

// ---------------------------------------------------------------------------
// Model 驱动的 View 同步
// ---------------------------------------------------------------------------

void GameScene::rebuildViewStacksFromModel() {
    _handViewStack.clear();
    _reserveViewStack.clear();
    _tableauViews.clear();

    for (auto* card : _model->getHand()) {
        auto it = _cardViewMap.find(card);
        if (it != _cardViewMap.end()) _handViewStack.push_back(it->second);
    }
    for (auto* card : _model->getReserve()) {
        auto it = _cardViewMap.find(card);
        if (it != _cardViewMap.end()) _reserveViewStack.push_back(it->second);
    }
    for (auto* card : _model->getTableau()) {
        auto it = _cardViewMap.find(card);
        if (it != _cardViewMap.end()) _tableauViews.push_back(it->second);
    }
}

Vec2 GameScene::handPositionFor(int /*index*/) const {
    return _handPilePos;
}

Vec2 GameScene::reservePositionFor(int index, int total) const {
    int fromTop = total - 1 - index;
    return _reservePos - Vec2(fromTop * RESERVE_STACK_OFFSET, 0.f);
}

Vec2 GameScene::slotPositionFor(CardModel* card) const {
    auto it = _cardSlotIndex.find(card);
    if (it == _cardSlotIndex.end()) return Vec2::ZERO;
    const SlotDef& slot = _layout[it->second];
    return Vec2(slot.x, slot.y);
}

int GameScene::slotZOrderFor(CardModel* card) const {
    auto it = _cardSlotIndex.find(card);
    if (it == _cardSlotIndex.end()) return 0;
    return _layout[it->second].zOrder;
}

void GameScene::applyAllLayouts(bool animate, CardView* flyingView) {
    const auto& hand    = _model->getHand();
    const auto& reserve = _model->getReserve();
    const auto& tableau = _model->getTableau();
    const int   rn      = (int)reserve.size();

    auto place = [&](CardView* view, const Vec2& target, int finalZ, bool faceUp) {
        view->stopAllActions();
        view->setFaceUp(faceUp);
        view->setVisible(true);

        bool isFlying = animate && flyingView && view == flyingView;
        view->setLocalZOrder(isFlying ? FLYING_Z_ORDER : finalZ);

        bool needsMove = view->getPosition().distance(target) > 1.f;
        if (animate && needsMove) {
            if (isFlying) {
                view->runAction(Sequence::create(
                    MoveTo::create(ANIM_DURATION, target),
                    CallFunc::create([view, finalZ]() { view->setLocalZOrder(finalZ); }),
                    nullptr));
            } else {
                view->runAction(MoveTo::create(ANIM_DURATION, target));
            }
        } else {
            view->setPosition(target);
        }
    };

    for (int i = 0; i < (int)hand.size(); i++)
        place(_cardViewMap[hand[i]], handPositionFor(i), 10 + i, true);

    for (int i = 0; i < rn; i++)
        place(_cardViewMap[reserve[i]], reservePositionFor(i, rn), 10 + i, true);

    for (auto* card : tableau)
        place(_cardViewMap[card], slotPositionFor(card), slotZOrderFor(card), true);
}

void GameScene::syncAllInteractable() {
    // 桌面/备用牌堆的点击由 Scene 级 hit-test 处理，清除所有 CardView 回调
    for (auto& kv : _cardViewMap)
        kv.second->setClickCallback(nullptr);
}

// ---------------------------------------------------------------------------
// Hit test（按 z-order 从高到低，优先点中最上层可操作的牌）
// ---------------------------------------------------------------------------

CardView* GameScene::hitTestTableau(const Vec2& scenePos) const {
    struct Entry { CardView* view; bool matchable; int z; };
    std::vector<Entry> hits;

    for (auto* card : _model->getTableau()) {
        if (!card->isAccessible()) continue;
        auto it = _cardViewMap.find(card);
        if (it == _cardViewMap.end()) continue;

        CardView* view = it->second;
        Vec2 local = view->convertToNodeSpace(scenePos);
        Rect bounds(0, 0, CardView::CARD_SIZE.width, CardView::CARD_SIZE.height);
        if (!bounds.containsPoint(local)) continue;

        hits.push_back({ view, _controller->canMatchTableau(card), view->getLocalZOrder() });
    }

    if (hits.empty()) return nullptr;

    // 优先：可匹配 + z 最高；其次：任意可操作 + z 最高
    auto best = [&](bool requireMatchable) -> CardView* {
        CardView* pick = nullptr;
        int bestZ = -1;
        for (const auto& e : hits) {
            if (requireMatchable && !e.matchable) continue;
            if (e.z > bestZ) { bestZ = e.z; pick = e.view; }
        }
        return pick;
    };

    if (CardView* v = best(true)) return v;
    return best(false);
}

CardView* GameScene::hitTestReserveTop(const Vec2& scenePos) const {
    if (_model->getReserveSize() == 0 || _reserveViewStack.empty()) return nullptr;
    CardView* top = _reserveViewStack.back();
    Vec2 local = top->convertToNodeSpace(scenePos);
    Rect bounds(0, 0, CardView::CARD_SIZE.width, CardView::CARD_SIZE.height);
    if (bounds.containsPoint(local)) return top;
    return nullptr;
}

// ---------------------------------------------------------------------------
// 用户交互
// ---------------------------------------------------------------------------

void GameScene::lockInput() {
    _inputLocked = true;
    unschedule("unlock_input");
}

void GameScene::unlockInput() {
    _inputLocked = false;
}

void GameScene::onTableauCardClicked(CardView* view) {
    if (_inputLocked) return;

    CardModel* card = view->getModel();
    if (!card->isAccessible()) return;
    if (!_controller->tryMatch(card)) return;

    lockInput();
    rebuildViewStacksFromModel();
    applyAllLayouts(true, _cardViewMap[_model->getHandTop()]);
    syncAllInteractable();
    scheduleOnce([this](float) { unlockInput(); }, ANIM_DURATION + 0.05f, "unlock_input");
}

void GameScene::onReserveClicked() {
    if (_inputLocked) return;
    if (!_controller->tryDraw()) return;

    lockInput();
    rebuildViewStacksFromModel();
    applyAllLayouts(true, _cardViewMap[_model->getHandTop()]);
    syncAllInteractable();
    scheduleOnce([this](float) { unlockInput(); }, ANIM_DURATION + 0.05f, "unlock_input");
}

void GameScene::onUndoClicked() {
    if (_inputLocked || !_controller->canUndo()) return;

    CardModel* flyingCard = _model->getHandTop();
    CardView*  flyingView = flyingCard ? _cardViewMap[flyingCard] : nullptr;

    lockInput();
    _controller->tryUndo();
    rebuildViewStacksFromModel();
    applyAllLayouts(true, flyingView);
    syncAllInteractable();
    scheduleOnce([this](float) { unlockInput(); }, ANIM_DURATION + 0.05f, "unlock_input");
}
