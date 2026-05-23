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

private:
    Suit _suit;
    int  _value;
    bool _faceUp;
};
