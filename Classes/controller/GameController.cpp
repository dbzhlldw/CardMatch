// GameController — 游戏规则入口的实现（tryMatch / tryDraw / tryUndo），校验操作是否合法、
// 创建 Command、执行并维护撤销栈
#include "GameController.h"
#include "command/MatchCommand.h"
#include "command/DrawCommand.h"
#include "model/GameModel.h"
#include "model/CardModel.h"
#include <cmath>

GameController::GameController(GameModel& model)
    : _model(&model) {}

void GameController::resetHistory() {
    _history.clear();
    _lastUndone.reset();
}

bool GameController::canMatch(CardModel* a, CardModel* b) const {
    return std::abs(a->getValue() - b->getValue()) == 1;
}

bool GameController::canMatchTableau(CardModel* tableauCard) const {
    auto handTop = _model->getHandTop();
    if (!handTop || !tableauCard->isFaceUp()) return false;
    if (!tableauCard->isAccessible())          return false;
    return canMatch(tableauCard, handTop);
}

MatchCommand* GameController::tryMatch(CardModel* tableauCard) {
    if (!canMatchTableau(tableauCard)) return nullptr;

    int idx = _model->getTableauIndex(tableauCard);
    if (idx < 0) return nullptr;

    auto cmd = std::make_unique<MatchCommand>(_model, tableauCard, idx);
    cmd->execute();
    auto raw = cmd.get();
    _history.push_back(std::move(cmd));
    return raw;
}

DrawCommand* GameController::tryDraw() {
    if (_model->getReserveSize() == 0) return nullptr;

    auto cmd = std::make_unique<DrawCommand>(_model);
    cmd->execute();
    auto raw = cmd.get();
    _history.push_back(std::move(cmd));
    return raw;
}

ICommand* GameController::tryUndo() {
    if (_history.empty()) return nullptr;

    auto& cmd = _history.back();
    cmd->undo();
    _lastUndone = std::move(cmd);
    _history.pop_back();
    return _lastUndone.get();
}
