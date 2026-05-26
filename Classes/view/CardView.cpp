// CardView — 单张牌的 cocos 视图，外观由 CardAppearance 配置
#include "CardView.h"
#include "data/CardAppearance.h"

USING_NS_CC;

const Size CardView::CARD_SIZE = Size(182, 282);

CardView* CardView::create(int cardId, CardKind kind, Suit suit, int value) {
    auto view = new (std::nothrow) CardView();
    if (view && view->init(cardId, kind, suit, value)) {
        view->autorelease();
        return view;
    }
    CC_SAFE_DELETE(view);
    return nullptr;
}

bool CardView::init(int cardId, CardKind kind, Suit suit, int value) {
    if (!Node::init()) return false;
    _cardId = cardId;
    _kind   = kind;
    _suit   = suit;
    _value  = value;
    setContentSize(CARD_SIZE);

    buildFace();
    return true;
}

void CardView::buildFace() {
    const CardVisualStyle& style = cardVisualStyleFor(_kind);

    _background = Sprite::create("res/card_general.png");
    _background->setPosition(CARD_SIZE.width / 2, CARD_SIZE.height / 2);
    addChild(_background, 0);

    if (style.useStandardPips) {
        _bigNumber = Sprite::create(bigNumberPath());
        if (_bigNumber) {
            _bigNumber->setPosition(CARD_SIZE.width / 2, CARD_SIZE.height / 2 - 30);
            addChild(_bigNumber, 1);
        }

        _smallNumber = Sprite::create(smallNumberPath());
        if (_smallNumber) {
            _smallNumber->setPosition(38, CARD_SIZE.height - 40);
            addChild(_smallNumber, 2);
        }

        _smallSuit = Sprite::create(suitPath());
        if (_smallSuit) {
            _smallSuit->setScale(1.0f);
            _smallSuit->setPosition(CARD_SIZE.width - 38, CARD_SIZE.height - 40);
            addChild(_smallSuit, 2);
        }
    }

    if (style.overlayImage) {
        _overlay = Sprite::create(style.overlayImage);
        if (_overlay) {
            _overlay->setPosition(CARD_SIZE.width / 2, CARD_SIZE.height / 2);
            addChild(_overlay, 3);
        }
    }
}

std::string CardView::bigNumberPath() const {
    std::string color = isRedSuit(_suit) ? "red" : "black";
    return "res/number/big_" + color + "_" + valueToString(_value) + ".png";
}

std::string CardView::smallNumberPath() const {
    std::string color = isRedSuit(_suit) ? "red" : "black";
    return "res/number/small_" + color + "_" + valueToString(_value) + ".png";
}

std::string CardView::suitPath() const {
    return "res/suits/" + suitToString(_suit) + ".png";
}
