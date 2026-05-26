// GameScene — 场景初始化、CardView 生命周期、布局动画与触摸的实现
#include "GameScene.h"
#include "CardView.h"
#include "model/CardModel.h"
#include "data/Levels.h"
#include <algorithm>
#include <functional>

USING_NS_CC;

const float GameScene::ANIM_DURATION  = 0.25f;
const int   GameScene::FLYING_Z_ORDER = 200;

namespace {

const float SHADOW_OFFSET       = 4.f;
const int   SHADOW_LAYERS       = 5;
const float SHADOW_SPREAD       = 6.f;
const float SPRITE_SHADOW_ALPHA = 0.35f;

cocos2d::Color4B hexColor(unsigned int rgb) {
    return cocos2d::Color4B((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF, 255);
}

void drawSoftCircleShadow(cocos2d::DrawNode* dn, float radius) {
    for (int i = SHADOW_LAYERS - 1; i >= 0; --i) {
        float t = (float)(i + 1) / SHADOW_LAYERS;
        float r = radius + SHADOW_SPREAD * t;
        float alpha = 0.20f * (1.f - t * 0.92f);
        dn->drawDot(cocos2d::Vec2::ZERO, r, cocos2d::Color4F(0.f, 0.f, 0.f, alpha));
    }
}

void addSoftSpriteShadow(cocos2d::Node* parent,
                         cocos2d::SpriteFrame* frame,
                         float scale) {
    auto* shadow = cocos2d::Sprite::createWithSpriteFrame(frame);
    shadow->setAnchorPoint(cocos2d::Vec2(0.5f, 0.5f));
    shadow->setScale(scale);
    shadow->setColor(cocos2d::Color3B::BLACK);
    shadow->setOpacity((GLubyte)(255 * SPRITE_SHADOW_ALPHA));
    shadow->setPosition(cocos2d::Vec2(SHADOW_OFFSET, -SHADOW_OFFSET));
    parent->addChild(shadow, 0);
}

enum class IconShadowStyle { Circle, Sprite };

struct IconButton {
    cocos2d::Node* root     = nullptr;
    float          hitHalfW = 0.f;
    float          hitHalfH = 0.f;
};

IconButton createIconButton(const char* imagePath,
                            float targetSize,
                            IconShadowStyle shadowStyle) {
    IconButton button;
    auto* icon = cocos2d::Sprite::create(imagePath);
    if (!icon) {
        CCLOG("%s not found", imagePath);
        return button;
    }

    float scale    = targetSize / icon->getContentSize().width;
    float hitHalfW = icon->getContentSize().width * scale * 0.5f;
    float hitHalfH = icon->getContentSize().height * scale * 0.5f;

    auto* root = cocos2d::Node::create();
    root->setAnchorPoint(cocos2d::Vec2(0.5f, 0.5f));

    if (shadowStyle == IconShadowStyle::Circle) {
        auto* shadow = cocos2d::DrawNode::create();
        drawSoftCircleShadow(shadow, targetSize * 0.5f);
        shadow->setPosition(cocos2d::Vec2(SHADOW_OFFSET, -SHADOW_OFFSET));
        root->addChild(shadow, 0);
    } else {
        addSoftSpriteShadow(root, icon->getSpriteFrame(), scale);
    }

    icon->setScale(scale);
    icon->setAnchorPoint(cocos2d::Vec2(0.5f, 0.5f));
    root->addChild(icon, 1);

    button.root     = root;
    button.hitHalfW = hitHalfW;
    button.hitHalfH = hitHalfH;
    return button;
}

bool hitTestCenteredNode(cocos2d::Node* node, float halfW, float halfH, cocos2d::Touch* touch) {
    cocos2d::Vec2 local = node->convertToNodeSpace(touch->getLocation());
    return cocos2d::Rect(-halfW, -halfH, halfW * 2.f, halfH * 2.f).containsPoint(local);
}

void addIconTouchHandler(cocos2d::Node* target,
                         float halfW,
                         float halfH,
                         cocos2d::EventDispatcher* dispatcher,
                         const std::function<void()>& onClick) {
    auto listener = cocos2d::EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [target, halfW, halfH](cocos2d::Touch* t, cocos2d::Event*) -> bool {
        return hitTestCenteredNode(target, halfW, halfH, t);
    };
    listener->onTouchEnded = [target, halfW, halfH, onClick](cocos2d::Touch* t, cocos2d::Event*) {
        if (hitTestCenteredNode(target, halfW, halfH, t))
            onClick();
    };
    dispatcher->addEventListenerWithSceneGraphPriority(listener, target);
}

} // namespace

Scene* GameScene::createScene() {
    return GameScene::create();
}

bool GameScene::init() {
    if (!Scene::init()) return false;

    _presenter = std::make_unique<GamePresenter>(Levels::PYRAMID_TEST);

    auto tableauBg = LayerColor::create(hexColor(0x7388A3), Layout::DESIGN_WIDTH, Layout::TABLEAU_AREA_H);
    tableauBg->setPosition(Vec2(0.f, Layout::PILE_AREA_H));
    addChild(tableauBg, -2);

    auto pileBg = LayerColor::create(hexColor(0xA7BDD5), Layout::DESIGN_WIDTH, Layout::PILE_AREA_H);
    pileBg->setPosition(Vec2::ZERO);
    addChild(pileBg, -1);

    createAllCardViews();
    rebuildViewStacksFromModel();
    applyAllLayouts(false);
    syncAllInteractable();
    setupPileButtons();
    setupRestartButton();
    setupSceneTouch();

    return true;
}

// ---------------------------------------------------------------------------
// 初始化
// ---------------------------------------------------------------------------

void GameScene::createAllCardViews() {
    const auto& tableau = _presenter->model().getTableau();
    const auto& level   = _presenter->level();

    for (int i = 0; i < (int)tableau.size(); i++) {
        CardModel* card = tableau[i];
        _presenter->registerTableauSlot(card, i);

        auto view = CardView::create(card);
        view->setAnchorPoint(Vec2(0.5f, 0.5f));
        addChild(view, level.layout[i].zOrder);
        _cardViewMap[card] = view;
    }

    for (auto* card : _presenter->model().getHand()) {
        auto view = CardView::create(card);
        view->setAnchorPoint(Vec2(0.5f, 0.5f));
        addChild(view, 10);
        _cardViewMap[card] = view;
    }

    for (auto* card : _presenter->model().getReserve()) {
        auto view = CardView::create(card);
        view->setAnchorPoint(Vec2(0.5f, 0.5f));
        addChild(view, 10);
        _cardViewMap[card] = view;
    }
}

void GameScene::destroyAllCardViews() {
    for (auto& kv : _cardViewMap)
        kv.second->removeFromParent();
    _cardViewMap.clear();
    _tableauViews.clear();
    _handViewStack.clear();
    _reserveViewStack.clear();
    _touchTarget = nullptr;
}

void GameScene::setupPileButtons() {
    const float TARGET_SIZE = 120.f;
    Vec2f pos = _presenter->undoButtonPosition();

    auto button = createIconButton("res/other/undo.png", TARGET_SIZE, IconShadowStyle::Circle);
    if (!button.root) return;

    button.root->setPosition(Vec2(pos.x, pos.y));
    addChild(button.root, 50);

    addIconTouchHandler(button.root, button.hitHalfW, button.hitHalfH, _eventDispatcher,
                        [this]() { onUndoClicked(); });
}

void GameScene::setupRestartButton() {
    const float TARGET_SIZE = 140.f;
    Vec2f pos = _presenter->restartButtonPosition();

    auto button = createIconButton("res/other/reset.png", TARGET_SIZE, IconShadowStyle::Sprite);
    if (!button.root) return;

    _restartButton   = button.root;
    _restartHitHalfW = button.hitHalfW;
    _restartHitHalfH = button.hitHalfH;
    _restartButton->setPosition(Vec2(pos.x, pos.y));
    _restartButton->setVisible(false);
    addChild(_restartButton, 101);

    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [this](Touch* t, Event*) -> bool {
        if (!_restartButton || !_restartButton->isVisible()) return false;
        return hitTestCenteredNode(_restartButton, _restartHitHalfW, _restartHitHalfH, t);
    };
    listener->onTouchEnded = [this](Touch* t, Event*) {
        if (_restartButton && _restartButton->isVisible()
            && hitTestCenteredNode(_restartButton, _restartHitHalfW, _restartHitHalfH, t))
            onRestartClicked();
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, _restartButton);
}

void GameScene::updateRestartButtonVisibility() {
    if (!_restartButton) return;
    _restartButton->setVisible(_presenter->isRestartButtonVisible());
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
            if (_presenter->isReserveTop(hit->getModel()))
                onReserveClicked();
            else
                onTableauCardClicked(hit);
        }
        _touchTarget = nullptr;
    };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

// ---------------------------------------------------------------------------
// Presenter 驱动的 View 同步
// ---------------------------------------------------------------------------

void GameScene::rebuildViewStacksFromModel() {
    _handViewStack.clear();
    _reserveViewStack.clear();
    _tableauViews.clear();

    for (auto* card : _presenter->model().getHand()) {
        if (auto* view = viewForCard(card)) _handViewStack.push_back(view);
    }
    for (auto* card : _presenter->model().getReserve()) {
        if (auto* view = viewForCard(card)) _reserveViewStack.push_back(view);
    }
    for (auto* card : _presenter->model().getTableau()) {
        if (auto* view = viewForCard(card)) _tableauViews.push_back(view);
    }
}

void GameScene::applyAllLayouts(bool animate, CardView* flyingView) {
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

    for (const auto& spec : _presenter->buildCardLayout()) {
        CardView* view = viewForCard(spec.card);
        if (!view) continue;
        place(view, Vec2(spec.position.x, spec.position.y), spec.zOrder, spec.faceUp);
    }
}

void GameScene::syncAllInteractable() {
    for (auto& kv : _cardViewMap)
        kv.second->setClickCallback(nullptr);
}

CardView* GameScene::viewForCard(CardModel* card) const {
    if (!card) return nullptr;
    auto it = _cardViewMap.find(card);
    return it != _cardViewMap.end() ? it->second : nullptr;
}

// ---------------------------------------------------------------------------
// Hit test（几何检测在 Scene，选牌决策在 Presenter）
// ---------------------------------------------------------------------------

CardView* GameScene::hitTestTableau(const Vec2& scenePos) const {
    std::vector<TableauHitCandidate> hits;

    for (auto* card : _presenter->model().getTableau()) {
        if (!card->isAccessible()) continue;
        CardView* view = viewForCard(card);
        if (!view) continue;

        Vec2 local = view->convertToNodeSpace(scenePos);
        Rect bounds(0, 0, CardView::CARD_SIZE.width, CardView::CARD_SIZE.height);
        if (!bounds.containsPoint(local)) continue;

        hits.push_back({ card, view->getLocalZOrder(),
                         _presenter->controller().canMatchTableau(card) });
    }

    CardModel* picked = _presenter->pickTableauCard(hits);
    return viewForCard(picked);
}

CardView* GameScene::hitTestReserveTop(const Vec2& scenePos) const {
    CardModel* top = _presenter->reserveTopCard();
    CardView* view = viewForCard(top);
    if (!view) return nullptr;

    Vec2 local = view->convertToNodeSpace(scenePos);
    Rect bounds(0, 0, CardView::CARD_SIZE.width, CardView::CARD_SIZE.height);
    if (bounds.containsPoint(local)) return view;
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

void GameScene::refreshViewAfterAction(const PresenterOutcome& outcome) {
    if (!outcome.success) return;

    if (outcome.fullRestart) {
        destroyAllCardViews();
        createAllCardViews();
        rebuildViewStacksFromModel();
        applyAllLayouts(false);
        syncAllInteractable();
        updateRestartButtonVisibility();
        return;
    }

    lockInput();
    CardView* flying = viewForCard(outcome.flyingCard);
    rebuildViewStacksFromModel();
    applyAllLayouts(true, flying);
    syncAllInteractable();
    scheduleOnce([this](float) {
        unlockInput();
        updateRestartButtonVisibility();
    }, ANIM_DURATION + 0.05f, "unlock_input");
}

void GameScene::onTableauCardClicked(CardView* view) {
    if (_inputLocked) return;
    refreshViewAfterAction(_presenter->onTableauTapped(view->getModel()));
}

void GameScene::onReserveClicked() {
    if (_inputLocked) return;
    refreshViewAfterAction(_presenter->onReserveTapped());
}

void GameScene::onUndoClicked() {
    if (_inputLocked) return;
    refreshViewAfterAction(_presenter->onUndoTapped());
}

void GameScene::onRestartClicked() {
    if (_inputLocked) return;

    lockInput();
    refreshViewAfterAction(_presenter->onRestartTapped());
    unlockInput();
}
