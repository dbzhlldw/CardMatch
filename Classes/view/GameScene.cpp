// GameScene — 主场景 View：cocos 节点、动画、触摸
#include "GameScene.h"
#include "CardView.h"
#include "data/Levels.h"
#include <algorithm>
#include <functional>

USING_NS_CC;

const float GameScene::ANIM_DURATION  = 0.25f;
const int   GameScene::FLYING_Z_ORDER = 200;

namespace {
const float REJECT_SHAKE_OFFSET     = 14.f;
const float REJECT_SHAKE_SEGMENT    = 0.055f;
const int   REJECT_SHAKE_SEGMENTS   = 5;
const float BLOCKED_BUMP_DROP       = 10.f;
const float BLOCKED_BUMP_SEGMENT    = 0.07f;
const int   BLOCKED_BUMP_SEGMENTS   = 4;
const float BLOCKER_HIGHLIGHT_SCALE = 1.05f;
const float BLOCKER_HIGHLIGHT_SEG   = 0.1f;
} // namespace

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
    _viewState   = _presenter->buildViewState();

    auto tableauBg = LayerColor::create(hexColor(0x7388A3), Layout::DESIGN_WIDTH, Layout::TABLEAU_AREA_H);
    tableauBg->setPosition(Vec2(0.f, Layout::PILE_AREA_H));
    addChild(tableauBg, -2);

    auto pileBg = LayerColor::create(hexColor(0xA7BDD5), Layout::DESIGN_WIDTH, Layout::PILE_AREA_H);
    pileBg->setPosition(Vec2::ZERO);
    addChild(pileBg, -1);

    createAllCardViews();
    applyViewState(_viewState, false);
    setupPileButtons();
    setupRestartButton();
    setupSceneTouch();

    return true;
}

void GameScene::createAllCardViews() {
    for (const auto& spec : _viewState.cards) {
        auto view = CardView::create(spec.cardId, spec.kind, spec.suit, spec.value);
        view->setAnchorPoint(Vec2(0.5f, 0.5f));
        addChild(view, spec.zOrder);
        _cardViewMap[spec.cardId] = view;
    }
    syncPileCache(_viewState);
}

void GameScene::destroyAllCardViews() {
    for (auto& kv : _cardViewMap)
        kv.second->removeFromParent();
    _cardViewMap.clear();
    _cardPileById.clear();
    _touchTargetCardId = -1;
    _reserveTopCardId  = -1;
}

void GameScene::setupPileButtons() {
    const float TARGET_SIZE = 120.f;
    Vec2f pos = _viewState.undoButtonPosition;

    auto button = createIconButton("res/other/undo.png", TARGET_SIZE, IconShadowStyle::Circle);
    if (!button.root) return;

    button.root->setPosition(Vec2(pos.x, pos.y));
    addChild(button.root, 50);

    addIconTouchHandler(button.root, button.hitHalfW, button.hitHalfH, _eventDispatcher,
                        [this]() { onUndoTapped(); });
}

void GameScene::setupRestartButton() {
    const float TARGET_SIZE = 140.f;
    Vec2f pos = _viewState.restartButtonPosition;

    auto button = createIconButton("res/other/reset.png", TARGET_SIZE, IconShadowStyle::Sprite);
    if (!button.root) return;

    _restartButton   = button.root;
    _restartHitHalfW = button.hitHalfW;
    _restartHitHalfH = button.hitHalfH;
    _restartButton->setPosition(Vec2(pos.x, pos.y));
    _restartButton->setVisible(_viewState.showRestartButton);
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
            onRestartTapped();
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, _restartButton);
}

void GameScene::updateRestartButtonVisibility() {
    if (!_restartButton) return;
    _restartButton->setVisible(_viewState.showRestartButton);
}

void GameScene::setupSceneTouch() {
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);

    listener->onTouchBegan = [this](Touch* t, Event*) -> bool {
        if (_inputLocked) return false;

        Vec2 pos = t->getLocation();
        int reserveId = hitTestReserveTopCardId(pos);
        if (reserveId >= 0) {
            _touchTargetCardId = reserveId;
            return true;
        }
        int tableauId = hitTestTableauCardId(pos);
        if (tableauId >= 0) {
            _touchTargetCardId = tableauId;
            return true;
        }
        return false;
    };

    listener->onTouchEnded = [this](Touch* t, Event*) {
        if (_touchTargetCardId < 0 || _inputLocked) {
            _touchTargetCardId = -1;
            return;
        }

        Vec2 pos = t->getLocation();
        int hitId = hitTestReserveTopCardId(pos);
        if (hitId < 0) hitId = hitTestTableauCardId(pos);

        if (hitId == _touchTargetCardId) {
            if (hitId == _reserveTopCardId)
                onReserveTapped();
            else
                onTableauTapped(hitId);
        }
        _touchTargetCardId = -1;
    };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

void GameScene::syncPileCache(const ViewState& state) {
    _cardPileById.clear();
    for (const auto& spec : state.cards)
        _cardPileById[spec.cardId] = spec.pile;
    _reserveTopCardId = state.reserveTopCardId;
}

void GameScene::applyViewState(const ViewState& state, bool animate, int flyingCardId) {
    _viewState = state;
    syncPileCache(state);
    updateRestartButtonVisibility();

    auto place = [&](CardView* view, const CardViewSpec& spec) {
        Vec2 target(spec.position.x, spec.position.y);
        view->stopAllActions();
        view->setVisible(true);

        bool isFlying = animate && flyingCardId >= 0 && view->getCardId() == flyingCardId;
        view->setLocalZOrder(isFlying ? FLYING_Z_ORDER : spec.zOrder);

        bool needsMove = view->getPosition().distance(target) > 1.f;
        if (animate && needsMove) {
            if (isFlying) {
                view->runAction(Sequence::create(
                    MoveTo::create(ANIM_DURATION, target),
                    CallFunc::create([view, spec]() { view->setLocalZOrder(spec.zOrder); }),
                    nullptr));
            } else {
                view->runAction(MoveTo::create(ANIM_DURATION, target));
            }
        } else {
            view->setPosition(target);
        }
    };

    for (const auto& spec : state.cards) {
        CardView* view = viewForCardId(spec.cardId);
        if (!view) continue;
        place(view, spec);
    }
}

CardView* GameScene::viewForCardId(int cardId) const {
    if (cardId < 0) return nullptr;
    auto it = _cardViewMap.find(cardId);
    return it != _cardViewMap.end() ? it->second : nullptr;
}

int GameScene::hitTestTableauCardId(const Vec2& scenePos) const {
    int bestId = -1;
    int bestZ  = -1;

    for (const auto& kv : _cardViewMap) {
        if (_cardPileById.count(kv.first) == 0
            || _cardPileById.at(kv.first) != CardPile::Tableau)
            continue;

        CardView* view = kv.second;
        Vec2 local = view->convertToNodeSpace(scenePos);
        Rect bounds(0, 0, CardView::CARD_SIZE.width, CardView::CARD_SIZE.height);
        if (!bounds.containsPoint(local)) continue;

        int z = view->getLocalZOrder();
        if (z > bestZ) {
            bestZ  = z;
            bestId = kv.first;
        }
    }

    return bestId;
}

int GameScene::hitTestReserveTopCardId(const Vec2& scenePos) const {
    if (_reserveTopCardId < 0) return -1;

    CardView* view = viewForCardId(_reserveTopCardId);
    if (!view) return -1;

    Vec2 local = view->convertToNodeSpace(scenePos);
    Rect bounds(0, 0, CardView::CARD_SIZE.width, CardView::CARD_SIZE.height);
    if (!bounds.containsPoint(local)) return -1;
    return _reserveTopCardId;
}

void GameScene::lockInput() {
    _inputLocked = true;
    unschedule("unlock_input");
}

void GameScene::unlockInput() {
    _inputLocked = false;
}

void GameScene::playRejectShake(int cardId) {
    CardView* view = viewForCardId(cardId);
    if (!view) return;

    view->stopAllActions();

    const float seg = REJECT_SHAKE_SEGMENT;
    lockInput();
    view->runAction(Sequence::create(
        MoveBy::create(seg, Vec2(-REJECT_SHAKE_OFFSET, 0)),
        MoveBy::create(seg, Vec2(REJECT_SHAKE_OFFSET * 2, 0)),
        MoveBy::create(seg, Vec2(-REJECT_SHAKE_OFFSET * 2, 0)),
        MoveBy::create(seg, Vec2(REJECT_SHAKE_OFFSET * 2, 0)),
        MoveBy::create(seg, Vec2(-REJECT_SHAKE_OFFSET, 0)),
        nullptr));

    scheduleOnce([this](float) { unlockInput(); },
                 seg * REJECT_SHAKE_SEGMENTS + 0.05f, "unlock_input");
}

void GameScene::playBlockerHighlight(int cardId) {
    CardView* view = viewForCardId(cardId);
    if (!view) return;

    view->runAction(Sequence::create(
        ScaleTo::create(BLOCKER_HIGHLIGHT_SEG, BLOCKER_HIGHLIGHT_SCALE),
        ScaleTo::create(BLOCKER_HIGHLIGHT_SEG, 1.f),
        nullptr));
}

void GameScene::playBlockedFeedback(int cardId, const std::vector<int>& blockerIds) {
    CardView* view = viewForCardId(cardId);
    if (!view) return;

    view->stopAllActions();
    view->setScale(1.f);

    const float seg  = BLOCKED_BUMP_SEGMENT;
    const float drop = BLOCKED_BUMP_DROP;
    lockInput();
    view->runAction(Sequence::create(
        MoveBy::create(seg, Vec2(0, -drop)),
        MoveBy::create(seg, Vec2(0, drop)),
        MoveBy::create(seg, Vec2(0, -drop * 0.6f)),
        MoveBy::create(seg, Vec2(0, drop * 0.6f)),
        nullptr));

    for (int blockerId : blockerIds)
        playBlockerHighlight(blockerId);

    const float bumpDuration = seg * BLOCKED_BUMP_SEGMENTS;
    const float highlightDuration = BLOCKER_HIGHLIGHT_SEG * 2.f;
    scheduleOnce([this](float) { unlockInput(); },
                 std::max(bumpDuration, highlightDuration) + 0.05f, "unlock_input");
}

void GameScene::handlePresenterOutcome(const PresenterOutcome& outcome) {
    switch (outcome.tableauResult) {
        case TableauTapResult::Blocked:
            playBlockedFeedback(outcome.primaryCardId, outcome.highlightCardIds);
            return;
        case TableauTapResult::NoMatch:
            playRejectShake(outcome.primaryCardId);
            return;
        default:
            break;
    }

    if (!outcome.success) return;

    if (outcome.fullRestart) {
        _viewState = _presenter->buildViewState();
        destroyAllCardViews();
        createAllCardViews();
        applyViewState(_viewState, false);
        return;
    }

    lockInput();
    applyViewState(_presenter->buildViewState(), true, outcome.flyingCardId);
    scheduleOnce([this](float) {
        unlockInput();
        updateRestartButtonVisibility();
    }, ANIM_DURATION + 0.05f, "unlock_input");
}

void GameScene::onTableauTapped(int cardId) {
    if (_inputLocked) return;
    handlePresenterOutcome(_presenter->onTableauTapped(cardId));
}

void GameScene::onReserveTapped() {
    if (_inputLocked) return;
    handlePresenterOutcome(_presenter->onReserveTapped());
}

void GameScene::onUndoTapped() {
    if (_inputLocked) return;
    handlePresenterOutcome(_presenter->onUndoTapped());
}

void GameScene::onRestartTapped() {
    if (_inputLocked) return;

    lockInput();
    handlePresenterOutcome(_presenter->onRestartTapped());
    unlockInput();
}
