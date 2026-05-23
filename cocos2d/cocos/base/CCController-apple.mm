/****************************************************************************
 Copyright (c) 2014 cocos2d-x.org
 Copyright (c) 2014-2016 Chukong Technologies Inc.
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.

 http://www.cocos2d-x.org

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "base/CCController.h"
#include "platform/CCPlatformConfig.h"
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS || CC_TARGET_PLATFORM == CC_PLATFORM_MAC)

// Stub implementation — GameController framework disabled to avoid macOS 15
// SDK incompatibility (dispatch_queue_t strong property error in GCDevice.h).
// This project does not use gamepad input.

NS_CC_BEGIN

class ControllerImpl
{
public:
    ControllerImpl(Controller* controller) : _controller(controller) {}
    Controller* _controller;
};

void Controller::startDiscoveryController() {}
void Controller::stopDiscoveryController() {}

Controller::Controller()
: _deviceId(0)
, _controllerTag(TAG_UNSET)
, _impl(new ControllerImpl(this))
, _connectEvent(nullptr)
, _keyEvent(nullptr)
, _axisEvent(nullptr)
{
    init();
}

Controller::~Controller()
{
    delete _impl;
    delete _connectEvent;
    delete _keyEvent;
    delete _axisEvent;
}

void Controller::registerListeners() {}

bool Controller::isConnected() const
{
    return false;
}

void Controller::receiveExternalKeyEvent(int externalKeyCode, bool receive) {}

NS_CC_END

#endif // #if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS || CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
