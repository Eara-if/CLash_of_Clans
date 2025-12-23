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
#include "DataManager.h"
#include "SaveGame.h"  // ?????????????浵????????
#include "network/HttpClient.h" // 必须
#include "ui/CocosGUI.h"        // 用于输入框
#include "json/document.h"
#include "json/writer.h"
#include "json/stringbuffer.h"
using namespace cocos2d::network;
using namespace cocos2d::ui;
USING_NS_CC;

// ??????????????????
extern int coin_count;
extern int water_count;
extern int gem_count;
extern int coin_limit;
extern int water_limit;
extern int gem_limit;
extern int army_limit;
extern cocos2d::Vector<Building*> g_allPurchasedBuildings;
// 正式定义这个全局变量，给它一个初始空字符串
std::string g_currentUsername = "";

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
    // 1. ????????? (???д??)
    if (!Scene::init())
    {
        return false;
    }

    // ??????????????С?????
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    //////////////////////////////
    // 2. ????????????? (Cocos ???????????)

    // ???????????????
    auto closeItem = MenuItemImage::create(
        "CloseNormal.png",
        "CloseSelected.png",
        CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));

    // ???????????
    if (closeItem == nullptr || closeItem->getContentSize().width <= 0 || closeItem->getContentSize().height <= 0)
    {
        problemLoading("'CloseNormal.png' and 'CloseSelected.png'");
    }
    else
    {
        // ??????λ????????
        float x = origin.x + visibleSize.width - closeItem->getContentSize().width / 2;
        float y = origin.y + closeItem->getContentSize().height / 2;
        closeItem->setPosition(Vec2(x, y));
    }

    // ??????????????
    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);

    //////////////////////////////
    // 3. ????屳?? (???????????????)

    // ================= ???????? =================
   //////////////////////////////
    // 3. ??????? (Tiled Background)

    // ================== ???????? ==================
    std::string filename = "TilesetField.png"; // ???????????
    // ??????????С?????32????32???????????(0, 32)
    // ?????????????????鲻???????С??λ??????????????4???????
    // ================== ???????? ==================
    float tileWidth = 22;  // ????????
    float tileHeight = 22; // ????????
    // ???????????????? (?????????A)
    Rect tileRect = Rect(6, 105, tileWidth, tileHeight);

    // ????????????? (???? 2.0 ????????????3.0 ????????)
    float scaleAmount = 4.0f;

    // ????????????????? (????????????)
    // ??????????2???????????????????????????????2???????
    float stepX = tileWidth * scaleAmount;
    float stepY = tileHeight * scaleAmount;
    // =============================================

    // --- ????????? ---
    // ???????? x += stepX ?? y += stepY ??????????
    for (float x = origin.x; x < origin.x + visibleSize.width; x += stepX)
    {
        for (float y = origin.y; y < origin.y + visibleSize.height; y += stepY)
        {
            auto tile = Sprite::create(filename, tileRect);

            // 1. ????ê???????? (???????)
            tile->setAnchorPoint(Vec2::ZERO);

            // 2. ???????????÷????
            tile->setScale(scaleAmount);

            // 3. ????λ??
            tile->setPosition(x, y);

            // 4. ???????????? (?????????????????)
            tile->getTexture()->setAliasTexParameters();

            this->addChild(tile, -1);
        }
    }
    // --- ?????????? ---

    //////////////////////////////
    // 4. ??????????
    auto title = Label::createWithTTF("CLASH OF COCO", "fonts/Marker Felt.ttf", 48);
    if (title != nullptr)
    {
        // ????????????м?
        title->setPosition(Vec2(origin.x + visibleSize.width / 2,
            origin.y + visibleSize.height - 100));
        title->setColor(Color3B::YELLOW);
        title->enableOutline(Color4B::BLACK, 3);
        this->addChild(title, 1);
    }
    // 在 title 标签代码下方添加
    auto editBox = EditBox::create(Size(300, 60), Scale9Sprite::create("CloseNormal.png")); // 暂时借用一个图片做背景
    editBox->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2 + 300));
    editBox->setFontSize(30);
    editBox->setFontColor(Color3B::BLACK);
    editBox->setPlaceHolder("Enter Username:");
    editBox->setPlaceholderFontColor(Color3B::GRAY);
    editBox->setMaxLength(10);
    editBox->setName("username_input"); // 给它起个名字，方便后面获取内容
    this->addChild(editBox, 1);

    // 密码框 (复制用户名框的代码，修改位置和 Placeholder)
    auto passwordBox = EditBox::create(Size(300, 60), Scale9Sprite::create("CloseNormal.png"));
    passwordBox->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2 + 200));
    passwordBox->setInputFlag(EditBox::InputFlag::PASSWORD); // 设置为密码模式（显示星号）
    passwordBox->setPlaceHolder("Enter Password:");
    passwordBox->setName("password_input");
    this->addChild(passwordBox, 1);


    // ???????????????д浵???
    bool hasSaveFile = SaveGame::getInstance()->isSaveFileExist();

    // ??????????????????Y????
    float buttonStartY = origin.y + visibleSize.height / 2 + 50;

    // 5. ????????????"???????"??? (????д浵)
    if (hasSaveFile)
    {
        auto continueLabel = Label::createWithTTF("CONTINUE GAME", "fonts/Marker Felt.ttf", 48);
        continueLabel->setTextColor(Color4B::GREEN);
        continueLabel->enableOutline(Color4B::BLACK, 2);

        auto continueItem = MenuItemLabel::create(continueLabel,
            CC_CALLBACK_1(HelloWorld::menuContinueGame, this));

        continueItem->setPosition(Vec2(origin.x + visibleSize.width / 2,
            buttonStartY));

        // ????????????
        auto startmenu = Menu::create(continueItem, NULL);
        startmenu->setPosition(Vec2::ZERO);
        this->addChild(startmenu, 1);

        // ???°?????λ??
        buttonStartY -= 80;
    }

    // 6. ????"??????"???
    auto startLabel = Label::createWithTTF("NEW GAME", "fonts/Marker Felt.ttf", 48);
    startLabel->setTextColor(Color4B::WHITE);
    startLabel->enableOutline(Color4B::BLACK, 2);

    auto startItem = MenuItemLabel::create(startLabel,
        CC_CALLBACK_1(HelloWorld::menuGotoNextScene, this));

    startItem->setPosition(Vec2(origin.x + visibleSize.width / 2,
        buttonStartY));

    // ??д浵???????"??????"???
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
void HelloWorld::startNewGame() {
    // 【新增】重置关卡进度为1（只能打第一关）
    DataManager::getInstance()->setMaxLevelUnlocked(1);

    // 【新增】清空所有训练过的兵种
    DataManager::getInstance()->clearArmy();
    coin_count = 5000;
    water_count = 5000;
    gem_count = 500;
    coin_limit = 5000;
    water_limit = 5000;
    gem_limit = 5000;
    army_limit = 10;

    for (auto& building : g_allPurchasedBuildings) {
        if (building) building->removeFromParent();
    }
    g_allPurchasedBuildings.clear();

    auto scene = GameScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
}
void HelloWorld::menuGotoNextScene(Ref* pSender)
{
    // 获取输入框里的名字
    auto editBox = static_cast<EditBox*>(this->getChildByName("username_input"));
    std::string name = editBox->getText();

    if (name.empty()) {
        // 如果没填名字，可以晃动一下输入框或者提示
        log("Please enter your name first!");
        return;
    }

    // 先登录服务器，登录成功后会自动调用 startNewGame
    this->loginToServer(name);
}

// ???????????????
void HelloWorld::menuContinueGame(Ref* pSender)
{
    log("Continue game...");

    // ???????浵
    if (SaveGame::getInstance()->loadGameState())
    {
        log("Save game loaded successfully!");

        // ???????????С?????????????????????????????
        this->runAction(Sequence::create(
            DelayTime::create(0.1f),
            CallFunc::create([=]() {
                // ???????????????????????????浵?????????
                auto scene = GameScene::createScene();
                Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
                }),
            nullptr
        ));
    }
    else
    {
        log("Failed to load save game!");

        // ??????????
        auto visibleSize = Director::getInstance()->getVisibleSize();
        Vec2 origin = Director::getInstance()->getVisibleOrigin();

        auto errorLabel = Label::createWithTTF("Failed to load save game!", "fonts/Marker Felt.ttf", 32);
        errorLabel->setPosition(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2);
        errorLabel->setColor(Color3B::RED);
        this->addChild(errorLabel, 10);

        // 2?????????
        errorLabel->runAction(Sequence::create(
            DelayTime::create(2.0f),
            RemoveSelf::create(),
            nullptr
        ));
    }
}

void HelloWorld::loginToServer(std::string username) {
    // 1. 获取密码（假设你已经创建了 password_input）
    auto passwordBox = static_cast<cocos2d::ui::EditBox*>(this->getChildByName("password_input"));
    std::string password = passwordBox ? passwordBox->getText() : "";

    auto request = new (std::nothrow) HttpRequest();
    request->setUrl("http://127.0.0.1:5000/login");
    request->setRequestType(HttpRequest::Type::POST);

    std::vector<std::string> headers;
    headers.push_back("Content-Type: application/json; charset=utf-8");
    request->setHeaders(headers);

    // --- 这里是代码插入的位置 ---
    request->setResponseCallback([=](HttpClient* client, HttpResponse* response) {
        if (!response || !response->isSucceed()) {
            log("Network Error!");
            return;
        }

        std::vector<char>* buffer = response->getResponseData();
        std::string resultStr(buffer->begin(), buffer->end());

        // 使用 RapidJSON 精确解析服务器返回的 JSON 结构
        rapidjson::Document doc;
        doc.Parse(resultStr.c_str());

        if (doc.HasParseError()) {
            log("JSON Parse Error from Server");
            return;
        }

        // 检查服务器状态码
        if (doc.HasMember("status") && std::string(doc["status"].GetString()) == "success") {

            // 保存当前用户名到全局，方便后面存盘用
            g_currentUsername = username;

            // 提取存档字符串 (data 字段)
            if (doc.HasMember("data") && doc["data"].IsString()) {
                std::string gameSaveData = doc["data"].GetString();

                // 如果存档不为空，让 SaveGame 解析并加载建筑
                if (!gameSaveData.empty() && gameSaveData != "") {
                    log("Loading game data for user: %s", username.c_str());
                    SaveGame::getInstance()->loadFromRemoteString(gameSaveData);
                }
                else {
                    log("New user detected, starting fresh game.");
                }
            }

            // 登录成功，进入游戏
            this->startNewGame();
        }
        else {
            log("Login failed: Username or password error.");
        }
        });

    // 发送登录请求
    std::string postData = "{\"username\":\"" + username + "\", \"password\":\"" + password + "\"}";
    request->setRequestData(postData.c_str(), postData.length());

    HttpClient::getInstance()->send(request);
    request->release();
}