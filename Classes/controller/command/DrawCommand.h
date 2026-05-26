// DrawCommand — 备用牌堆顶牌入手牌堆的实现
#pragma once
#include "ICommand.h"

class CardModel;
class GameModel;

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
