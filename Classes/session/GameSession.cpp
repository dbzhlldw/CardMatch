// GameSession — 构造时发牌、restart 时清 Command 栈并重发牌
#include "GameSession.h"

GameSession::GameSession(const LevelDef& level)
    : _level(level)
    , _controller(_model)
{
    _model.setupGame(_level);
}

void GameSession::restart() {
    _controller.resetHistory();
    _model.setupGame(_level);
}
