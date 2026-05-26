// MatchCommand — 桌面牌移入手牌堆，并更新遮挡解锁状态
#include "MatchCommand.h"
#include "model/GameModel.h"

MatchCommand::MatchCommand(GameModel* model, CardModel* card, int tableauIndex)
    : _model(model), _card(card), _tableauIndex(tableauIndex) {}

void MatchCommand::execute() {
    // 先更新遮挡计数，记录哪些牌因此被解锁
    _newlyUnblocked = _model->onTableauCardRemoved(_card);
    // 再从桌面移走，压入手牌堆
    _model->removeFromTableau(_tableauIndex);
    _model->pushHand(_card);
}

void MatchCommand::undo() {
    _model->popHand();
    _model->insertToTableau(_tableauIndex, _card);
    // 恢复遮挡计数
    _model->onTableauCardRestored(_card);
}
