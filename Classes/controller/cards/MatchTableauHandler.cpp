// MatchTableauHandler — 普通牌的匹配规则
#include "MatchTableauHandler.h"
#include "controller/command/MatchCommand.h"
#include "model/GameModel.h"
#include "model/CardModel.h"
#include <cmath>

bool MatchTableauHandler::valuesMatch(const CardModel& a, const CardModel& b) {
    return std::abs(a.getValue() - b.getValue()) == 1;
}

TableauActionVerdict MatchTableauHandler::preview(const CardModel& card,
                                                  const GameModel& model) const {
    const CardModel* handTop = model.getHandTop();
    if (!handTop || !card.isAccessible()) return TableauActionVerdict::CannotExecute;
    return valuesMatch(card, *handTop) ? TableauActionVerdict::CanExecute
                                       : TableauActionVerdict::CannotExecute;
}

std::unique_ptr<ICommand> MatchTableauHandler::createCommand(GameModel& model,
                                                             CardModel* card) const {
    int idx = model.getTableauIndex(card);
    if (idx < 0) return nullptr;
    return std::make_unique<MatchCommand>(&model, card, idx);
}
