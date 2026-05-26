// GamePresenter — 布局计算、触摸决策、驱动 Session/Controller，不算动画，只给目标坐标
#include "GamePresenter.h"
#include "model/GameModel.h"
#include "model/CardModel.h"
#include "controller/GameController.h"

GamePresenter::GamePresenter(const LevelDef& level)
    : _session(level) {}

GameModel& GamePresenter::model() { return _session.model(); }
const GameModel& GamePresenter::model() const { return _session.model(); }
GameController& GamePresenter::controller() { return _session.controller(); }
const GameController& GamePresenter::controller() const { return _session.controller(); }
const LevelDef& GamePresenter::level() const { return _session.level(); }

void GamePresenter::restart() {
    _session.restart();
    clearSlotRegistry();
}

void GamePresenter::registerTableauSlot(CardModel* card, int slotIndex) {
    _slotIndex[card] = slotIndex;
}

void GamePresenter::clearSlotRegistry() {
    _slotIndex.clear();
}

Vec2f GamePresenter::handPilePosition() const {
    return { Layout::DESIGN_WIDTH * 0.65f, Layout::PILE_AREA_H / 2.f };
}

Vec2f GamePresenter::undoButtonPosition() const {
    return { Layout::DESIGN_WIDTH * 0.88f, Layout::PILE_AREA_H / 2.f };
}

Vec2f GamePresenter::restartButtonPosition() const {
    return { Layout::DESIGN_WIDTH * 0.5f,
             Layout::PILE_AREA_H + Layout::TABLEAU_AREA_H * 0.5f };
}

Vec2f GamePresenter::reservePositionFor(int index, int total) const {
    int fromTop = total - 1 - index;
    Vec2f base { Layout::DESIGN_WIDTH * 0.35f, Layout::PILE_AREA_H / 2.f };
    return { base.x - fromTop * Layout::RESERVE_STACK_OFFSET, base.y };
}

Vec2f GamePresenter::slotPositionFor(CardModel* card) const {
    auto it = _slotIndex.find(card);
    if (it == _slotIndex.end()) return {};
    const SlotDef& slot = level().layout[it->second];
    return { slot.x, slot.y };
}

int GamePresenter::slotZOrderFor(CardModel* card) const {
    auto it = _slotIndex.find(card);
    if (it == _slotIndex.end()) return 0;
    return level().layout[it->second].zOrder;
}

std::vector<CardLayoutSpec> GamePresenter::buildCardLayout() const {
    const auto& m = model();
    std::vector<CardLayoutSpec> layout;

    const auto& hand = m.getHand();
    for (int i = 0; i < (int)hand.size(); ++i) {
        layout.push_back({ hand[i], handPilePosition(), 10 + i, true });
    }

    const auto& reserve = m.getReserve();
    const int rn = (int)reserve.size();
    for (int i = 0; i < rn; ++i) {
        layout.push_back({ reserve[i], reservePositionFor(i, rn), 10 + i, true });
    }

    for (auto* card : m.getTableau()) {
        layout.push_back({ card, slotPositionFor(card), slotZOrderFor(card), true });
    }

    return layout;
}

bool GamePresenter::isRestartButtonVisible() const {
    return model().isTableauClear();
}

CardModel* GamePresenter::pickTableauCard(const std::vector<TableauHitCandidate>& candidates) const {
    if (candidates.empty()) return nullptr;

    auto best = [&](bool requireMatchable) -> CardModel* {
        CardModel* pick = nullptr;
        int bestZ = -1;
        for (const auto& e : candidates) {
            if (requireMatchable && !e.matchable) continue;
            if (e.zOrder > bestZ) {
                bestZ = e.zOrder;
                pick = e.card;
            }
        }
        return pick;
    };

    if (CardModel* v = best(true)) return v;
    return best(false);
}

std::vector<CardModel*> GamePresenter::getDirectBlockers(CardModel* card) const {
    std::vector<CardModel*> blockers;
    if (!card) return blockers;

    auto it = _slotIndex.find(card);
    if (it == _slotIndex.end()) return blockers;

    const int slotIdx = it->second;
    const auto& layout = level().layout;
    if (slotIdx < 0 || slotIdx >= (int)layout.size()) return blockers;

    for (int blockerSlot : layout[slotIdx].blockedBy) {
        for (const auto& kv : _slotIndex) {
            if (kv.second != blockerSlot) continue;
            if (model().getTableauIndex(kv.first) >= 0)
                blockers.push_back(kv.first);
            break;
        }
    }
    return blockers;
}

CardModel* GamePresenter::reserveTopCard() const {
    if (model().getReserveSize() == 0) return nullptr;
    return model().getReserve().back();
}

bool GamePresenter::isReserveTop(CardModel* card) const {
    return card && card == reserveTopCard();
}

bool GamePresenter::canUndo() const {
    return controller().canUndo();
}

PresenterOutcome GamePresenter::onTableauTapped(CardModel* card) {
    PresenterOutcome outcome;
    if (!card || !card->isAccessible()) return outcome;
    if (!controller().tryMatch(card)) return outcome;

    outcome.success    = true;
    outcome.flyingCard = model().getHandTop();
    return outcome;
}

PresenterOutcome GamePresenter::onReserveTapped() {
    PresenterOutcome outcome;
    if (!controller().tryDraw()) return outcome;

    outcome.success    = true;
    outcome.flyingCard = model().getHandTop();
    return outcome;
}

PresenterOutcome GamePresenter::onUndoTapped() {
    PresenterOutcome outcome;
    if (!canUndo()) return outcome;

    outcome.flyingCard = model().getHandTop();
    controller().tryUndo();
    outcome.success = true;
    return outcome;
}

PresenterOutcome GamePresenter::onRestartTapped() {
    restart();
    PresenterOutcome outcome;
    outcome.success     = true;
    outcome.fullRestart = true;
    return outcome;
}
