#pragma once
#include "ICommand.h"

class CardModel;
class GameModel;

// 从备用牌堆抽一张牌到手牌堆顶部
class DrawCommand : public ICommand {
public:
    explicit DrawCommand(GameModel* model);

    void execute() override;
    void undo()    override;

    CardModel* getDrawnCard() const { return _drawnCard; }

private:
    GameModel* _model;
    CardModel* _drawnCard; // execute 时记录，undo 时用来放回去
};
