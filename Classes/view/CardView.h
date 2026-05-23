#pragma once
#include "cocos2d.h"
#include <functional>

class CardModel;

// 单张牌的视图
// 背景 + 数字 + 花色图片拼合而成
class CardView : public cocos2d::Node {
public:
    static const cocos2d::Size CARD_SIZE; // 182 x 282

    static CardView* create(CardModel* model);
    bool init(CardModel* model);

    CardModel* getModel() const { return _model; }

    void setFaceUp(bool faceUp);
    void refreshDisplay();

    void setClickCallback(const std::function<void(CardView*)>& cb);

private:
    CardModel*        _model       = nullptr;
    cocos2d::Sprite*  _background  = nullptr;
    cocos2d::Sprite*  _bigNumber   = nullptr;
    cocos2d::Sprite*  _suitIcon    = nullptr;
    cocos2d::Sprite*  _smallNumber = nullptr;
    cocos2d::Sprite*  _smallSuit   = nullptr;
    cocos2d::Node*    _faceNode    = nullptr; // 正面
    cocos2d::LayerColor* _backNode = nullptr; // 背面

    std::function<void(CardView*)> _clickCallback;

    void buildFace();
    void setupTouch();

    std::string bigNumberPath()   const;
    std::string smallNumberPath() const;
    std::string suitPath()        const;
};
