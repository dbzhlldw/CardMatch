// MatchTableauHandler — 普通牌的匹配规则
#pragma once
#include "ITableauCardHandler.h"

class MatchTableauHandler : public ITableauCardHandler {
public:
    TableauActionVerdict preview(const CardModel& card,
                                 const GameModel& model) const override;
    std::unique_ptr<ICommand> createCommand(GameModel& model,
                                            CardModel* card) const override;

private:
    static bool valuesMatch(const CardModel& a, const CardModel& b);
};
