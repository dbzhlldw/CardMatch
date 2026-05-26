// GameSession — 一局游戏的会话容器：绑定 LevelDef + Model + Controller，生命周期一致
#pragma once
#include "data/LevelDef.h"
#include "model/GameModel.h"
#include "controller/GameController.h"

class GameSession {
public:
    explicit GameSession(const LevelDef& level);

    GameModel&       model()       { return _model; }
    const GameModel& model() const { return _model; }
    GameController&       controller()       { return _controller; }
    const GameController& controller() const { return _controller; }
    const LevelDef&  level() const { return _level; }

    // 同一关卡重新开始（清空 Command 栈并重发牌）
    void restart();

private:
    const LevelDef& _level;
    GameModel       _model;
    GameController  _controller;
};
