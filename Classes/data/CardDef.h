// CardDef — 扑克牌领域基础：花色、点数及字符串工具，与布局/关卡无关
#pragma once
#include <string>

enum class Suit { Heart = 0, Diamond, Spade, Club };

// 功能牌类型：行为在 controller/cards/ 注册，外观在 CardAppearance 配置
enum class CardKind {
    Normal,
    // Wild,   // 示例：万能匹配 → 新建 XxxHandler + registerHandler
    // Bonus,  // 示例：点击加牌 → 新建 XxxHandler + registerHandler
};

inline bool isRedSuit(Suit suit) {
    return suit == Suit::Heart || suit == Suit::Diamond;
}

inline std::string suitToString(Suit suit) {
    switch (suit) {
        case Suit::Heart:   return "heart";
        case Suit::Diamond: return "diamond";
        case Suit::Spade:   return "spade";
        case Suit::Club:    return "club";
    }
    return "heart";
}

inline std::string valueToString(int v) {
    if (v == 1)  return "A";
    if (v == 11) return "J";
    if (v == 12) return "Q";
    if (v == 13) return "K";
    return std::to_string(v);
}
