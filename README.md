# CardMatch

纸牌消除游戏，基于 cocos2d-x 3.17.2（C++）。

## 玩法

- 点击桌面未被遮挡的牌，与当前手牌点数相差 1 即可消除。
- 点击备用牌堆抽牌。
- 支持操作回退。
- 清空桌面即通关。同一关卡槽位与遮挡关系固定，每局洗牌随机发牌。

## 运行环境

- macOS，Xcode，CMake
- 编译架构 **x86_64**（引擎预编译库暂不支持 arm64）
- 窗口默认 **1080×2080**；macOS 屏高不足时自动等比缩小

## 编译与运行

```bash
mkdir -p build_mac && cd build_mac
cmake .. -GXcode -DCMAKE_OSX_ARCHITECTURES=x86_64
xcodebuild -scheme CardMatch -configuration Debug -arch x86_64 build
open bin/CardMatch/Debug/CardMatch.app
```

若在 `Resources/` 下增删资源，需重新执行 `cmake`。

## 程序设计文档

[docs/design.md](docs/design.md)
