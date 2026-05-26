// UndoManager — 撤销门面，扩展新撤销类型时在此聚合多种后端
#pragma once
#include <memory>
#include "CommandHistory.h"
#include "controller/command/ICommand.h"

class UndoManager {
public:
    void record(std::unique_ptr<ICommand> cmd);
    ICommand* undo();
    bool      canUndo() const;
    void      clear();

    CommandHistory&       commandHistory()       { return _commandHistory; }
    const CommandHistory& commandHistory() const { return _commandHistory; }

private:
    CommandHistory _commandHistory;
};
