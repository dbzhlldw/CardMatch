// GamePresenter — 界面与游戏内核的隔离层：GameScene 不直接访问 Model/Controller，只通过本类处理点击并获取显示数据
#include "GamePresenter.h"
#include "model/GameModel.h"
#include "model/CardModel.h"
#include "controller/GameController.h"

GamePresenter::GamePresenter(const LevelDef& level)
    : _session(level) {
    registerTableauSlotsFromModel();
}

GameModel& GamePresenter::model() { return _session.model(); }
const GameModel& GamePresenter::model() const { return _session.model(); }
GameController& GamePresenter::controller() { return _session.controller(); }
const GameController& GamePresenter::controller() const { return _session.controller(); }
const LevelDef& GamePresenter::level() const { return _session.level(); }

void GamePresenter::restart() {
    _session.restart();
    clearSlotRegistry();
    registerTableauSlotsFromModel();
}

void GamePresenter::registerTableauSlotsFromModel() {
    const auto& tableau = model().getTableau();
    for (int i = 0; i < (int)tableau.size(); ++i)
        _slotIndex[tableau[i]->getId()] = i;
}

void GamePresenter::clearSlotRegistry() {
    _slotIndex.clear();
}

CardModel* GamePresenter::cardById(int cardId) const {
    return model().getCardById(cardId);
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

Vec2f GamePresenter::slotPositionFor(int slotIndex) const {
    if (slotIndex < 0 || slotIndex >= (int)level().layout.size()) return {};
    const SlotDef& slot = level().layout[slotIndex];
    return { slot.x, slot.y };
}

int GamePresenter::slotZOrderFor(int slotIndex) const {
    if (slotIndex < 0 || slotIndex >= (int)level().layout.size()) return 0;
    return level().layout[slotIndex].zOrder;
}

CardViewSpec GamePresenter::makeSpec(CardModel* card, CardPile pile,
                                     int stackIndex, int stackTotal) const {
    CardViewSpec spec;
    spec.cardId = card->getId();
    spec.kind   = card->getKind();
    spec.suit   = card->getSuit();
    spec.value  = card->getValue();
    spec.pile   = pile;

    auto slotIt = _slotIndex.find(card->getId());
    if (slotIt != _slotIndex.end())
        spec.slotIndex = slotIt->second;

    if (pile == CardPile::Tableau && slotIt != _slotIndex.end()) {
        spec.position = slotPositionFor(spec.slotIndex);
        spec.zOrder   = slotZOrderFor(spec.slotIndex);
    } else if (pile == CardPile::Hand) {
        spec.position = handPilePosition();
        spec.zOrder   = 10 + stackIndex;
    } else {
        spec.position = reservePositionFor(stackIndex, stackTotal);
        spec.zOrder   = 10 + stackIndex;
    }

    return spec;
}

ViewState GamePresenter::buildViewState() const {
    ViewState state;
    state.undoButtonPosition    = undoButtonPosition();
    state.restartButtonPosition = restartButtonPosition();
    state.showRestartButton     = model().isTableauClear();
    state.canUndo               = controller().canUndo();

    const auto& hand = model().getHand();
    for (int i = 0; i < (int)hand.size(); ++i)
        state.cards.push_back(makeSpec(hand[i], CardPile::Hand, i, (int)hand.size()));

    const auto& reserve = model().getReserve();
    const int reserveCount = (int)reserve.size();
    for (int i = 0; i < reserveCount; ++i)
        state.cards.push_back(makeSpec(reserve[i], CardPile::Reserve, i, reserveCount));

    if (reserveCount > 0)
        state.reserveTopCardId = reserve.back()->getId();

    for (auto* card : model().getTableau())
        state.cards.push_back(makeSpec(card, CardPile::Tableau, 0, 0));

    return state;
}

std::vector<CardModel*> GamePresenter::getDirectBlockers(CardModel* card) const {
    std::vector<CardModel*> blockers;
    if (!card) return blockers;

    auto it = _slotIndex.find(card->getId());
    if (it == _slotIndex.end()) return blockers;

    const int slotIdx = it->second;
    const auto& layout = level().layout;
    if (slotIdx < 0 || slotIdx >= (int)layout.size()) return blockers;

    for (int blockerSlot : layout[slotIdx].blockedBy) {
        for (const auto& kv : _slotIndex) {
            if (kv.second != blockerSlot) continue;
            if (model().getTableauIndex(cardById(kv.first)) >= 0)
                blockers.push_back(cardById(kv.first));
            break;
        }
    }
    return blockers;
}

PresenterOutcome GamePresenter::onTableauTapped(int cardId) {
    PresenterOutcome outcome;
    CardModel* card = cardById(cardId);
    if (!card) return outcome;

    outcome.primaryCardId = cardId;

    if (!card->isAccessible()) {
        outcome.tableauResult = TableauTapResult::Blocked;
        for (CardModel* blocker : getDirectBlockers(card))
            outcome.highlightCardIds.push_back(blocker->getId());
        return outcome;
    }

    if (!controller().canTableauAction(card)) {
        outcome.tableauResult = TableauTapResult::NoMatch;
        return outcome;
    }

    if (!controller().tryTableauAction(card).success) return outcome;

    outcome.tableauResult = TableauTapResult::Matched;
    outcome.success       = true;
    outcome.flyingCardId  = model().getHandTop()->getId();
    return outcome;
}

PresenterOutcome GamePresenter::onReserveTapped() {
    PresenterOutcome outcome;
    if (!controller().tryDraw()) return outcome;

    outcome.success      = true;
    outcome.flyingCardId = model().getHandTop()->getId();
    return outcome;
}

PresenterOutcome GamePresenter::onUndoTapped() {
    PresenterOutcome outcome;
    if (!controller().canUndo()) return outcome;

    outcome.flyingCardId = model().getHandTop()->getId();
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
