// GameSession — 一局游戏的会话容器：绑定 LevelDef + Model + Controller，生命周期一致
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
