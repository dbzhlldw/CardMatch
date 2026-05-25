#pragma once
#include <vector>
#include <memory>
#include "command/ICommand.h"

class CardModel;
class GameModel;
class MatchCommand;
class DrawCommand;

class GameController {
public:
    static GameController* getInstance();

    void init();

    // 尝试将桌面牌匹配到手牌堆，成功返回命令指针，失败返回 nullptr
    MatchCommand* tryMatch(CardModel* tableauCard);

    // 尝试从备用牌堆抽牌，成功返回命令指针，失败返回 nullptr
    DrawCommand* tryDraw();

    // 撤回最近一步，返回被撤回的命令，无可撤则返回 nullptr
    // 返回的指针会在下一次 tryUndo/tryMatch/tryDraw 后失效
    ICommand* tryUndo();

    bool canUndo() const { return !_history.empty(); }

    // 当前手牌顶是否可与该桌面牌匹配（含遮挡/面朝上检查）
    bool canMatchTableau(CardModel* tableauCard) const;

private:
    GameController() = default;
    static GameController* _instance;

    GameModel* _model = nullptr;
    std::vector<std::unique_ptr<ICommand>> _history;
    std::unique_ptr<ICommand> _lastUndone; // 保持 tryUndo 返回指针的生命期

    bool canMatch(CardModel* a, CardModel* b) const;
};
