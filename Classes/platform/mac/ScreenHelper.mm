#import <AppKit/AppKit.h>
#include "platform/ScreenHelper.h"

float getAvailableScreenHeight() {
    // visibleFrame 排除了 macOS 菜单栏和 Dock 的高度
    NSRect frame = [[NSScreen mainScreen] visibleFrame];
    return (float)frame.size.height;
}
