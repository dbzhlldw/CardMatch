// TableauActionResult — tryTableauAction 是否执行成功；复杂功能牌可在此带回受影响牌 id 等（Presenter 不必再扫 Model）
// 例：奖励牌一次消多张 → success + affectedCardIds；普通匹配仍只需 success
#pragma once

struct TableauActionResult {
    bool success = false;
};
