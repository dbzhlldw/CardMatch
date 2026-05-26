// GameController — 游戏规则入口：校验操作、驱动 Model 变更，并管理撤销
#include "GameController.h"
#include "command/DrawCommand.h"
#include "model/GameModel.h"
#include "model/CardModel.h"

GameController::GameController(GameModel& model)
    : _model(&model) {}

void GameController::resetHistory() {
    _undo.clear();
}

bool GameController::canUndo() const {
    return _undo.canUndo();
}

bool GameController::canTableauAction(CardModel* card) const {
    if (!card) return false;
    ITableauCardHandler* handler = _tableauHandlers.handlerFor(card->getKind());
    if (!handler) return false;
    return handler->preview(*card, *_model) == TableauActionVerdict::CanExecute;
}

TableauActionResult GameController::tryTableauAction(CardModel* card) {
    TableauActionResult result;
    if (!card) return result;

    ITableauCardHandler* handler = _tableauHandlers.handlerFor(card->getKind());
    if (!handler) return result;
    if (handler->preview(*card, *_model) != TableauActionVerdict::CanExecute)
        return result;

    auto cmd = handler->createCommand(*_model, card);
    if (!cmd) return result;

    cmd->execute();
    _undo.record(std::move(cmd));
    result.success = true;
    return result;
}

DrawCommand* GameController::tryDraw() {
    if (_model->getReserveSize() == 0) return nullptr;

    auto cmd = std::make_unique<DrawCommand>(_model);
    cmd->execute();
    auto raw = cmd.get();
    _undo.record(std::move(cmd));
    return raw;
}

ICommand* GameController::tryUndo() {
    return _undo.undo();
}
