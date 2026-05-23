#include "GameController.h"
#include "command/MatchCommand.h"
#include "command/DrawCommand.h"
#include "model/GameModel.h"
#include "model/CardModel.h"
#include <cmath>

GameController* GameController::_instance = nullptr;

GameController* GameController::getInstance() {
    if (!_instance) _instance = new GameController();
    return _instance;
}

void GameController::init() {
    _model = GameModel::getInstance();
    _history.clear();
}

bool GameController::canMatch(CardModel* a, CardModel* b) const {
    return std::abs(a->getValue() - b->getValue()) == 1;
}

MatchCommand* GameController::tryMatch(CardModel* tableauCard) {
    auto handTop = _model->getHandTop();
    if (!handTop || !tableauCard->isFaceUp()) return nullptr;
    if (!canMatch(tableauCard, handTop))       return nullptr;

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
    _lastUndone = std::move(cmd); // 把裸指针暂存在 _lastUndone 里，调用方在下次操作前有效
    _history.pop_back();
    return _lastUndone.get();
}
