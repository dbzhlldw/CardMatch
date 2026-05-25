#pragma once
#include "data/CardDef.h"

class CardModel {
public:
    CardModel(Suit suit, int value, bool faceUp = true)
        : _suit(suit), _value(value), _faceUp(faceUp) {}

    Suit getSuit()  const { return _suit; }
    int  getValue() const { return _value; }
    bool isFaceUp() const { return _faceUp; }
    void setFaceUp(bool v)  { _faceUp = v; }

    // 遮挡计数：当前还有几张牌压在上面（0 = 可操作）
    int  getBlockerCount()  const { return _blockerCount; }
    bool isAccessible()     const { return _blockerCount == 0; }
    void setBlockerCount(int n)   { _blockerCount = n; }
    void incrementBlocker()       { ++_blockerCount; }
    void decrementBlocker()       { if (_blockerCount > 0) --_blockerCount; }

private:
    Suit _suit;
    int  _value;
    bool _faceUp;
    int  _blockerCount = 0;
};
