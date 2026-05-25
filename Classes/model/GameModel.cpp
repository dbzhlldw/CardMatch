#include "GameModel.h"
#include <algorithm>
#include <cmath>
#include <queue>
#include <unordered_set>
#include <tuple>

GameModel* GameModel::_instance = nullptr;

GameModel::GameModel()
    : _rng(std::random_device{}()) {}

GameModel* GameModel::getInstance() {
    if (!_instance) _instance = new GameModel();
    return _instance;
}

// ---------------------------------------------------------------------------
// 可解性验证（BFS）
//
// 状态三元组：(tableau_mask, hand_value, reserve_depth)
//   - tableau_mask : 第 i 位为 1 表示槽位 i 的牌仍在桌面（n ≤ 10 → 10 bit）
//   - hand_value   : 当前手牌堆顶的点数 1-13（4 bit）
//   - reserve_depth: 备用堆剩余张数 0-6（3 bit）
// 编码总空间 ≤ 1024 × 13 × 7 = 93184，BFS 在微秒级完成。
//
// reserveVals[rn-1] = 顶部（第一张被抽到），reserveVals[0] = 底部（最后抽到）
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

        if (mask == 0) return true; // 桌面清空，通关

        // 尝试匹配每张可操作桌面牌
        for (int i = 0; i < n; ++i) {
            if (!(mask & (1 << i))) continue;           // 已移走
            bool accessible = true;
            for (int b : blockedBy[i])
                if (mask & (1 << b)) { accessible = false; break; }
            if (!accessible) continue;
            if (std::abs(tableauVals[i] - hv) != 1) continue; // 点数差不为 1

            int nm = mask & ~(1 << i);
            int nh = tableauVals[i];
            int s  = encode(nm, nh, rd);
            if (!visited.count(s)) { visited.insert(s); q.push({nm, nh, rd}); }
        }

        // 尝试从备用堆抽牌（顶部是 reserveVals[rd-1]）
        if (rd > 0) {
            int nh = reserveVals[rd - 1];
            int s  = encode(mask, nh, rd - 1);
            if (!visited.count(s)) { visited.insert(s); q.push({mask, nh, rd - 1}); }
        }
    }
    return false; // 无解
}

// ---------------------------------------------------------------------------
// 初始化一局
// ---------------------------------------------------------------------------

void GameModel::setupGame(const LayoutDef& layout) {
    clearAll();

    const int RESERVE_COUNT = 6;
    int tableauSize = (int)layout.size();

    // 预提取布局的 blockedBy（供 BFS 使用）
    std::vector<std::vector<int>> blockedBy(tableauSize);
    for (int i = 0; i < tableauSize; ++i)
        blockedBy[i] = layout[i].blockedBy;

    // 生成 52 张牌规格
    struct CardSpec { Suit suit; int val; };
    std::vector<CardSpec> deck;
    deck.reserve(52);
    for (int s = 0; s < 4; ++s)
        for (int v = 1; v <= 13; ++v)
            deck.push_back({ (Suit)s, v });

    // 洗牌 + BFS 可解性验证，最多重试 500 次
    // 发牌顺序：deck[0..tableauSize-1] → 桌面
    //           deck[tableauSize]       → 初始手牌
    //           deck[tableauSize+1..]   → 备用堆（back = 顶部）
    int needed = tableauSize + 1 + RESERVE_COUNT;
    for (int attempt = 0; attempt < 500; ++attempt) {
        std::shuffle(deck.begin(), deck.end(), _rng);
        if ((int)deck.size() < needed) break; // 理论上不会触发

        // 提取值数组供 BFS 使用
        std::vector<int> tableauVals(tableauSize);
        for (int i = 0; i < tableauSize; ++i)
            tableauVals[i] = deck[i].val;

        int handVal = deck[tableauSize].val;

        std::vector<int> reserveVals(RESERVE_COUNT);
        for (int i = 0; i < RESERVE_COUNT; ++i)
            reserveVals[i] = deck[tableauSize + 1 + i].val;
        // reserveVals[RESERVE_COUNT-1] = 顶部（最先抽到）

        if (isSolvable(tableauVals, blockedBy, handVal, reserveVals))
            break; // 找到可解配置
        // 未找到则继续重试，最终使用最后一次洗牌结果（极低概率走到这里）
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

    // 备用牌堆：取 RESERVE_COUNT 张，面朝下，按顺序压入（最后一张在顶部）
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
        int before = blocked->getBlockerCount();
        blocked->decrementBlocker();
        // 仅当本次遮挡解除后才算「新解锁」，避免重复记录导致撤销时误清回调
        if (before > 0 && blocked->isAccessible()) {
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
