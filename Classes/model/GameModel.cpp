#include "GameModel.h"
#include <algorithm>

GameModel* GameModel::_instance = nullptr;

GameModel* GameModel::getInstance() {
    if (!_instance) _instance = new GameModel();
    return _instance;
}

void GameModel::setupGame() {
    clearAll();

    auto make = [&](Suit s, int v, bool up = true) -> CardModel* {
        auto c = new CardModel(s, v, up);
        _allCards.emplace_back(c);
        return c;
    };

    // 暂时用固定的牌测试
    // 手牌堆初始顶牌
    _hand.push_back(make(Suit::Club, 4));

    // 桌面牌
    _tableau.push_back(make(Suit::Diamond, 3));
    _tableau.push_back(make(Suit::Spade,   2));
    _tableau.push_back(make(Suit::Club,    6));

    // 备用牌堆
    _reserve.push_back(make(Suit::Club,    8, false));
    _reserve.push_back(make(Suit::Heart,   1, false));
}

void GameModel::clearAll() {
    _tableau.clear();
    _hand.clear();
    _reserve.clear();
    _allCards.clear();
}

void GameModel::removeFromTableau(int index) {
    _tableau.erase(_tableau.begin() + index);
}

void GameModel::insertToTableau(int index, CardModel* card) {
    _tableau.insert(_tableau.begin() + index, card);
}

int GameModel::getTableauIndex(CardModel* card) const {
    for (int i = 0; i < (int)_tableau.size(); i++) {
        if (_tableau[i] == card) return i;
    }
    return -1;
}

CardModel* GameModel::getHandTop() const {
    return _hand.empty() ? nullptr : _hand.back();
}

void GameModel::pushHand(CardModel* card) {
    card->setFaceUp(true);
    _hand.push_back(card);
}

CardModel* GameModel::popHand() {
    if (_hand.empty()) return nullptr;
    auto card = _hand.back();
    _hand.pop_back();
    return card;
}

CardModel* GameModel::getReserveTop() const {
    return _reserve.empty() ? nullptr : _reserve.back();
}

CardModel* GameModel::drawFromReserve() {
    if (_reserve.empty()) return nullptr;
    auto card = _reserve.back();
    _reserve.pop_back();
    card->setFaceUp(true);
    return card;
}

void GameModel::pushToReserve(CardModel* card) {
    card->setFaceUp(false);
    _reserve.push_back(card);
}
