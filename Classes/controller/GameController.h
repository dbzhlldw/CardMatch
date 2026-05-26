// GameController — 游戏规则入口：校验操作、创建并执行 Command、维护撤销历史栈
#pragma once
#include <vector>
#include <memory>
#include "command/ICommand.h"

class CardModel;
class GameModel;
class MatchCommand;
class DrawCommand;

class GameController {
public:
    explicit GameController(GameModel& model);

    MatchCommand* tryMatch(CardModel* tableauCard);
    DrawCommand*  tryDraw();
    ICommand*     tryUndo();

    bool canUndo() const { return !_history.empty(); }
    bool canMatchTableau(CardModel* tableauCard) const;

    void resetHistory();

private:
    GameModel* _model;
    std::vector<std::unique_ptr<ICommand>> _history;
    std::unique_ptr<ICommand> _lastUndone;

    bool canMatch(CardModel* a, CardModel* b) const;
};
