// Levels — 可玩关卡目录（定义）；新关卡在此登记，新布局形状另建 LayoutDef_<Name>.*
#include "Levels.h"
#include "LayoutDef_Pyramid.h"

namespace Levels {

const LevelDef PYRAMID_TEST(
    "pyramid_test",
    Layouts::PYRAMID,
    6);

} // namespace Levels
