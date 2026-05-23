#include "DrawCommand.h"
#include "model/GameModel.h"

DrawCommand::DrawCommand(GameModel* model)
    : _model(model), _drawnCard(nullptr) {}

void DrawCommand::execute() {
    _drawnCard = _model->drawFromReserve();
    _model->pushHand(_drawnCard);
}

void DrawCommand::undo() {
    _model->popHand();
    _model->pushToReserve(_drawnCard);
}
