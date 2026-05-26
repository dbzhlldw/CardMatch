// CardView — 单张牌的 cocos 视图，外观由 CardAppearance 配置
#pragma once
#include "cocos2d.h"
#include "data/CardDef.h"

class CardView : public cocos2d::Node {
public:
    static const cocos2d::Size CARD_SIZE; // 182 x 282

    static CardView* create(int cardId, CardKind kind, Suit suit, int value);
    bool init(int cardId, CardKind kind, Suit suit, int value);

    int getCardId() const { return _cardId; }

private:
    int      _cardId = -1;
    CardKind _kind   = CardKind::Normal;
    Suit     _suit   = Suit::Heart;
    int      _value  = 1;

    cocos2d::Sprite* _background  = nullptr;
    cocos2d::Sprite* _bigNumber   = nullptr;
    cocos2d::Sprite* _smallNumber = nullptr;
    cocos2d::Sprite* _smallSuit   = nullptr;
    cocos2d::Sprite* _overlay     = nullptr;

    void buildFace();

    std::string bigNumberPath()   const;
    std::string smallNumberPath() const;
    std::string suitPath()        const;
};
