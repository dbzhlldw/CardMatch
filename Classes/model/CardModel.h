// CardModel — 单张牌的状态实例：ID、类型、花色、点数、遮挡计数
#pragma once
#include "data/CardDef.h"

class CardModel {
public:
    CardModel(int id, Suit suit, int value, CardKind kind = CardKind::Normal)
        : _id(id), _kind(kind), _suit(suit), _value(value) {}

    int      getId()    const { return _id; }
    CardKind getKind()  const { return _kind; }
    void     setKind(CardKind kind) { _kind = kind; }
    Suit     getSuit()  const { return _suit; }
    int      getValue() const { return _value; }

    int  getBlockerCount()  const { return _blockerCount; }
    bool isAccessible()     const { return _blockerCount == 0; }
    void setBlockerCount(int n)   { _blockerCount = n; }
    void incrementBlocker()       { ++_blockerCount; }
    void decrementBlocker()       { if (_blockerCount > 0) --_blockerCount; }

private:
    int      _id;
    CardKind _kind;
    Suit     _suit;
    int      _value;
    int      _blockerCount = 0;
};
