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

 // 包含必要的头文件
#include "HelloWorldScene.h"
#include "GameScene.h"
#include "SaveGame.h"  // 用于本地存档功能
#include "network/HttpClient.h" // 用于HTTP网络请求
#include "ui/CocosGUI.h"        // 用于UI组件（如输入框）
#include "json/document.h"      // 用于JSON解析
#include "json/writer.h"        // 用于JSON写入
#include "json/stringbuffer.h"  // 用于JSON字符串缓冲区
using namespace cocos2d::network; // 使用网络命名空间
using namespace cocos2d::ui;      // 使用UI命名空间
USING_NS_CC;                      // 使用Cocos2d-x命名空间

// 声明全局变量（这些变量的定义在其他文件中）
extern int coin_count;      // 当前金币数量
extern int water_count;     // 当前水资源数量
extern int gem_count;       // 当前宝石数量
extern int coin_limit;      // 金币存储上限
extern int water_limit;     // 水资源存储上限
extern int gem_limit;       // 宝石存储上限
extern int army_limit;      // 军队数量上限
extern cocos2d::Vector<Building*> g_allPurchasedBuildings; // 所有已购买建筑的容器
// 定义全局变量：当前用户名（初始为空字符串）
std::string g_currentUsername = "";

// 创建场景的静态方法
Scene* HelloWorld::createScene()
{
    // 创建并返回HelloWorld场景
    return HelloWorld::create();
}

// 加载文件失败时的错误处理函数
static void problemLoading(const char* filename)
{
    // 打印错误信息
    printf("Error while loading: %s\n", filename);
    printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in HelloWorldScene.cpp\n");
}

// 初始化场景
bool HelloWorld::init()
{
    // 1. 调用父类Scene的初始化方法，如果失败则返回false
    if (!Scene::init())
    {
        return false;
    }

    // 获取屏幕可见区域的大小和原点坐标
    const auto visible_size = Director::getInstance()->getVisibleSize();
    const Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 2. 创建关闭按钮（位于屏幕右下角）
    auto close_item = MenuItemImage::create(
        "CloseNormal.png",       // 正常状态图片
        "CloseSelected.png",     // 选中状态图片
        CC_CALLBACK_1(HelloWorld::menuCloseCallback, this)); // 点击回调函数

    // 检查图片是否加载成功
    if (close_item == nullptr || close_item->getContentSize().width <= 0 || close_item->getContentSize().height <= 0)
    {
        problemLoading("'CloseNormal.png' and 'CloseSelected.png'");
    }
    else
    {
        // 设置关闭按钮位置：屏幕右上角
        float x = origin.x + visible_size.width - close_item->getContentSize().width / 2;
        float y = origin.y + close_item->getContentSize().height / 2;
        close_item->setPosition(Vec2(x, y));
    }

    // 创建菜单并添加关闭按钮
    auto menu = Menu::create(close_item, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);

    // 3. 创建平铺背景
    std::string filename = "TilesetField.png"; // 使用的图集文件
    const float tile_width = 22;  // 每个瓦片的宽度（原图截取）
    const float tile_height = 22; // 每个瓦片的高度（原图截取）
    Rect tile_rect = Rect(6, 105, tile_width, tile_height); // 在图集中截取瓦片的矩形区域
    const float scale_amount = 4.0f; // 瓦片的缩放比例
    const float step_x = tile_width * scale_amount; // 水平方向瓦片之间的间隔
    const float step_y = tile_height * scale_amount; // 垂直方向瓦片之间的间隔

    // 使用双重循环创建平铺背景
    for (float x = origin.x; x < origin.x + visible_size.width; x += step_x)
    {
        for (float y = origin.y; y < origin.y + visible_size.height; y += step_y)
        {
            auto tile = Sprite::create(filename, tile_rect); // 创建一个瓦片精灵
            tile->setAnchorPoint(Vec2::ZERO);                // 设置锚点为左下角
            tile->setScale(scale_amount);                    // 设置缩放
            tile->setPosition(x, y);                         // 设置位置
            tile->getTexture()->setAliasTexParameters();     // 设置纹理抗锯齿参数
            this->addChild(tile, -1);                        // 添加到场景，z-order为-1（在背景层）
        }
    }

    // 4. 创建游戏标题
    auto title = Label::createWithTTF("CLASH OF COCO", "fonts/Marker Felt.ttf", 56);
    if (title != nullptr)
    {
        title->setPosition(Vec2(origin.x + visible_size.width / 2, origin.y + visible_size.height - 100)); // 居中靠上
        title->setColor(Color3B::YELLOW); // 设置颜色为黄色
        title->enableOutline(Color4B::BLACK, 3); // 添加黑色描边
        this->addChild(title, 1);
    }

    // 创建用户名输入框
    auto edit_box = EditBox::create(Size(300, 60), Scale9Sprite::create("CloseNormal.png")); // 暂时借用CloseNormal.png作为背景
    edit_box->setPosition(Vec2(origin.x + visible_size.width / 2, origin.y + visible_size.height / 2 + 300));
    edit_box->setFontSize(30);
    edit_box->setFontColor(Color3B::BLACK);
    edit_box->setPlaceHolder("Enter Username:"); // 设置占位符文本
    edit_box->setPlaceholderFontColor(Color3B::GRAY);
    edit_box->setMaxLength(10); // 设置最大输入长度为10
    edit_box->setName("username_input"); // 设置节点名称，方便后续获取
    this->addChild(edit_box, 1);

    // 创建密码输入框
    auto password_box = EditBox::create(Size(300, 60), Scale9Sprite::create("CloseNormal.png"));
    password_box->setPosition(Vec2(origin.x + visible_size.width / 2, origin.y + visible_size.height / 2 + 200));
    password_box->setInputFlag(EditBox::InputFlag::PASSWORD); // 设置为密码模式（显示为星号）
    password_box->setPlaceHolder("Enter Password:");
    password_box->setName("password_input");
    this->addChild(password_box, 1);

    // 检查是否存在本地存档文件
    bool has_save_file = SaveGame::getInstance()->isSaveFileExist();
    float button_start_y = origin.y + visible_size.height / 2 + 50; // 按钮起始Y坐标

    // 5. 如果存在存档，创建"继续游戏"按钮
    if (has_save_file)
    {
        auto continue_label = Label::createWithTTF("LOCAL LOAD(READ FROM FILES)", "fonts/Marker Felt.ttf", 48);
        continue_label->setTextColor(Color4B::GREEN);
        continue_label->enableOutline(Color4B::BLACK, 2);

        auto continue_item = MenuItemLabel::create(continue_label, CC_CALLBACK_1(HelloWorld::menuContinueGame, this));
        continue_item->setPosition(Vec2(origin.x + visible_size.width / 2, button_start_y));

        auto start_menu = Menu::create(continue_item, NULL);
        start_menu->setPosition(Vec2::ZERO);
        this->addChild(start_menu, 1);

        button_start_y -= 80; // 调整下一个按钮的Y坐标
    }

    // 6. 创建"登录/开始游戏"按钮
    auto start_label = Label::createWithTTF("LOGIN/START", "fonts/Marker Felt.ttf", 48);
    start_label->setTextColor(Color4B::WHITE);
    start_label->enableOutline(Color4B::BLACK, 2);

    auto start_item = MenuItemLabel::create(start_label, CC_CALLBACK_1(HelloWorld::menuGotoNextScene, this));
    start_item->setPosition(Vec2(origin.x + visible_size.width / 2, button_start_y));

    auto start_menu = Menu::create(start_item, NULL);
    start_menu->setPosition(Vec2::ZERO);
    this->addChild(start_menu, 1);

    // 创建"本地新游戏"按钮（离线模式）
    auto local_new_game_label = Label::createWithTTF("LOCAL NEW GAME", "fonts/Marker Felt.ttf", 40);
    local_new_game_label->setTextColor(Color4B::YELLOW);
    local_new_game_label->enableOutline(Color4B::BLACK, 2);

    auto local_new_game_item = MenuItemLabel::create(local_new_game_label, CC_CALLBACK_1(HelloWorld::menuLocalNewGame, this));
    local_new_game_item->setPosition(Vec2(origin.x + visible_size.width / 2, button_start_y - 80));

    auto local_menu = Menu::create(local_new_game_item, NULL);
    local_menu->setPosition(Vec2::ZERO);
    this->addChild(local_menu, 1);

    return true; // 初始化成功
}

// 关闭按钮回调函数
void HelloWorld::menuCloseCallback(const Ref* p_sender)
{
    Director::getInstance()->end(); // 结束游戏
}

// 开始新游戏（切换到游戏场景）
void HelloWorld::startNewGame()
{
    auto scene = GameScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene)); // 使用淡入淡出效果切换场景
}

// 登录/开始游戏按钮回调函数
void HelloWorld::menuGotoNextScene(const Ref* p_sender)
{
    auto edit_box = static_cast<EditBox*>(this->getChildByName("username_input")); // 获取用户名输入框
    std::string name = edit_box->getText(); // 获取输入的用户名

    if (name.empty())
    {
        log("Please enter your name first!"); // 如果用户名为空，打印日志
        return;
    }

    this->loginToServer(name); // 调用登录服务器函数
}

// 继续游戏按钮回调函数（加载本地存档）
void HelloWorld::menuContinueGame(const Ref* p_sender)
{
    g_currentUsername = "LocalPlayer"; // 设置用户名为本地玩家
    log("Continue game...");

    // 尝试加载本地存档
    if (SaveGame::getInstance()->loadGameState())
    {
        log("Save game loaded successfully!");
        // 延迟0.1秒后切换到游戏场景，确保数据加载完成
        this->runAction(Sequence::create(
            DelayTime::create(0.1f),
            CallFunc::create([=]()
                {
                    auto scene = GameScene::createScene();
                    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
                }),
            nullptr
        ));
    }
    else
    {
        log("Failed to load save game!");
        const auto visible_size = Director::getInstance()->getVisibleSize();
        const Vec2 origin = Director::getInstance()->getVisibleOrigin();

        auto error_label = Label::createWithTTF("Failed to load save game!", "fonts/Marker Felt.ttf", 32);
        error_label->setPosition(origin.x + visible_size.width / 2, origin.y + visible_size.height / 2);
        error_label->setColor(Color3B::RED);
        this->addChild(error_label, 10);

        // 2秒后移除错误提示标签
        error_label->runAction(Sequence::create(
            DelayTime::create(2.0f),
            RemoveSelf::create(),
            nullptr
        ));
    }
}

// 登录服务器函数
void HelloWorld::loginToServer(const std::string& username)
{
    // 获取密码输入框中的密码
    auto password_box = static_cast<cocos2d::ui::EditBox*>(this->getChildByName("password_input"));
    std::string password = password_box ? password_box->getText() : "";

    auto request = new (std::nothrow) HttpRequest(); // 创建HTTP请求对象
    request->setUrl("http://100.80.248.229:5000/login"); // 设置请求URL
    request->setRequestType(HttpRequest::Type::POST); // 设置请求类型为POST

    std::vector<std::string> headers;
    headers.push_back("Content-Type: application/json; charset=utf-8"); // 设置请求头
    request->setHeaders(headers);

    // 设置响应回调函数
    request->setResponseCallback([=](HttpClient* client, HttpResponse* response)
        {
            if (!response || !response->isSucceed())
            {
                log("Network Error!"); // 网络错误
                return;
            }

            std::vector<char>* buffer = response->getResponseData(); // 获取响应数据
            std::string resultStr(buffer->begin(), buffer->end()); // 转换为字符串

            rapidjson::Document doc;
            doc.Parse(resultStr.c_str()); // 解析JSON数据

            if (doc.HasParseError())
            {
                log("JSON Parse Error from Server"); // JSON解析错误
                return;
            }

            if (doc.HasMember("status") && std::string(doc["status"].GetString()) == "success")
            {
                // 登录成功，初始化游戏数据
                coin_count = 5000;
                water_count = 5000;
                gem_count = 500;
                army_limit = 10;
                g_allPurchasedBuildings.clear();
                g_currentUsername = username; // 设置当前用户名

                bool is_new_user = true; // 标记是否为新用户

                // 尝试从服务器响应中获取存档数据
                if (doc.HasMember("data") && doc["data"].IsString())
                {
                    std::string game_save_data = doc["data"].GetString();

                    // 如果存档数据不为空，则加载存档
                    if (!game_save_data.empty() && game_save_data != "")
                    {
                        log("Loading game data for user: %s", username.c_str());
                        g_allPurchasedBuildings.clear(); // 清空建筑容器
                        SaveGame::getInstance()->loadFromRemoteString(game_save_data); // 从字符串加载存档
                        is_new_user = false; // 标记为老玩家
                    }
                }

                // 如果是新玩家，则执行初始化逻辑
                if (is_new_user)
                {
                    log("New user detected, initializing fresh game data.");
                    coin_count = 5000;
                    water_count = 5000;
                    gem_count = 500;
                    coin_limit = 5000;
                    water_limit = 5000;
                    gem_limit = 5000;
                    army_limit = 10;
                    g_allPurchasedBuildings.clear(); // 确保容器为空
                }

                this->startNewGame(); // 进入游戏场景
            }
            else
            {
                log("Login failed: Username or password error."); // 登录失败
            }
        });

    // 设置请求体（JSON格式）
    std::string post_data = "{\"username\":\"" + username + "\", \"password\":\"" + password + "\"}";
    request->setRequestData(post_data.c_str(), post_data.length());

    HttpClient::getInstance()->send(request); // 发送请求
    request->release(); // 释放请求对象
}

// 本地新游戏按钮回调函数（离线模式）
void HelloWorld::menuLocalNewGame(const Ref* p_sender)
{
    log("Starting Local New Game (Offline Mode)...");

    // 1. 重置所有全局资源数据
    coin_count = 5000;
    water_count = 5000;
    gem_count = 500;
    coin_limit = 5000;
    water_limit = 5000;
    gem_limit = 5000;
    army_limit = 10;

    // 2. 清空建筑容器
    for (auto b : g_allPurchasedBuildings)
    {
        if (b && b->getParent())
        {
            b->removeFromParent(); // 从父节点移除
        }
    }
    g_allPurchasedBuildings.clear();

    // 3. 设置本地用户名
    g_currentUsername = "LocalPlayer";

    // 4. 直接进入游戏场景
    this->startNewGame();
}