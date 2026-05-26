// GameModel — 一局游戏的牌堆状态：桌面/手牌/备用堆、发牌、遮挡、可解性验证
#pragma once
#include <vector>
#include <memory>
#include <unordered_map>
#include <random>
#include "CardModel.h"
#include "data/LevelDef.h"

class GameModel {
public:
    GameModel();

    void setupGame(const LevelDef& level);
    void clearAll();

    const LevelDef* getCurrentLevel() const { return _currentLevel; }

    // 桌面牌区
    const std::vector<CardModel*>& getTableau() const { return _tableau; }
    void removeFromTableau(int index);
    void insertToTableau(int index, CardModel* card);
    int  getTableauIndex(CardModel* card) const;

    // 遮挡系统
    std::vector<CardModel*> onTableauCardRemoved(CardModel* card);
    void onTableauCardRestored(CardModel* card);

    // 手牌堆
    CardModel* getHandTop() const;
    void       pushHand(CardModel* card);
    CardModel* popHand();
    size_t     getHandSize() const { return _hand.size(); }

    // 备用牌堆
    CardModel* getReserveTop() const;
    CardModel* drawFromReserve();
    void       pushToReserve(CardModel* card);
    size_t     getReserveSize() const { return _reserve.size(); }
    const std::vector<CardModel*>& getReserve() const { return _reserve; }
    const std::vector<CardModel*>& getHand()    const { return _hand; }

    CardModel* getCardById(int id) const;

    bool isTableauClear() const { return _tableau.empty(); }

private:
    int _nextCardId = 0;
    std::vector<CardModel*>              _tableau;
    std::vector<CardModel*>              _hand;
    std::vector<CardModel*>              _reserve;
    std::vector<std::unique_ptr<CardModel>> _allCards;

    std::unordered_map<CardModel*, std::vector<CardModel*>> _unlocks;

    std::default_random_engine _rng;
    const LevelDef* _currentLevel = nullptr;
};
