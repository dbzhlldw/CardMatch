#include "CardView.h"
#include "model/CardModel.h"
#include "data/CardDef.h"

USING_NS_CC;

const Size CardView::CARD_SIZE = Size(182, 282);

CardView* CardView::create(CardModel* model) {
    auto view = new (std::nothrow) CardView();
    if (view && view->init(model)) {
        view->autorelease();
        return view;
    }
    CC_SAFE_DELETE(view);
    return nullptr;
}

bool CardView::init(CardModel* model) {
    if (!Node::init()) return false;
    _model = model;
    setContentSize(CARD_SIZE);

    buildFace();
    setupTouch();
    refreshDisplay();
    return true;
}

void CardView::buildFace() {
    // --- 正面 ---
    _faceNode = Node::create();
    _faceNode->setContentSize(CARD_SIZE);
    addChild(_faceNode, 0);

    _background = Sprite::create("res/card_general.png");
    _background->setPosition(CARD_SIZE.width / 2, CARD_SIZE.height / 2);
    _faceNode->addChild(_background, 0);

    // 大数字
    _bigNumber = Sprite::create(bigNumberPath());
    if (_bigNumber) {
        _bigNumber->setPosition(CARD_SIZE.width / 2, CARD_SIZE.height / 2 + 20);
        _faceNode->addChild(_bigNumber, 1);
    }

    // 大花色
    _suitIcon = Sprite::create(suitPath());
    if (_suitIcon) {
        _suitIcon->setPosition(CARD_SIZE.width / 2, CARD_SIZE.height / 2 - 65);
        _faceNode->addChild(_suitIcon, 1);
    }

    // 左上角小数字
    _smallNumber = Sprite::create(smallNumberPath());
    if (_smallNumber) {
        _smallNumber->setPosition(28, CARD_SIZE.height - 28);
        _faceNode->addChild(_smallNumber, 2);
    }

    // 左上角小花色
    _smallSuit = Sprite::create(suitPath());
    if (_smallSuit) {
        _smallSuit->setScale(0.7f);
        _smallSuit->setPosition(28, CARD_SIZE.height - 60);
        _faceNode->addChild(_smallSuit, 2);
    }

    // --- 背面 ---
    _backNode = LayerColor::create(Color4B(30, 80, 200, 230),
                                   CARD_SIZE.width, CARD_SIZE.height);
    addChild(_backNode, 1);
}

void CardView::refreshDisplay() {
    bool up = _model->isFaceUp();
    _faceNode->setVisible(up);
    _backNode->setVisible(!up);
}

void CardView::setFaceUp(bool faceUp) {
    _model->setFaceUp(faceUp);
    refreshDisplay();
}

void CardView::setClickCallback(const std::function<void(CardView*)>& cb) {
    _clickCallback = cb;
}

void CardView::setupTouch() {
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);

    listener->onTouchBegan = [this](Touch* touch, Event*) -> bool {
        auto local = convertToNodeSpace(touch->getLocation());
        Rect bounds(0, 0, CARD_SIZE.width, CARD_SIZE.height);
        return bounds.containsPoint(local);
    };

    listener->onTouchEnded = [this](Touch* touch, Event*) {
        auto local = convertToNodeSpace(touch->getLocation());
        Rect bounds(0, 0, CARD_SIZE.width, CARD_SIZE.height);
        if (bounds.containsPoint(local) && _clickCallback) {
            _clickCallback(this);
        }
    };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

std::string CardView::bigNumberPath() const {
    std::string color = isRedSuit(_model->getSuit()) ? "red" : "black";
    return "res/number/big_" + color + "_" + valueToString(_model->getValue()) + ".png";
}

std::string CardView::smallNumberPath() const {
    std::string color = isRedSuit(_model->getSuit()) ? "red" : "black";
    return "res/number/small_" + color + "_" + valueToString(_model->getValue()) + ".png";
}

std::string CardView::suitPath() const {
    return "res/suits/" + suitToString(_model->getSuit()) + ".png";
}
