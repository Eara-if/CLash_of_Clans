/****************************************************************************
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

#ifndef HELLOWORLD_SCENE_H_
#define HELLOWORLD_SCENE_H_

#include "cocos2d.h"

 // HelloWorld场景类，继承自cocos2d::Scene
class HelloWorld : public cocos2d::Scene
{
public:
    // 创建场景的静态方法
    static cocos2d::Scene* createScene();

    // 初始化场景（重写父类方法）
    virtual bool init();

    // 回调函数：关闭按钮
    void menuCloseCallback(const cocos2d::Ref* p_sender);
    // 回调函数：登录/开始游戏按钮
    void menuGotoNextScene(const cocos2d::Ref* p_sender);
    // 回调函数：继续游戏按钮（加载本地存档）
    void menuContinueGame(const cocos2d::Ref* p_sender);
    // 登录服务器函数
    void loginToServer(const std::string& username);
    // 开始新游戏（切换到游戏场景）
    void startNewGame();
    // 回调函数：本地新游戏按钮（离线模式）
    void menuLocalNewGame(const cocos2d::Ref* p_sender);
    // Cocos2d-x宏，用于创建对象
    CREATE_FUNC(HelloWorld);
};

#endif // HELLOWORLD_SCENE_H_