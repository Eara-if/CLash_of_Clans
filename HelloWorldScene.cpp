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
#include "SaveGame.h"  // �������������浵����ͷ�ļ�
#include "network/HttpClient.h" // 必须
#include "ui/CocosGUI.h"        // 用于输入框
#include "json/document.h"
#include "json/writer.h"
#include "json/stringbuffer.h"
using namespace cocos2d::network;
using namespace cocos2d::ui;
USING_NS_CC;

// ������������ȫ�ֱ���
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
    // 1. ��ʼ������ (�̶�д��)
    if (!Scene::init())
    {
        return false;
    }

    // ��ȡ��Ļ�ɼ������С��ԭ��
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    //////////////////////////////
    // 2. ������½ǵĹرհ�ť (Cocos Ĭ���Դ��Ĺ���)

    // �����رհ�ť��ͼƬ��
    auto closeItem = MenuItemImage::create(
        "CloseNormal.png",
        "CloseSelected.png",
        CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));

    // ���ͼƬ�Ƿ����
    if (closeItem == nullptr || closeItem->getContentSize().width <= 0 || closeItem->getContentSize().height <= 0)
    {
        problemLoading("'CloseNormal.png' and 'CloseSelected.png'");
    }
    else
    {
        // ���ð�ťλ�ã����½�
        float x = origin.x + visibleSize.width - closeItem->getContentSize().width / 2;
        float y = origin.y + closeItem->getContentSize().height / 2;
        closeItem->setPosition(Vec2(x, y));
    }

    // �����˵�����Ӱ�ť
    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);

    //////////////////////////////
    // 3. �Զ��屳�� (������Ҫ�ĺ��Ĺ���)

    // ================= �������� =================
   //////////////////////////////
    // 3. ����ƽ�� (Tiled Background)

    // ================== �������� ==================
    std::string filename = "TilesetField.png"; // ����ز��ļ���
    // ���������ɫС�����32����32����ʼ������(0, 32)
    // ����Ҫ���������ķ��鲻�������С��λ�ã����޸�������4�����֣�
    // ================== �������� ==================
    float tileWidth = 22;  // ԭͼ��ȡ���
    float tileHeight = 22; // ԭͼ��ȡ�߶�
    // ʹ�øղŲ��Գɹ������� (�����Ƿ���A)
    Rect tileRect = Rect(6, 105, tileWidth, tileHeight);

    // ���������Ŵ��� (���� 2.0 ���ǷŴ�������3.0 ��������)
    float scaleAmount = 4.0f;

    // ���ؼ��������µĲ��� (ʵ������ʱ�ļ��)
    // ���ͼƬ�Ŵ���2������ô��������һ��ש��ʱ�򣬾�Ҫ���Խ2���ľ���
    float stepX = tileWidth * scaleAmount;
    float stepY = tileHeight * scaleAmount;
    // =============================================

    // --- ˫��ѭ����ʼ ---
    // ע�⣺����� x += stepX �� y += stepY ���޸ĵĹؼ���
    for (float x = origin.x; x < origin.x + visibleSize.width; x += stepX)
    {
        for (float y = origin.y; y < origin.y + visibleSize.height; y += stepY)
        {
            auto tile = Sprite::create(filename, tileRect);

            // 1. ����ê��Ϊ���½� (���ֲ���)
            tile->setAnchorPoint(Vec2::ZERO);

            // 2. �����������÷Ŵ���
            tile->setScale(scaleAmount);

            // 3. ����λ��
            tile->setPosition(x, y);

            // 4. ������������ (���ڷŴ�����ر���Ҫ��)
            tile->getTexture()->setAliasTexParameters();

            this->addChild(tile, -1);
        }
    }
    // --- ˫��ѭ������ ---

    //////////////////////////////
    // 4. �����Ϸ����
    auto title = Label::createWithTTF("CLASH OF COCO", "fonts/Marker Felt.ttf", 48);
    if (title != nullptr)
    {
        // ������Ļ�����м�
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


    // ������������Ƿ��д浵�ļ�
    bool hasSaveFile = SaveGame::getInstance()->isSaveFileExist();

    // ���޸ġ�������ť����ʼY����
    float buttonStartY = origin.y + visibleSize.height / 2 + 50;

    // 5. ������������"������Ϸ"��ť (����д浵)
    if (hasSaveFile)
    {
        auto continueLabel = Label::createWithTTF("LOCAL LOAD(READ FROM FILES)", "fonts/Marker Felt.ttf", 48);
        continueLabel->setTextColor(Color4B::GREEN);
        continueLabel->enableOutline(Color4B::BLACK, 2);

        auto continueItem = MenuItemLabel::create(continueLabel,
            CC_CALLBACK_1(HelloWorld::menuContinueGame, this));

        continueItem->setPosition(Vec2(origin.x + visibleSize.width / 2,
            buttonStartY));

        // ����ť��ӵ��˵�
        auto startmenu = Menu::create(continueItem, NULL);
        startmenu->setPosition(Vec2::ZERO);
        this->addChild(startmenu, 1);

        // ���°�ť��ʼλ��
        buttonStartY -= 80;
    }

    // 6. ����"��ʼ��Ϸ"��ť
    auto startLabel = Label::createWithTTF("LOGIN/START", "fonts/Marker Felt.ttf", 48);
    startLabel->setTextColor(Color4B::WHITE);
    startLabel->enableOutline(Color4B::BLACK, 2);

    auto startItem = MenuItemLabel::create(startLabel,
        CC_CALLBACK_1(HelloWorld::menuGotoNextScene, this));

    startItem->setPosition(Vec2(origin.x + visibleSize.width / 2,
        buttonStartY));

    // û�д浵ʱ��ֻ��ʾ"��ʼ��Ϸ"��ť
    auto startmenu = Menu::create(startItem, NULL);
    startmenu->setPosition(Vec2::ZERO);
    this->addChild(startmenu, 1);

    auto localNewGameLabel = Label::createWithTTF("LOCAL NEW GAME", "fonts/Marker Felt.ttf", 40);
    localNewGameLabel->setTextColor(Color4B::YELLOW);
    localNewGameLabel->enableOutline(Color4B::BLACK, 2);

    auto localNewGameItem = MenuItemLabel::create(localNewGameLabel,
        CC_CALLBACK_1(HelloWorld::menuLocalNewGame, this));

    // 放在 Login 按钮下方
    localNewGameItem->setPosition(Vec2(origin.x + visibleSize.width / 2, buttonStartY - 80));

    auto localMenu = Menu::create(localNewGameItem, NULL);
    localMenu->setPosition(Vec2::ZERO);
    this->addChild(localMenu, 1);
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

// ��������������Ϸ
void HelloWorld::menuContinueGame(Ref* pSender)
{
    log("Continue game...");

    // ���Լ��ش浵
    if (SaveGame::getInstance()->loadGameState())
    {
        log("Save game loaded successfully!");

        // ����Ҫ���ӳ�һС��ʱ��󴴽���Ϸ������ȷ�������Ѽ���
        this->runAction(Sequence::create(
            DelayTime::create(0.1f),
            CallFunc::create([=]() {
                // ������Ϸ�����������뽨������Ϊ�浵���Ѱ�����
                auto scene = GameScene::createScene();
                Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
                }),
            nullptr
        ));
    }
    else
    {
        log("Failed to load save game!");

        // ��ʾ������ʾ
        auto visibleSize = Director::getInstance()->getVisibleSize();
        Vec2 origin = Director::getInstance()->getVisibleOrigin();

        auto errorLabel = Label::createWithTTF("Failed to load save game!", "fonts/Marker Felt.ttf", 32);
        errorLabel->setPosition(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2);
        errorLabel->setColor(Color3B::RED);
        this->addChild(errorLabel, 10);

        // 2����Ƴ���ʾ
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
    request->setUrl("http://100.80.248.229:5000/login");
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

        if (doc.HasMember("status") && std::string(doc["status"].GetString()) == "success") {

            coin_count = 5000;
            water_count = 5000;
            gem_count = 500;
            army_limit = 10;
            g_allPurchasedBuildings.clear();
            g_currentUsername = username;

            bool isNewUser = true; // 标记是否为新用户

            // 提取存档字符串
            if (doc.HasMember("data") && doc["data"].IsString()) {
                std::string gameSaveData = doc["data"].GetString();

                // 如果存档不为空，加载数据
                if (!gameSaveData.empty() && gameSaveData != "") {
                    log("Loading game data for user: %s", username.c_str());

                    // 【关键】先清空旧数据，防止叠加（这也是你之前想做的事，但要在这里做）
                    g_allPurchasedBuildings.clear();

                    SaveGame::getInstance()->loadFromRemoteString(gameSaveData);
                    isNewUser = false; // 标记为老玩家
                }
            }

            //如果是新玩家，才执行初始化逻辑
            if (isNewUser) {
                log("New user detected, initializing fresh game data.");

                // 这里执行你原来写在 startNewGame 里的初始化代码
                coin_count = 5000;
                water_count = 5000;
                gem_count = 500;
                coin_limit = 5000;
                water_limit = 5000;
                gem_limit = 5000;
                army_limit = 10;

                g_allPurchasedBuildings.clear(); // 确保容器是空的
            }

            // 登录成功，进入游戏场景
            // 此时数据状态已经正确（老玩家是加载的数据，新玩家是初始数据）
            this->startNewGame(); // 调用修改后的函数
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
void HelloWorld::menuLocalNewGame(Ref* pSender)
{
    log("Starting Local New Game (Offline Mode)...");

    // 1. 重置所有全局资源数据
    coin_count = 5000;
    water_count = 5000;
    gem_count = 500;

    coin_limit = 5000;
    water_limit = 5000;
    gem_limit = 5000;
    army_limit = 10; // 初始兵力上限

    // 2. 彻底清空建筑容器
    // 注意：必须先从父节点移除，再清空容器，防止野指针
    for (auto b : g_allPurchasedBuildings) {
        if (b && b->getParent()) {
            b->removeFromParent();
        }
    }
    g_allPurchasedBuildings.clear();

    // 3. 设置特殊的本地用户名
    // 这可以让 GameScene 知道现在是离线模式
    g_currentUsername = "LocalPlayer";

    // 4. (可选) 如果你想让“新游戏”覆盖掉之前的本地存档，可以在这里删除旧存档
    // SaveGame::getInstance()->deleteSaveFile(); 

    // 5. 直接进入游戏场景，不经过 loginToServer
    this->startNewGame();
}