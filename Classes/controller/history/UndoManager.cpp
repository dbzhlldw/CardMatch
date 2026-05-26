// UndoManager — 撤销门面，扩展新撤销类型时在此聚合多种后端
#include "UndoManager.h"

void UndoManager::record(std::unique_ptr<ICommand> cmd) {
    _commandHistory.push(std::move(cmd));
}

ICommand* UndoManager::undo() {
    return _commandHistory.undoLast();
}

bool UndoManager::canUndo() const {
    return _commandHistory.canUndo();
}

void UndoManager::clear() {
    _commandHistory.clear();
}
