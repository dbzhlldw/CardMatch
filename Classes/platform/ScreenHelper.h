// ScreenHelper — 跨平台屏幕工具声明；返回可用屏幕高度，供 AppDelegate 窗口缩放
#pragma once

// 返回当前屏幕可用高度（点）。非 macOS 返回 0 表示不做限制。
float getAvailableScreenHeight();
