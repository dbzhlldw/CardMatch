#include "GameModel.h"
#include <algorithm>
#include <cmath>

GameModel* GameModel::_instance = nullptr;

GameModel::GameModel()
    : _rng(std::random_device{}()) {}

GameModel* GameModel::getInstance() {
    if (!_instance) _instance = new GameModel();
    return _instance;
}

// ---------------------------------------------------------------------------
// 初始化一局
// ---------------------------------------------------------------------------

void GameModel::setupGame(const LayoutDef& layout) {
    clearAll();

    int tableauSize = (int)layout.size(); // e.g., 28

    // 生成 52 张牌规格（花色 × 点数）
    struct CardSpec { Suit suit; int val; };
    std::vector<CardSpec> deck;
    deck.reserve(52);
    for (int s = 0; s < 4; ++s)
        for (int v = 1; v <= 13; ++v)
            deck.push_back({ (Suit)s, v });

    // 洗牌（最多重试 100 次），确保初始手牌与至少一张可操作桌面牌点数相差 1
    // 发牌顺序：deck[0..tableauSize-1] → 桌面，deck[tableauSize] → 手牌顶，其余 → 备用
    for (int attempt = 0; attempt < 100; ++attempt) {
        std::shuffle(deck.begin(), deck.end(), _rng);

        if (tableauSize < (int)deck.size()) {
            int handVal = deck[tableauSize].val;
            bool ok = false;
            for (int i = 0; i < tableauSize && !ok; ++i) {
                if (layout[i].blockedBy.empty()) {            // 底行（可操作）
                    if (std::abs(deck[i].val - handVal) == 1)
                        ok = true;
                }
            }
            if (ok) break; // 满足条件，使用此次洗牌结果
        }
        // 100 次都不满足则用最后一次（极小概率）
    }

    // 创建所有牌
    _allCards.reserve(52);
    auto make = [&](Suit s, int v, bool up) -> CardModel* {
        auto c = std::make_unique<CardModel>(s, v, up);
        auto* raw = c.get();
        _allCards.push_back(std::move(c));
        return raw;
    };

    // 桌面牌：全部正面朝上，遮挡关系只影响能否点击
    _tableau.reserve(tableauSize);
    for (int i = 0; i < tableauSize; ++i) {
        auto* card = make(deck[i].suit, deck[i].val, true); // 全部 faceUp
        card->setBlockerCount((int)layout[i].blockedBy.size());
        _tableau.push_back(card);
    }

    // 构建逆映射：_unlocks[blocker] = 当 blocker 被移走后需要减少计数的牌
    _unlocks.clear();
    for (int i = 0; i < tableauSize; ++i) {
        for (int bi : layout[i].blockedBy) {
            _unlocks[_tableau[bi]].push_back(_tableau[i]);
        }
    }

    // 初始手牌（1 张，面朝上）
    if (tableauSize < (int)deck.size()) {
        _hand.push_back(make(deck[tableauSize].suit, deck[tableauSize].val, true));
    }

    // 备用牌堆：取 6 张，面朝下，按顺序压入（最后一张在顶部）
    const int RESERVE_COUNT = 6;
    for (int i = tableauSize + 1; i < tableauSize + 1 + RESERVE_COUNT && i < (int)deck.size(); ++i) {
        _reserve.push_back(make(deck[i].suit, deck[i].val, false));
    }
}

void GameModel::clearAll() {
    _tableau.clear();
    _hand.clear();
    _reserve.clear();
    _allCards.clear();
    _unlocks.clear();
}

// ---------------------------------------------------------------------------
// 桌面牌操作
// ---------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------
// 遮挡系统
// ---------------------------------------------------------------------------

std::vector<CardModel*> GameModel::onTableauCardRemoved(CardModel* card) {
    std::vector<CardModel*> newlyUnblocked;
    auto it = _unlocks.find(card);
    if (it == _unlocks.end()) return newlyUnblocked;

    for (auto* blocked : it->second) {
        blocked->decrementBlocker();
        if (blocked->isAccessible()) {
            newlyUnblocked.push_back(blocked);
        }
    }
    return newlyUnblocked;
}

void GameModel::onTableauCardRestored(CardModel* card) {
    // 恢复 card 对所有它曾遮挡的牌的遮挡计数
    auto it = _unlocks.find(card);
    if (it != _unlocks.end()) {
        for (auto* blocked : it->second) {
            blocked->incrementBlocker();
        }
    }
}

// ---------------------------------------------------------------------------
// 手牌堆
// ---------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------
// 备用牌堆
// ---------------------------------------------------------------------------

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
