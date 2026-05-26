#include "AppDelegate.h"
#include "view/GameScene.h"
#if (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
#include "platform/ScreenHelper.h"
#endif

USING_NS_CC;

AppDelegate::AppDelegate() {}
AppDelegate::~AppDelegate() {}

void AppDelegate::initGLContextAttrs() {
    GLContextAttrs glContextAttrs = {8, 8, 8, 8, 24, 8, 0};
    GLView::setGLContextAttrs(glContextAttrs);
}

static int register_all_packages() { return 0; }

bool AppDelegate::applicationDidFinishLaunching() {
    auto director = Director::getInstance();
    auto glview   = director->getOpenGLView();

    if (!glview) {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || \
    (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)  || \
    (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
        // 默认按设计尺寸 1:1 创建窗口（1080×2080）；仅当屏幕高度放不下时再等比缩小
        float scale = 1.0f;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
        // visibleFrame 已排除菜单栏和 Dock，再减去窗口标题栏（约28px）和底部安全边距
        static const float WINDOW_TITLE_BAR_H = 28.f;
        static const float BOTTOM_MARGIN      = 10.f;
        float screenH = getAvailableScreenHeight() - WINDOW_TITLE_BAR_H - BOTTOM_MARGIN;
        float idealWindowH = GameScene::Layout::DESIGN_HEIGHT * scale;
        if (screenH > 0 && screenH < idealWindowH) {
            scale = screenH / GameScene::Layout::DESIGN_HEIGHT;
        }
#endif
        glview = GLViewImpl::createWithRect(
            "CardMatch",
            cocos2d::Rect(0, 0, GameScene::Layout::DESIGN_WIDTH, GameScene::Layout::DESIGN_HEIGHT),
            scale);
#else
        glview = GLViewImpl::create("CardMatch");
#endif
        director->setOpenGLView(glview);
    }

    director->setDisplayStats(false);
    director->setAnimationInterval(1.0f / 60);

    glview->setDesignResolutionSize(GameScene::Layout::DESIGN_WIDTH, GameScene::Layout::DESIGN_HEIGHT,
                                    ResolutionPolicy::FIXED_WIDTH);

    register_all_packages();

    auto scene = GameScene::createScene();
    director->runWithScene(scene);

    return true;
}

void AppDelegate::applicationDidEnterBackground() {
    Director::getInstance()->stopAnimation();
}

void AppDelegate::applicationWillEnterForeground() {
    Director::getInstance()->startAnimation();
}
