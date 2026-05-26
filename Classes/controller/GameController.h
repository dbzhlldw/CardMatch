// GameController — 游戏规则入口：校验操作、驱动 Model 变更，并管理撤销
#pragma once
#include "TableauActionResult.h"
#include "cards/TableauCardHandlerRegistry.h"
#include "history/UndoManager.h"

class CardModel;
class GameModel;
class DrawCommand;
class ICommand;

class GameController {
public:
    explicit GameController(GameModel& model);

    bool               canTableauAction(CardModel* tableauCard) const;
    TableauActionResult tryTableauAction(CardModel* tableauCard);

    DrawCommand* tryDraw();
    ICommand*    tryUndo();

    bool canUndo() const;
    void resetHistory();

    UndoManager&       undoManager()       { return _undo; }
    const UndoManager& undoManager() const { return _undo; }

private:
    GameModel*               _model;
    UndoManager              _undo;
    TableauCardHandlerRegistry _tableauHandlers;
};
