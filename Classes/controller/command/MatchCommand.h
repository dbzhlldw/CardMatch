#pragma once
#include "ICommand.h"

class CardModel;
class GameModel;

// 将桌面牌移动到手牌堆顶部（点击匹配操作）
class MatchCommand : public ICommand {
public:
    MatchCommand(GameModel* model, CardModel* card, int tableauIndex);

    void execute() override;
    void undo()    override;

    CardModel* getCard() const       { return _card; }
    int        getTableauIndex() const { return _tableauIndex; }

private:
    GameModel* _model;
    CardModel* _card;
    int        _tableauIndex; // 撤回时重新插回该位置
};
