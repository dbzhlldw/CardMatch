// LevelDef — 关卡配方类型：引用一套 LayoutDef，并定义发牌数、可解性等规则
#pragma once
#include "LayoutDef.h"

struct LevelDef {
    const char*    id;
    const LayoutDef& layout;
    int            reserveCount;
    int            initialHandCount;
    bool           requireSolvable;
    int            solvableMaxAttempts;

    LevelDef(const char* id,
             const LayoutDef& layout,
             int reserveCount,
             int initialHandCount     = 1,
             bool requireSolvable     = true,
             int solvableMaxAttempts  = 1000)
        : id(id)
        , layout(layout)
        , reserveCount(reserveCount)
        , initialHandCount(initialHandCount)
        , requireSolvable(requireSolvable)
        , solvableMaxAttempts(solvableMaxAttempts) {}

    int cardsInPlay() const {
        return (int)layout.size() + initialHandCount + reserveCount;
    }
};
