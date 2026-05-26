// ICommand — 可撤销操作的抽象接口（execute / undo），供 Command 模式使用
#pragma once

class ICommand {
public:
    virtual ~ICommand() = default;
    virtual void execute() = 0;
    virtual void undo()    = 0;
};
