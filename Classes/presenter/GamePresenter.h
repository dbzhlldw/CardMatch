// GamePresenter — 界面与游戏内核的隔离层：GameScene 不直接访问 Model/Controller，只通过本类处理点击并获取显示数据
#pragma once
#include <unordered_map>
#include "presenter/ViewTypes.h"
#include "session/GameSession.h"

class CardModel;

class GamePresenter {
public:
    struct Layout {
        static constexpr float DESIGN_WIDTH         = 1080.f;
        static constexpr float DESIGN_HEIGHT        = 2080.f;
        static constexpr float PILE_AREA_H          = 580.f;
        static constexpr float TABLEAU_AREA_H       = DESIGN_HEIGHT - PILE_AREA_H;
        static constexpr float RESERVE_STACK_OFFSET = 15.f;
    };

    explicit GamePresenter(const LevelDef& level);

    ViewState buildViewState() const;

    PresenterOutcome onTableauTapped(int cardId);
    PresenterOutcome onReserveTapped();
    PresenterOutcome onUndoTapped();
    PresenterOutcome onRestartTapped();

private:
    GameSession _session;
    std::unordered_map<int, int> _slotIndex; // cardId → layout 槽位

    GameModel&            model();
    const GameModel&      model() const;
    GameController&       controller();
    const GameController& controller() const;
    const LevelDef&       level() const;

    void restart();
    void registerTableauSlotsFromModel();
    void clearSlotRegistry();

    CardModel* cardById(int cardId) const;
    CardViewSpec makeSpec(CardModel* card, CardPile pile, int stackIndex, int stackTotal) const;
    std::vector<CardModel*> getDirectBlockers(CardModel* card) const;

    Vec2f handPilePosition() const;
    Vec2f undoButtonPosition() const;
    Vec2f restartButtonPosition() const;
    Vec2f reservePositionFor(int index, int total) const;
    Vec2f slotPositionFor(int slotIndex) const;
    int   slotZOrderFor(int slotIndex) const;
};
