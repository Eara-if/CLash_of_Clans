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
#include "SaveGame.h"  // 【新增】包含存档管理头文件
USING_NS_CC;

// 【新增】声明全局变量
extern int coin_count;
extern int water_count;
extern int gem_count;
extern int coin_limit;
extern int water_limit;
extern int gem_limit;
extern int army_limit;
extern cocos2d::Vector<Building*> g_allPurchasedBuildings;

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
    // 4. 添加游戏标题
    auto title = Label::createWithTTF("CLASH OF COCO", "fonts/Marker Felt.ttf", 48);
    if (title != nullptr)
    {
        // 放在屏幕顶部中间
        title->setPosition(Vec2(origin.x + visibleSize.width / 2,
            origin.y + visibleSize.height - 100));
        title->setColor(Color3B::YELLOW);
        title->enableOutline(Color4B::BLACK, 3);
        this->addChild(title, 1);
    }

    // 【新增】检查是否有存档文件
    bool hasSaveFile = SaveGame::getInstance()->isSaveFileExist();

    // 【修改】创建按钮的起始Y坐标
    float buttonStartY = origin.y + visibleSize.height / 2 + 50;

    // 5. 【新增】创建"继续游戏"按钮 (如果有存档)
    if (hasSaveFile)
    {
        auto continueLabel = Label::createWithTTF("CONTINUE GAME", "fonts/Marker Felt.ttf", 48);
        continueLabel->setTextColor(Color4B::GREEN);
        continueLabel->enableOutline(Color4B::BLACK, 2);

        auto continueItem = MenuItemLabel::create(continueLabel,
            CC_CALLBACK_1(HelloWorld::menuContinueGame, this));

        continueItem->setPosition(Vec2(origin.x + visibleSize.width / 2,
            buttonStartY));

        // 将按钮添加到菜单
        auto startmenu = Menu::create(continueItem, NULL);
        startmenu->setPosition(Vec2::ZERO);
        this->addChild(startmenu, 1);

        // 更新按钮起始位置
        buttonStartY -= 80;
    }

    // 6. 创建"开始游戏"按钮
    auto startLabel = Label::createWithTTF("NEW GAME", "fonts/Marker Felt.ttf", 48);
    startLabel->setTextColor(Color4B::WHITE);
    startLabel->enableOutline(Color4B::BLACK, 2);

    auto startItem = MenuItemLabel::create(startLabel,
        CC_CALLBACK_1(HelloWorld::menuGotoNextScene, this));

    startItem->setPosition(Vec2(origin.x + visibleSize.width / 2,
        buttonStartY));

    // 没有存档时，只显示"开始游戏"按钮
    auto startmenu = Menu::create(startItem, NULL);
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

// 开始新游戏
void HelloWorld::menuGotoNextScene(Ref* pSender)
{
    // 【新增】重置游戏数据
    coin_count = 5000;
    water_count = 5000;
    gem_count = 500;
    coin_limit = 5000;
    water_limit = 5000;
    gem_limit = 5000;
    army_limit = 10;

    // 清空已购买建筑
    for (auto& building : g_allPurchasedBuildings) {
        if (building) {
            building->removeFromParent();
        }
    }
    g_allPurchasedBuildings.clear();

    // 1. 创建目标场景
    auto scene = GameScene::createScene();

    // 2. 告诉导演切换场景
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
}

// 【新增】继续游戏
void HelloWorld::menuContinueGame(Ref* pSender)
{
    log("Continue game...");

    // 尝试加载存档
    if (SaveGame::getInstance()->loadGameState())
    {
        log("Save game loaded successfully!");

        // 【重要】延迟一小段时间后创建游戏场景，确保数据已加载
        this->runAction(Sequence::create(
            DelayTime::create(0.1f),
            CallFunc::create([=]() {
                // 创建游戏场景（不传入建筑，因为存档中已包含）
                auto scene = GameScene::createScene();
                Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
                }),
            nullptr
        ));
    }
    else
    {
        log("Failed to load save game!");

        // 显示错误提示
        auto visibleSize = Director::getInstance()->getVisibleSize();
        Vec2 origin = Director::getInstance()->getVisibleOrigin();

        auto errorLabel = Label::createWithTTF("Failed to load save game!", "fonts/Marker Felt.ttf", 32);
        errorLabel->setPosition(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2);
        errorLabel->setColor(Color3B::RED);
        this->addChild(errorLabel, 10);

        // 2秒后移除提示
        errorLabel->runAction(Sequence::create(
            DelayTime::create(2.0f),
            RemoveSelf::create(),
            nullptr
        ));
    }
}
