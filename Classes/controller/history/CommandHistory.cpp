// CommandHistory — 逐步撤销栈的实现
#include "CommandHistory.h"

void CommandHistory::push(std::unique_ptr<ICommand> cmd) {
    if (!cmd) return;
    _stack.push_back(std::move(cmd));
}

ICommand* CommandHistory::undoLast() {
    if (_stack.empty()) return nullptr;

    auto& cmd = _stack.back();
    cmd->undo();
    _lastUndone = std::move(cmd);
    _stack.pop_back();
    return _lastUndone.get();
}

void CommandHistory::clear() {
    _stack.clear();
    _lastUndone.reset();
}
