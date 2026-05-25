#pragma once
#include "ICommand.h"
#include <vector>

class CardModel;
class GameModel;

// 将桌面牌移动到手牌堆顶部（点击匹配操作）
class MatchCommand : public ICommand {
public:
    MatchCommand(GameModel* model, CardModel* card, int tableauIndex);

    void execute() override;
    void undo()    override;

    CardModel* getCard()         const { return _card; }
    int        getTableauIndex() const { return _tableauIndex; }

    // execute() 执行后，新变为可操作（被解锁）的桌面牌列表
    const std::vector<CardModel*>& getNewlyUnblocked() const { return _newlyUnblocked; }

private:
    GameModel* _model;
    CardModel* _card;
    int        _tableauIndex; // 撤回时重新插回该位置

    std::vector<CardModel*> _newlyUnblocked; // execute 时记录，undo 时恢复
};
