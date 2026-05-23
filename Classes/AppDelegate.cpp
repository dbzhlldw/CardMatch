#include "AppDelegate.h"
#include "view/GameScene.h"

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
        // 设计分辨率 1080×2080，0.5 缩放在桌面显示
        glview = GLViewImpl::createWithRect(
            "CardMatch", cocos2d::Rect(0, 0, 1080, 2080), 0.5f);
#else
        glview = GLViewImpl::create("CardMatch");
#endif
        director->setOpenGLView(glview);
    }

    director->setDisplayStats(true);
    director->setAnimationInterval(1.0f / 60);

    glview->setDesignResolutionSize(1080, 2080, ResolutionPolicy::FIXED_WIDTH);

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
