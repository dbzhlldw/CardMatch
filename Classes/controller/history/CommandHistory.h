// CommandHistory — 基于 ICommand 的逐步撤销栈
#pragma once
#include <memory>
#include <vector>
#include "controller/command/ICommand.h"

class CommandHistory {
public:
    void push(std::unique_ptr<ICommand> cmd);
    ICommand* undoLast();
    bool      canUndo() const { return !_stack.empty(); }
    void      clear();
    size_t    size() const { return _stack.size(); }

private:
    std::vector<std::unique_ptr<ICommand>> _stack;
    std::unique_ptr<ICommand>              _lastUndone;
};
