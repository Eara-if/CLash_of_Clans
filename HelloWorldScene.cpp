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

#include "HelloWorldScene.h"
#include "GameScene.h"
USING_NS_CC;

Scene* HelloWorld::createScene()
{
    return HelloWorld::create();
}

// Print useful error message instead of segfaulting when files are not there.
static void problemLoading(const char* filename)
{
    printf("Error while loading: %s\n", filename);
    printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in HelloWorldScene.cpp\n");
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. 初始化父类 (固定写法)
    if (!Scene::init())
    {
        return false;
    }

    // 获取屏幕可见区域大小和原点
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    //////////////////////////////
    // 2. 添加右下角的关闭按钮 (Cocos 默认自带的功能)

    // 创建关闭按钮的图片项
    auto closeItem = MenuItemImage::create(
        "CloseNormal.png",
        "CloseSelected.png",
        CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));

    // 检查图片是否存在
    if (closeItem == nullptr || closeItem->getContentSize().width <= 0 || closeItem->getContentSize().height <= 0)
    {
        problemLoading("'CloseNormal.png' and 'CloseSelected.png'");
    }
    else
    {
        // 设置按钮位置：右下角
        float x = origin.x + visibleSize.width - closeItem->getContentSize().width / 2;
        float y = origin.y + closeItem->getContentSize().height / 2;
        closeItem->setPosition(Vec2(x, y));
    }

    // 创建菜单并添加按钮
    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);

    //////////////////////////////
    // 3. 自定义背景 (这是你要的核心功能)

    // ================= 配置区域 =================
   //////////////////////////////
    // 3. 背景平铺 (Tiled Background)

    // ================== 配置区域 ==================
    std::string filename = "TilesetField.png"; // 你的素材文件名
    // 假设你的绿色小方块宽32，高32，起始坐标在(0, 32)
    // 【重要】：如果你的方块不是这个大小或位置，请修改下面这4个数字！
    // ================== 参数配置 ==================
    float tileWidth = 22;  // 原图截取宽度
    float tileHeight = 22; // 原图截取高度
    // 使用刚才测试成功的坐标 (假设是方案A)
    Rect tileRect = Rect(6, 105, tileWidth, tileHeight);

    // 【新增】放大倍数 (比如 2.0 就是放大两倍，3.0 就是三倍)
    float scaleAmount = 4.0f;

    // 【关键】计算新的步长 (实际铺设时的间距)
    // 如果图片放大了2倍，那么我们铺下一块砖的时候，就要多跨越2倍的距离
    float stepX = tileWidth * scaleAmount;
    float stepY = tileHeight * scaleAmount;
    // =============================================

    // --- 双重循环开始 ---
    // 注意：这里的 x += stepX 和 y += stepY 是修改的关键！
    for (float x = origin.x; x < origin.x + visibleSize.width; x += stepX)
    {
        for (float y = origin.y; y < origin.y + visibleSize.height; y += stepY)
        {
            auto tile = Sprite::create(filename, tileRect);

            // 1. 设置锚点为左下角 (保持不变)
            tile->setAnchorPoint(Vec2::ZERO);

            // 2. 【新增】设置放大倍数
            tile->setScale(scaleAmount);

            // 3. 设置位置
            tile->setPosition(x, y);

            // 4. 保持像素清晰 (对于放大操作特别重要！)
            tile->getTexture()->setAliasTexParameters();

            this->addChild(tile, -1);
        }
    }
    // --- 双重循环结束 ---

    //////////////////////////////
    // (下面应该接着是你之前写的 "START GAME" 按钮代码)

    //////////////////////////////
    // 4. 添加一个标题文字 (可选)

    auto label = Label::createWithTTF("My Game Start!", "fonts/Marker Felt.ttf", 24);
    if (label != nullptr)
    {
        // 放在屏幕顶部中间
        label->setPosition(Vec2(origin.x + visibleSize.width / 2,
            origin.y + visibleSize.height - label->getContentSize().height));

        // 可以在这里改颜色，防止白色文字在浅绿色背景上看不清
        // label->setTextColor(Color4B::BLACK); 

        this->addChild(label, 1);
    }
    // --- 在 init() 函数内部 ---

// 1. 创建文字标签
    auto startLabel = Label::createWithTTF("START GAME", "fonts/Marker Felt.ttf", 48);
    // 改变文字颜色为红色，方便在绿色背景上看清
    startLabel->setTextColor(Color4B::WHITE);

    // 2. 创建菜单项 (点击文字触发)
    // 参数2: CC_CALLBACK_1(类名::函数名, this)
    auto startItem = MenuItemLabel::create(startLabel, CC_CALLBACK_1(HelloWorld::menuGotoNextScene, this));

    // 设置位置：屏幕正中心偏下一点
    startItem->setPosition(Vec2(origin.x + visibleSize.width / 2,
        origin.y + visibleSize.height / 2 - 100));

    // 3. 创建菜单并添加 (注意：Cocos中按钮必须放在 Menu 里才能点击)
    // create 的参数最后必须加 NULL
    auto startmenu = Menu::create(startItem, NULL);
    // Menu 默认位置是屏幕中心，我们把它的坐标设为(0,0)，这样上面的 setPosition 才是基于屏幕左下角的
    startmenu->setPosition(Vec2::ZERO);
    this->addChild(startmenu, 1);

   

    return true;
}


void HelloWorld::menuCloseCallback(Ref* pSender)
{
    //Close the cocos2d-x game scene and quit the application
    Director::getInstance()->end();

    /*To navigate back to native iOS screen(if present) without quitting the application  ,do not use Director::getInstance()->end() as given above,instead trigger a custom event created in RootViewController.mm as below*/

    //EventCustom customEndEvent("game_scene_close_event");
    //_eventDispatcher->dispatchEvent(&customEndEvent);


}
// --- 在 HelloWorldScene.cpp 文件末尾 ---

void HelloWorld::menuGotoNextScene(Ref* pSender)
{
    // 1. 创建目标场景
    auto scene = GameScene::createScene();

    // 2. 告诉导演切换场景
    // replaceScene 会销毁当前场景，释放内存，进入新场景

    // 【方式 A】直接切换 (生硬)
    // Director::getInstance()->replaceScene(scene);

    // 【方式 B】带特效切换 (推荐)
    // TransitionFade: 淡入淡出，持续 1.0 秒
    Director::getInstance()->replaceScene(TransitionFade::create(1.0f, scene));
}
