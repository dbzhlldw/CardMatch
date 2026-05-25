#pragma once
#include <vector>
#include <memory>
#include <unordered_map>
#include <random>
#include "CardModel.h"
#include "data/LayoutDef.h"

// 持有所有牌的数据状态：桌面牌、手牌堆、备用牌堆
class GameModel {
public:
    static GameModel* getInstance();

    // 根据布局定义初始化一局游戏（52 张随机洗牌 + 简单初始可匹配检查）
    void setupGame(const LayoutDef& layout);
    void clearAll();

    // 桌面牌区
    const std::vector<CardModel*>& getTableau() const { return _tableau; }
    void removeFromTableau(int index);
    void insertToTableau(int index, CardModel* card);
    int  getTableauIndex(CardModel* card) const;

    // 遮挡系统
    // 移走一张桌面牌后，更新相关牌的遮挡计数，返回新变为可操作的牌列表
    std::vector<CardModel*> onTableauCardRemoved(CardModel* card);
    // 撤回时将桌面牌归还，恢复被遮挡牌的计数
    void onTableauCardRestored(CardModel* card);

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
    GameModel();
    static GameModel* _instance;

    std::vector<CardModel*>              _tableau;
    std::vector<CardModel*>              _hand;    // 尾端为顶部
    std::vector<CardModel*>              _reserve; // 尾端为顶部
    std::vector<std::unique_ptr<CardModel>> _allCards; // 内存所有者

    // 遮挡逆映射：card → 移走该 card 后遮挡计数减少的牌列表
    std::unordered_map<CardModel*, std::vector<CardModel*>> _unlocks;

    std::default_random_engine _rng;
};
