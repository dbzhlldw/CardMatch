// GameModel — 一局游戏的牌堆状态：桌面/手牌/备用堆、发牌、遮挡、可解性验证
#include "GameModel.h"
#include <algorithm>
#include <cmath>
#include <queue>
#include <unordered_set>
#include <tuple>

GameModel::GameModel()
    : _rng(std::random_device{}()) {}

// ---------------------------------------------------------------------------
// 可解性验证
// ---------------------------------------------------------------------------
static bool isSolvable(
    const std::vector<int>&              tableauVals,
    const std::vector<std::vector<int>>& blockedBy,
    int                                  handVal,
    const std::vector<int>&              reserveVals)
{
    int n  = (int)tableauVals.size();
    int rn = (int)reserveVals.size();

    auto encode = [](int mask, int v, int d) -> int {
        return mask | ((v - 1) << 10) | (d << 14);
    };

    std::unordered_set<int> visited;
    std::queue<std::tuple<int,int,int>> q;

    int initMask = (1 << n) - 1;
    q.push({initMask, handVal, rn});
    visited.insert(encode(initMask, handVal, rn));

    while (!q.empty()) {
        auto [mask, hv, rd] = q.front();
        q.pop();

        if (mask == 0) return true;

        for (int i = 0; i < n; ++i) {
            if (!(mask & (1 << i))) continue;
            bool accessible = true;
            for (int b : blockedBy[i])
                if (mask & (1 << b)) { accessible = false; break; }
            if (!accessible) continue;
            if (std::abs(tableauVals[i] - hv) != 1) continue;

            int nm = mask & ~(1 << i);
            int nh = tableauVals[i];
            int s  = encode(nm, nh, rd);
            if (!visited.count(s)) { visited.insert(s); q.push({nm, nh, rd}); }
        }

        if (rd > 0) {
            int nh = reserveVals[rd - 1];
            int s  = encode(mask, nh, rd - 1);
            if (!visited.count(s)) { visited.insert(s); q.push({mask, nh, rd - 1}); }
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// 初始化一局
// ---------------------------------------------------------------------------

void GameModel::setupGame(const LevelDef& level) {
    clearAll();
    _currentLevel = &level;

    const LayoutDef& layout = level.layout;
    int tableauSize   = (int)layout.size();
    int reserveCount  = level.reserveCount;
    int initialHand   = level.initialHandCount;

    std::vector<std::vector<int>> blockedBy(tableauSize);
    for (int i = 0; i < tableauSize; ++i)
        blockedBy[i] = layout[i].blockedBy;

    struct CardSpec { Suit suit; int val; };
    std::vector<CardSpec> deck;
    deck.reserve(52);
    for (int s = 0; s < 4; ++s)
        for (int v = 1; v <= 13; ++v)
            deck.push_back({ (Suit)s, v });

    // 发牌顺序：
    //   deck[0 .. tableauSize-1]                       → 桌面
    //   deck[tableauSize .. tableauSize+initialHand-1] → 初始手牌
    //   deck[tableauSize+initialHand .. ]              → 备用堆
    auto buildDealValues = [&]() {
        std::vector<int> tableauVals(tableauSize);
        for (int i = 0; i < tableauSize; ++i)
            tableauVals[i] = deck[i].val;

        int handVal = deck[tableauSize + initialHand - 1].val;

        std::vector<int> reserveVals(reserveCount);
        for (int i = 0; i < reserveCount; ++i)
            reserveVals[i] = deck[tableauSize + initialHand + i].val;

        return std::make_tuple(tableauVals, handVal, reserveVals);
    };

    if (level.requireSolvable) {
        for (;;) {
            std::shuffle(deck.begin(), deck.end(), _rng);

            auto [tableauVals, handVal, reserveVals] = buildDealValues();
            if (isSolvable(tableauVals, blockedBy, handVal, reserveVals))
                break;
        }
    } else {
        std::shuffle(deck.begin(), deck.end(), _rng);
    }

    _allCards.reserve(52);
    auto make = [&](Suit s, int v) -> CardModel* {
        auto c = std::make_unique<CardModel>(_nextCardId++, s, v);
        auto* raw = c.get();
        _allCards.push_back(std::move(c));
        return raw;
    };

    _tableau.reserve(tableauSize);
    for (int i = 0; i < tableauSize; ++i) {
        auto* card = make(deck[i].suit, deck[i].val);
        card->setBlockerCount((int)layout[i].blockedBy.size());
        _tableau.push_back(card);
    }

    _unlocks.clear();
    for (int i = 0; i < tableauSize; ++i) {
        for (int bi : layout[i].blockedBy)
            _unlocks[_tableau[bi]].push_back(_tableau[i]);
    }

    for (int i = 0; i < initialHand; ++i) {
        int idx = tableauSize + i;
        if (idx < (int)deck.size())
            _hand.push_back(make(deck[idx].suit, deck[idx].val));
    }

    for (int i = 0; i < reserveCount; ++i) {
        int idx = tableauSize + initialHand + i;
        if (idx < (int)deck.size())
            _reserve.push_back(make(deck[idx].suit, deck[idx].val));
    }
}

void GameModel::clearAll() {
    _tableau.clear();
    _hand.clear();
    _reserve.clear();
    _allCards.clear();
    _unlocks.clear();
    _currentLevel = nullptr;
    _nextCardId = 0;
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

CardModel* GameModel::getCardById(int id) const {
    for (const auto& card : _allCards) {
        if (card->getId() == id) return card.get();
    }
    return nullptr;
}

// ---------------------------------------------------------------------------
// 遮挡系统
// ---------------------------------------------------------------------------

std::vector<CardModel*> GameModel::onTableauCardRemoved(CardModel* card) {
    std::vector<CardModel*> newlyUnblocked;
    auto it = _unlocks.find(card);
    if (it == _unlocks.end()) return newlyUnblocked;

    for (auto* blocked : it->second) {
        int before = blocked->getBlockerCount();
        blocked->decrementBlocker();
        if (before > 0 && blocked->isAccessible())
            newlyUnblocked.push_back(blocked);
    }
    return newlyUnblocked;
}

void GameModel::onTableauCardRestored(CardModel* card) {
    auto it = _unlocks.find(card);
    if (it != _unlocks.end()) {
        for (auto* blocked : it->second)
            blocked->incrementBlocker();
    }
}

// ---------------------------------------------------------------------------
// 手牌堆
// ---------------------------------------------------------------------------

CardModel* GameModel::getHandTop() const {
    return _hand.empty() ? nullptr : _hand.back();
}

void GameModel::pushHand(CardModel* card) {
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
    return card;
}

void GameModel::pushToReserve(CardModel* card) {
    _reserve.push_back(card);
}
