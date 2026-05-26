// ITableauCardHandler — 桌面牌点击行为，一种 CardKind 一个实现类
#pragma once
#include <memory>
#include "controller/command/ICommand.h"

class CardModel;
class GameModel;

enum class TableauActionVerdict {
    CanExecute,
    CannotExecute,
};

class ITableauCardHandler {
public:
    virtual ~ITableauCardHandler() = default;

    // 当前能否对这张牌执行操作（用于 NoMatch 反馈，不修改 Model）
    virtual TableauActionVerdict preview(const CardModel& card,
                                         const GameModel& model) const = 0;

    // 创建可撤销操作（由 GameController 负责 execute + 压入 UndoManager）
    virtual std::unique_ptr<ICommand> createCommand(GameModel& model,
                                                    CardModel* card) const = 0;
};
