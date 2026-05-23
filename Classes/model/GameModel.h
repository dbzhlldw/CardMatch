#pragma once
#include <vector>
#include <memory>
#include "CardModel.h"

// 持有所有牌的数据状态：桌面牌、手牌堆、备用牌堆
class GameModel {
public:
    static GameModel* getInstance();

    void setupGame();
    void clearAll();

    // 桌面牌区
    const std::vector<CardModel*>& getTableau() const { return _tableau; }
    void removeFromTableau(int index);
    void insertToTableau(int index, CardModel* card);
    int  getTableauIndex(CardModel* card) const;

    // 手牌堆
    CardModel* getHandTop() const;
    void       pushHand(CardModel* card);
    CardModel* popHand();
    size_t     getHandSize() const { return _hand.size(); }

    // 备用牌堆
    CardModel* getReserveTop() const;
    CardModel* drawFromReserve();
    void       pushToReserve(CardModel* card);  // 用于撤回
    size_t     getReserveSize() const { return _reserve.size(); }
    const std::vector<CardModel*>& getReserve() const { return _reserve; }
    const std::vector<CardModel*>& getHand()    const { return _hand; }

private:
    GameModel() = default;
    static GameModel* _instance;

    std::vector<CardModel*>              _tableau;
    std::vector<CardModel*>              _hand;    // vector 的尾端为牌堆的顶部
    std::vector<CardModel*>              _reserve; // vector 的尾端为牌堆的顶部
    std::vector<std::unique_ptr<CardModel>> _allCards; // 内存所有者
};
