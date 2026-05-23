#include "MatchCommand.h"
#include "model/GameModel.h"

MatchCommand::MatchCommand(GameModel* model, CardModel* card, int tableauIndex)
    : _model(model), _card(card), _tableauIndex(tableauIndex) {}

void MatchCommand::execute() {
    _model->removeFromTableau(_tableauIndex);
    _model->pushHand(_card);
}

void MatchCommand::undo() {
    _model->popHand();
    _model->insertToTableau(_tableauIndex, _card);
}
