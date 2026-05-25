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

    // 大数字（居中偏下，不再配大花色）
    _bigNumber = Sprite::create(bigNumberPath());
    if (_bigNumber) {
        _bigNumber->setPosition(CARD_SIZE.width / 2, CARD_SIZE.height / 2 - 30);
        _faceNode->addChild(_bigNumber, 1);
    }

    // 左上角小数字（带 padding）
    // 小数字图片 38×46，anchor 默认 0.5；x=38 距左边约 19px，y 距顶边约 18px
    _smallNumber = Sprite::create(smallNumberPath());
    if (_smallNumber) {
        _smallNumber->setPosition(38, CARD_SIZE.height - 40);
        _faceNode->addChild(_smallNumber, 2);
    }

    // 右上角小花色（唯一的花色元素）
    // 花色图片 43×43，scale=1.0 → 43×43，与左上角小数字（38×46）接近
    _smallSuit = Sprite::create(suitPath());
    if (_smallSuit) {
        _smallSuit->setScale(1.0f);
        _smallSuit->setPosition(CARD_SIZE.width - 38, CARD_SIZE.height - 40);
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
        // 没有点击回调（被遮挡）时直接放行，不拦截触摸，让下层牌能收到事件
        if (!_clickCallback) return false;
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
