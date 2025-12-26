#include "GameScene.h"
#include "HelloWorldScene.h"
#include "BuildingInfoLayer.h"
#include "BattleScene.h"
#include "Building.h" 
#include "ShopScene.h"
#include "SaveGame.h"
#include "AudioEngine.h"
#include "network/HttpClient.h" // 必须
#include "json/document.h"
#include "json/writer.h"
#include "json/stringbuffer.h"
#include"Resource.h"
#include"PlayerListLayer.h"
using namespace cocos2d::network;
USING_NS_CC;

extern std::string g_currentUsername;

cocos2d::Vector<Building*> g_allPurchasedBuildings;// ???????????????????? 

int army_limit = 0;


Scene* GameScene::createScene(Building* purchasedBuilding) {
    auto scene = GameScene::create(); // ???? GameScene ???

    // ????????????????????????????????????��???????
    if (scene && purchasedBuilding) {
        // ????????????????
        purchasedBuilding->retain();
        g_allPurchasedBuildings.pushBack(purchasedBuilding);
        purchasedBuilding->release(); // GameScene?????????
    }
    return scene;
}


void GameScene::menuGotoBattleCallback(Ref* pSender)
{
    log("Starting Battle...");
    AudioEngine::stopAll();

    auto scene = BattleScene::createScene(1, "");
    Director::getInstance()->pushScene(TransitionFade::create(0.5f, scene));

}
bool GameScene::init()
{
    if (!Scene::init()) return false;

    extern std::string g_currentUsername;

    // 1. 【所有权初始化】
    // 如果是从 loadOtherPlayerData 传过来的值，则保持；如果是首次登录进入，则设为自己
    if (_currentSceneOwner.empty()) {
        _currentSceneOwner = g_currentUsername;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 2. 【加载地图】
    _tiledMap = TMXTiledMap::create("Grass.tmx");
    if (!_tiledMap) {
        log("Error: Failed to load map!");
        return false;
    }
    this->addChild(_tiledMap, 1);

    // 适配屏幕缩放
    Size mapSize = _tiledMap->getContentSize();
    float scaleX = visibleSize.width / mapSize.width;
    float scaleY = visibleSize.height / mapSize.height;
    float minScale = std::min(scaleX, scaleY);
    _tiledMap->setScale(minScale);

    this->checkAndClampMapPosition();
    this->recalculateArmyLimit();

    // 3. 【加载地图装饰物/障碍物】
    auto objectGroup = _tiledMap->getObjectGroup("Objects");
    if (objectGroup) {
        ValueVector objects = objectGroup->getObjects();
        for (const auto& v : objects) {
            ValueMap dict = v.asValueMap();
            if (dict.find("fileName") != dict.end()) {
                std::string path = dict["fileName"].asString();
                auto sprite = Sprite::create(path);
                if (sprite) {
                    _tiledMap->addChild(sprite, 1);
                    this->addObstacle(sprite);
                    sprite->setAnchorPoint(Vec2::ZERO);
                    float x = dict["x"].asFloat();
                    float y = dict["y"].asFloat();
                    sprite->setPosition(x, y + 150);
                    if (dict.find("width") != dict.end() && dict.find("height") != dict.end()) {
                        float width = dict["width"].asFloat();
                        float height = dict["height"].asFloat();

                        sprite->setScaleX(width / sprite->getContentSize().width);
                        sprite->setScaleY(height / sprite->getContentSize().height);
                    }
                    sprite->setLocalZOrder(10000 - (int)y);
                }
            }
        }
    }

    // 4. 【生成/加载建筑】
    if (_currentSceneOwner == g_currentUsername) {
        this->reloadBuildingsFromSave(); // 调用我们写好的同步函数
    }
    // 5. 【UI 资源条显示】
    myCoin = new goldcoin();
    myCoin->print(this);
    _coinTextLabel = this->showText("Coin " + std::to_string(coin_count), origin.x + visibleSize.width - 370, origin.y + visibleSize.height - 50, Color4B::WHITE);

    mywater = new water();
    mywater->print(this);
    _waterTextLabel = this->showText("Water " + std::to_string(water_count), origin.x + visibleSize.width - 370, origin.y + visibleSize.height - 120, Color4B::WHITE);

    mygem = new Gem();
    mygem->print(this);
    _gemTextLabel = this->showText("Gem " + std::to_string(gem_count), origin.x + visibleSize.width - 370, origin.y + visibleSize.height - 182, Color4B::WHITE);

    // 6. 【功能按钮逻辑分支】
    this->scheduleOnce([=](float dt) {
        extern std::string g_currentUsername;
        auto visibleSize = Director::getInstance()->getVisibleSize();
        auto origin = Director::getInstance()->getVisibleOrigin();

        // 判定：如果是自己的家 (业主等于当前登录用户)
        if (_currentSceneOwner == g_currentUsername) {
            // 显示原本的商店和保存按钮
            this->addShopButton();
            this->addSaveButton();

            // 在左下角添加原本的 Back 按钮

            auto backItem = MenuItemFont::create("Back(Save to Cloud)", CC_CALLBACK_1(GameScene::menuBackCallback, this));
            backItem->setColor(Color3B::YELLOW);
            backItem->setPosition(Vec2(origin.x + backItem->getContentSize().width / 2 + 20,
                origin.y + backItem->getContentSize().height / 2 + 20));

            auto menu = Menu::create(backItem, NULL);
            menu->setPosition(Vec2::ZERO);
            this->addChild(menu, 100);
        }
        else {
            
        }
        }, 0, "setup_ui_key");

    // 7. 【交互监听】
    isMapDragging = false;
    auto touchListener = EventListenerTouchOneByOne::create();
    touchListener->setSwallowTouches(true);
    touchListener->onTouchBegan = CC_CALLBACK_2(GameScene::onTouchBegan, this);
    touchListener->onTouchMoved = CC_CALLBACK_2(GameScene::onTouchMoved, this);
    touchListener->onTouchEnded = CC_CALLBACK_2(GameScene::onTouchEnded, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, _tiledMap);

    auto mouseListener = EventListenerMouse::create();
    mouseListener->onMouseScroll = CC_CALLBACK_1(GameScene::onMouseScroll, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);

    // 8. 【社交与自动保存逻辑】
    
    if (g_currentUsername != "LocalPlayer") {
        // 1. 创建文字标签 
        auto label = Label::createWithTTF("FRIENDS", "fonts/Marker Felt.ttf", 30);
        label->setColor(Color3B::WHITE);
        label->enableOutline(Color4B::BLACK, 2); // 加个黑边让字更清晰

        // 2. 创建点击事件
        auto socialItem = MenuItemLabel::create(label, [=](Ref* sender) {
            // A. 创建列表层
            auto playerLayer = PlayerListLayer::create();

            // B. 绑定回调函数：当列表里的某个人被点击时触发
            playerLayer->setOnVisitCallback([=](std::string targetName) {

                // 如果点击的是当前正在玩的人（自己），或者显示的是 My Home
                if (targetName == g_currentUsername) {
                    log("Loading my own home...");
                    // 如果你已经在自己家，其实不需要重新加载，但为了逻辑统一，可以重载一次

                }
                else {
                    log("Visiting neighbor: %s", targetName.c_str());
                    // 加载别人的数据
                    this->loadOtherPlayerData(targetName);
                }
                });

            // C. 添加到场景 (ZOrder设为 1000 保证盖在所有东西上面)
            this->addChild(playerLayer, 1000);

            // D. 显示动画
            playerLayer->show();
            });

        // 3. 设置按钮位置 
        // 假设放在屏幕左侧，稍微靠上一点的位置 (x=80, y=高度-200)
        // 你可以根据需要调整这个坐标
        socialItem->setPosition(Vec2(origin.x + 120, origin.y + visibleSize.height - 200));

        // 4. 加入菜单
        auto menu = Menu::create(socialItem, NULL);
        menu->setPosition(Vec2::ZERO);
        this->addChild(menu, 200);
    }
    // 只有在【自己家】才开启每 60 秒一次的自动保存
    if (_currentSceneOwner == g_currentUsername) {
        this->schedule(CC_CALLBACK_1(GameScene::onAutoSave, this), 10.0f, "auto_save_key");
        log("Auto-save ENABLED for: %s", _currentSceneOwner.c_str());
    }
    else {
        log("Visiting Mode: Auto-save DISABLED.");
    }

    return true;
}


Label* GameScene::showText(std::string content, float x, float y, cocos2d::Color4B color)
{
    auto label = Label::createWithTTF(content, "fonts/Marker Felt.ttf", 24);
    if (label) {
        label->setPosition(Vec2(x, y));
        label->setTextColor(color);
        this->addChild(label, 100);
    }
    return label;
}

void GameScene::reloadBuildingsFromSave() {
    if (!_tiledMap) return;

    // 1. 清理当前地图上旧的建筑
    for (auto b : _allBuildings) {
        if (b->getParent()) b->removeFromParent();
    }
    _allBuildings.clear();

    // 2. 核心判断：存档容器到底有没有货？
    if (g_allPurchasedBuildings.empty()) {
        // --- 情况 A：真的是新玩家（或存档还没加载到） ---
        Size mapSize = _tiledMap->getContentSize();
        Vec2 center = Vec2(mapSize.width / 2, mapSize.height / 2);

        // 创建初始建筑
        this->setbuilding("House.png", Rect::ZERO, "My House", 500, BuildingType::BASE, center);
        this->setbuilding("junying.png", Rect::ZERO, "My junying", 500, BuildingType::BARRACKS, center + Vec2(200, 0));
        log("reload: No data found, showing default buildings.");
    }
    else {
        // --- 情况 B：老玩家，且存档数据已经就位 ---
        for (auto& building : g_allPurchasedBuildings) {
            if (building) {
                if (building->getParent()) building->removeFromParent();
                _tiledMap->addChild(building, 2);
                _allBuildings.pushBack(building);
                this->addObstacle(building);
                building->setPosition(building->getPosition());
            }
        }
        log("reload: Successfully restored %d buildings.", (int)g_allPurchasedBuildings.size());
    }

    this->recalculateArmyLimit();
    this->updateResourceDisplay();
}

void GameScene::setbuilding(const std::string& filename, const cocos2d::Rect& rect, const std::string& name, int cost, BuildingType type1, cocos2d::Vec2 position)
{
    auto building = Building::create(filename, rect, name, cost, type1);

    building->setScale(0.5f);
    building->getTexture()->setAliasTexParameters();
    building->setPosition(position);

    building->setOnUpgradeCallback([=]() {

        if (type1 == BuildingType::BASE) {
            extern int coin_limit, water_limit;
            coin_limit += 1500;
            water_limit += 1500;
        }
        // 2. ??????? -> ????????? (?????��??????? army_limit)
        else if (type1 == BuildingType::BARRACKS) {
            this->recalculateArmyLimit();
            log("Barracks upgraded, army limit recalculated to %d", army_limit);
        }

        this->updateResourceDisplay();
        });

    // 4. ?????????
    this->addObstacle(building);
    _tiledMap->addChild(building, 1);
    _allBuildings.pushBack(building);

    // ������������Ǳ�Ӫ�����¼����˿�����
    if (type1 == BuildingType::BARRACKS) {
        this->recalculateArmyLimit();
    }

    // ���ؼ��޸ġ���ӵ�ȫ����������������ʱ�����ҵ�
    if (!g_allPurchasedBuildings.contains(building)) {
        building->retain();
        g_allPurchasedBuildings.pushBack(building);
        building->release();

        CCLOG("=== GameScene: Default building added to global container: %s (type: %d) ===",
            name.c_str(), (int)type1);
    }
}
void GameScene::checkAndClampMapPosition()
{
    // ��ȡ��Ļ�ߴ�
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // ��ȡ��ͼ��ǰ�����ź��ʵ�ʳߴ�
    // ���ݳߴ� * ���ű���
    float mapWidth = _tiledMap->getContentSize().width * _tiledMap->getScale();
    float mapHeight = _tiledMap->getContentSize().height * _tiledMap->getScale();

    // ��ȡ��ǰλ��
    Vec2 currentPos = _tiledMap->getPosition();

    // ==========================================
    // �����㷨������ X �� Y �������Χ
    // ==========================================

    // ԭ���
    // 1. �����ͼ����ĻС(���ź�С)��ͨ��������ʾ��
    // 2. �����ͼ����Ļ��(�������)����λ�� x ������ [minX, 0] ֮�䡣
    //    ��� x > 0����߾ͻ�©�ڱߡ�
    //    ��� x < minX���ұ߾ͻ�©�ڱߡ�

    // --- X ������ ---
    float minX = visibleSize.width - mapWidth; // ����һ������
    float maxX = 0;

    if (mapWidth < visibleSize.width) {
        // �����ͼ����Ļխ��ֱ�Ӿ���
        currentPos.x = (visibleSize.width - mapWidth) / 2;
    }
    else {
        // ������ [minX, 0] ֮��
        if (currentPos.x > maxX) currentPos.x = maxX;
        if (currentPos.x < minX) currentPos.x = minX;
    }

    // --- Y ������ ---
    float minY = visibleSize.height - mapHeight; // ����һ������
    float maxY = 0;

    if (mapHeight < visibleSize.height) {
        currentPos.y = (visibleSize.height - mapHeight) / 2;
    }
    else {
        if (currentPos.y > maxY) currentPos.y = maxY;
        if (currentPos.y < minY) currentPos.y = minY;
    }

    // Ӧ���������λ��
    _tiledMap->setPosition(currentPos);
}
void GameScene::onMouseScroll(Event* event)
{
    EventMouse* e = (EventMouse*)event;
    float scrollY = e->getScrollY();

    // 1. �����µ�����ֵ
    float factor = 0.1f;
    float newScale = _tiledMap->getScale() + (scrollY > 0 ? factor : -factor);

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Size mapSize = _tiledMap->getContentSize();

    float scaleX = visibleSize.width / mapSize.width;
    float scaleY = visibleSize.height / mapSize.height;
    float minLimit = std::min(scaleX, scaleY); // ����ǡ�ȫͼ��ʾ��������ֵ

    // ���Ʒ�Χ
    if (newScale < minLimit) newScale = minLimit; // ���ܱ�ȫͼ��С
    if (newScale > 2.0f) newScale = 2.0f;         // ���Ŵ���

    // ��������
    _tiledMap->setScale(newScale);

    // ����λ�� (��֤����ʱ���л�����)
    checkAndClampMapPosition();
}
bool GameScene::onTouchBegan(Touch* touch, Event* event)
{
    // ��¼��ʼ״̬����һ��������ק��Ҳ��������㽨��
    isMapDragging = false;
    return true; // ���뷵�� true ���ܽ��պ����� Moved �� Ended
}

void GameScene::onTouchMoved(Touch* touch, Event* event)
{
    // 1. ��ȡ�ƶ��ľ��� (Delta)
    Vec2 delta = touch->getDelta();

    // 2. ����ƶ����뼫С����������ָ����������
    if (delta.getLength() < 2.0f && !isMapDragging) {
        return;
    }

    // 3. ȷ��Ϊ��קģʽ
    isMapDragging = true;

    // 4. ������λ�ã���ǰλ�� + ƫ����
    Vec2 newPos = _tiledMap->getPosition() + delta;
    _tiledMap->setPosition(newPos);

    // 5. ���ؼ���ʵʱ����λ�ã���ֹ�ϳ���
    checkAndClampMapPosition();
}

void GameScene::onTouchEnded(Touch* touch, Event* event)
{

    isMapDragging = false;
}
void GameScene::menuBackCallback(Ref* pSender)
{
    log("Back button clicked! Syncing to cloud before exit...");
    extern std::string g_currentUsername;
    if (this->_currentSceneOwner == g_currentUsername) {
        SaveGame::getInstance()->saveGameState();
        log("Local save executed.");
    }

    // 2. [新增] 检查是否为离线模式
    // 如果是我们在 HelloWorld 里设置的 "LocalPlayer"，直接退出，不联网
    if (g_currentUsername == "LocalPlayer") {
        log("Offline Mode: Skipping cloud sync.");

        g_allPurchasedBuildings.clear(); // 清理内存
        auto scene = HelloWorld::createScene();
        Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
        return;
    }
    // --- 权限检查 (防止访问别人家时点返回把别人家的数据存到自己名下) ---
    extern std::string g_currentUsername;
    if (this->_currentSceneOwner != g_currentUsername) {
        log("Visiting mode: Skip cloud save, just return.");
        // 如果是访问模式，直接清空并返回，不触发保存
        g_allPurchasedBuildings.clear();
        auto scene = HelloWorld::createScene();
        Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
        return;
    }

    // 1. 获取 JSON (确保此时 g_allPurchasedBuildings 是最新的)
    std::string currentData = SaveGame::getInstance()->getGameStateAsJsonString();

    // 2. 准备网络请求
    auto request = new (std::nothrow) cocos2d::network::HttpRequest();
    request->setUrl("http://100.80.248.229:5000/save"); // 注意：这是你 Python 服务器的地址
    request->setRequestType(cocos2d::network::HttpRequest::Type::POST);

    // 设置请求头，告诉服务器我们要发 JSON
    std::vector<std::string> headers;
    headers.push_back("Content-Type: application/json; charset=utf-8");
    request->setHeaders(headers);

    // 3. 构造发送给服务器的完整包 (包含用户名和存档数据)
    // 假设你在 SharedData.h 中定义了全局变量 g_currentUsername
    extern std::string g_currentUsername;

    // 这里构造一个合法的 JSON 字符串
    std::string postData = "{\"username\":\"" + g_currentUsername + "\", \"gameData\":" + currentData + "}";
    request->setRequestData(postData.c_str(), postData.length());

    // 4. 设置回调：无论成功失败，最后都要切换场景
    request->setResponseCallback([=](cocos2d::network::HttpClient* client, cocos2d::network::HttpResponse* response) {
        if (response && response->isSucceed()) {
            log("Cloud Save Success!");
        }
        else {
            log("Cloud Save Failed!");
        }

        // 【关键修复点】：在离开场景前，必须彻底清空全局容器
        // 否则下次登录或切换回来时，旧的指针还在 vector 里，会导致建筑重叠或报错
        g_allPurchasedBuildings.clear();

        // 切换场景
        auto scene = HelloWorld::createScene();
        Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
        });

    cocos2d::network::HttpClient::getInstance()->send(request);
    request->release();
}

void GameScene::addShopButton() {
    // --- 核心修改：如果是访客模式，直接返回，不创建商店按钮 ---
    // 假设你的 GameScene 中有 _isVisitorMode 变量，或者通过用户名判断
    // 逻辑：如果当前查看的不是自己的基地，就不显示商店
    if (_currentSceneOwner != g_currentUsername) {
        return; 
    }
    // 或者如果你使用了之前定义的标记位：
    // if (_isVisitorMode) return; 
    // -------------------------------------------------------

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 1. 创建商店按钮精灵
    auto shopNormal = Sprite::create("ui/shop_normal.png"); 
    auto shopSelected = Sprite::create("ui/shop_selected.png"); 

    // 2. 容错处理：如果图片资源丢失，创建备用的彩色矩形按钮
    if (!shopNormal) {
        shopNormal = Sprite::create();
        shopNormal->setTextureRect(Rect(0, 0, 150, 80));
        shopNormal->setColor(Color3B(255, 200, 0)); 

        auto shopLabel = Label::createWithTTF("MARKET", "fonts/Marker Felt.ttf", 30);
        shopLabel->setPosition(Vec2(75, 40));
        shopLabel->setColor(Color3B::WHITE);
        shopNormal->addChild(shopLabel);
    }

    if (!shopSelected) {
        shopSelected = Sprite::create();
        shopSelected->setTextureRect(Rect(0, 0, 150, 80)); // 建议大小与Normal保持一致
        shopSelected->setColor(Color3B(255, 150, 0)); 
    }

    // 3. 创建菜单项
    auto shopItem = MenuItemSprite::create(shopNormal, shopSelected,
        [](Ref* pSender) {
            auto scene = ShopScene::createScene();
            Director::getInstance()->pushScene(TransitionFade::create(0.5f, scene));
        }
    );

    // 4. 设置按钮位置 (左上角)
    float x = origin.x + 150; 
    float y = origin.y + visibleSize.height - 100; 
    shopItem->setPosition(Vec2(x, y));

    // 5. 将按钮加入菜单并添加到层中
    auto shopMenu = Menu::create(shopItem, NULL);
    shopMenu->setPosition(Vec2::ZERO);
    this->addChild(shopMenu, 200); 
}

void GameScene::addAllPurchasedBuildings() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Size mapSize = _tiledMap->getContentSize();
    for (auto& building : g_allPurchasedBuildings) {
        if (building && !building->getParent()) {
            _tiledMap->addChild(building, 10);

            if (building->getPositionX() == 0 && building->getPositionY() == 0) {
                Vec2 mapCenter = Vec2(mapSize.width / 2, mapSize.height / 2);

                // �ӵ�ͼ���Ŀ�ʼ������Ŀ�λ
                Vec2 pos = getNearestFreePosition(building, mapCenter);

                building->setPosition(pos);
            }

            // ???????????????
            building->setOnUpgradeCallback([=]() {
                // ���¼����˿����ޣ�����Ǳ�Ӫ���Ӫ��
                if (building->getType() == BuildingType::BARRACKS) {
                    this->recalculateArmyLimit();
                }
                if (_coinTextLabel) {
                    std::string txt = "Coin " + std::to_string(coin_count) + "/" + std::to_string(coin_limit);
                    _coinTextLabel->setString(txt);
                }
                if (_waterTextLabel) {
                    std::string txt = "Water " + std::to_string(water_count) + "/" + std::to_string(water_limit);
                    _waterTextLabel->setString(txt);
                }
                if (_gemTextLabel) {
                    std::string txt = "Gem " + std::to_string(gem_count) + "/" + std::to_string(gem_limit);
                    _gemTextLabel->setString(txt);
                }
                // ������Դ��ʾ
                this->updateResourceDisplay();
                });

            // ?????????????
            _allBuildings.pushBack(building);
        }
    }
    // ����������������н��������¼����˿�����
    this->recalculateArmyLimit();
}

void GameScene::onEnter() {
    cocos2d::Scene::onEnter();

    extern std::string g_currentUsername;
    AudioEngine::stopAll();

    // 【关键修复 1】：进入场景前，清空当前场景的逻辑容器，防止重复添加
    _allBuildings.clear();
    // 如果你有 _obstacles 容器，也建议清空一下
    // _obstacles.clear(); 

    // 【核心渲染逻辑】
    for (auto& building : g_allPurchasedBuildings) {
        if (building) {
            // 【关键修复 2】：强行断开旧父节点。
            // 只有先执行这个，addChild 到新地图才不会报错
            if (building->getParent()) {
                building->removeFromParent();
            }

            // 添加到当前地图
            _tiledMap->addChild(building, 10);
            building->scheduleUpdate();
            // 【关键修复 3】：只有真正坐标为 0 的新建筑才自动找位置
            // 已经保存过坐标的老建筑不要去动它的 Position
            if (building->getPositionX() == 0 && building->getPositionY() == 0) {
                Size mapSize = _tiledMap->getContentSize();
                Vec2 center = Vec2(mapSize.width / 2, mapSize.height / 2);
                Vec2 pos = getNearestFreePosition(building, center);
                building->setPosition(pos);
            }

            // 重新绑定碰撞逻辑（障碍物列表）
            this->addObstacle(building);

            // 重新绑定回调（确保 UI 刷新逻辑指向当前最新的场景实例）
            building->setOnUpgradeCallback([=]() {
                if (building->getType() == BuildingType::BARRACKS) {
                    this->recalculateArmyLimit();
                }
                this->updateResourceDisplay(); // 建议直接封装一个总刷新函数
                });

            // 加入当前场景的活跃建筑容器
            _allBuildings.pushBack(building);
        }
    }

    // 刷新显示
    this->recalculateArmyLimit();
    this->updateResourceDisplay();
    AudioEngine::play2d("music/1.ogg", true, 0.5f);
}

void GameScene::updateResourceDisplay() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // ?????????
    if (_coinTextLabel) {
        std::string txt = "Coin " + std::to_string(coin_count) + "/" + std::to_string(coin_limit);
        _coinTextLabel->setString(txt);
    }
    if (myCoin) {
        myCoin->refresh();
    }

    // ????????
    if (_waterTextLabel) {
        std::string txt = "Water " + std::to_string(water_count) + "/" + std::to_string(water_limit);
        _waterTextLabel->setString(txt);
    }
    if (mywater) {
        mywater->refresh();
    }

    // ???��?????
    if (_gemTextLabel) {
        std::string txt = "Gem " + std::to_string(gem_count) + "/" + std::to_string(gem_limit);
        _gemTextLabel->setString(txt);
    }
    if (mygem) {
        mygem->refresh();
    }
}
// GameScene.cpp



void GameScene::removeObstacle(Node* node)
{
    if (_obstacles.contains(node)) {
        _obstacles.eraseObject(node);
    }
}

void GameScene::addObstacle(Node* node) {
    if (node && !_obstacles.contains(node)) {
        _obstacles.pushBack(node);
    }
}

Rect GameScene::getWorldBoundingBox(Node* node)
{
    if (!node->getParent()) return Rect::ZERO;

    Rect rect = node->getBoundingBox();
    Vec2 worldOrigin = node->getParent()->convertToWorldSpace(rect.origin);
    Rect worldRect = Rect(worldOrigin.x, worldOrigin.y, rect.size.width, rect.size.height);
    return worldRect;
}

bool GameScene::checkCollision(Rect targetRect, Node* ignoreNode)
{
    for (auto obstacle : _obstacles)
    {
        if (obstacle == ignoreNode) continue;

        // ??????????????��
        if (!obstacle->getParent()) continue;

        // ?????????????��??
        Rect obstacleRect = obstacle->getBoundingBox();

        // ??????????????��??????????�ʦ�??????
        if (targetRect.intersectsRect(obstacleRect))
        {
            return true; // ???
        }
    }
    return false; // ???
}
Vec2 GameScene::getNearestFreePosition(Building* building, Vec2 targetMapPos)
{
    Size size = building->getContentSize();
    float w = size.width * building->getScaleX();
    float h = size.height * building->getScaleY();
    // 2. ????????
    int step = (int)(w / 2);
    if (step < 10) step = 10;
    int maxRadius = 30;

    for (int r = 1; r <= maxRadius; r++) {
        for (int x = -r; x <= r; x++) {
            for (int y = -r; y <= r; y++) {
                if (std::abs(x) != r && std::abs(y) != r) continue;

                Vec2 candidateMapPos = targetMapPos + Vec2(x * step, y * step);

                // ====================================================
                // ���޸������Ҫת WorldSpace��ֱ���� Local �㣡
                // ====================================================

                // A. ����� candidateMapPos ������� Local ��
                // B. ����� w �� h ����Ҳ�� Local �� (Building ��ԭʼ��С)
                Rect testLocalRect = Rect(
                    candidateMapPos.x - w * 0.5f,
                    candidateMapPos.y - h * 0.5f,
                    w, h
                );

                // C. ֱ�Ӵ��� Local Rect
                if (!checkCollision(testLocalRect, nullptr)) {
                    return candidateMapPos;
                }
            }
        }
    }
    return Vec2::ZERO;
}
void GameScene::addSaveButton() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // �������水ť
    auto saveLabel = Label::createWithTTF("SAVE GAME \n(Save to local device)", "fonts/Marker Felt.ttf", 28);
    saveLabel->setColor(Color3B::GREEN);
    saveLabel->enableOutline(Color4B::BLACK, 2);

    auto saveItem = MenuItemLabel::create(saveLabel,
        CC_CALLBACK_1(GameScene::menuSaveGameCallback, this));

    // ���ð�ťλ�ã����½ǣ�
    float x = origin.x + visibleSize.width - 150;
    float y = origin.y + 100;

    saveItem->setPosition(Vec2(x, y));

    // �����������԰�ť
    auto debugLabel = Label::createWithTTF("DEBUG SAVE", "fonts/Marker Felt.ttf", 20);
    debugLabel->setColor(Color3B::YELLOW);

    auto debugItem = MenuItemLabel::create(debugLabel,
        [](Ref* pSender) {
            CCLOG("=== Debug Save System ===");
            SaveGame::getInstance()->debugWritablePath();

            // ���Ա���ͼ��ز�������
            auto saveGame = SaveGame::getInstance();
            bool saveResult = saveGame->saveGameState("test_save.json");
            CCLOG("Test save result: %s", saveResult ? "SUCCESS" : "FAILED");

            bool loadResult = saveGame->loadGameState("test_save.json");
            CCLOG("Test load result: %s", loadResult ? "SUCCESS" : "FAILED");
        });

    // �����˵�����
    auto saveMenu = Menu::create(saveItem, NULL);
    saveMenu->setPosition(Vec2::ZERO);
    this->addChild(saveMenu, 200);
}

void GameScene::menuSaveGameCallback(Ref* pSender) {
    log("Saving game...");

    // ����SaveGame����������Ϸ
    SaveGame::getInstance()->saveGameState();
}

int GameScene::calculateArmyLimit()
{
    int totalLimit = 0; // �����˿�����

    // �������н������ҵ���Ӫ���ۼ������ṩ���˿�����
    for (auto& building : g_allPurchasedBuildings) {
        if (building && building->getType() == BuildingType::BARRACKS) {
            // ÿ����Ӫ�ṩ�Ļ����˿� + ÿ���������ӵ��˿�
            // ���磺1����Ӫ�ṩ10�˿ڣ�ÿ��һ������10�˿�
            totalLimit += 10 * building->getLevel();
        }
    }


    CCLOG("=== GameScene: Calculated army limit: %d ===", totalLimit);
    return totalLimit;
}

void GameScene::recalculateArmyLimit()
{
    army_limit = calculateArmyLimit();
    CCLOG("=== GameScene: Army limit recalculated to: %d ===", army_limit);

    // ����UI��ʾ�������Ҫ��
    this->updateResourceDisplay();
}

// GameScene.cpp

// 1. 通用的气泡提示函数 (Toast)
void GameScene::showToast(std::string message, Color3B color) {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto label = Label::createWithTTF(message, "fonts/Marker Felt.ttf", 30);
    label->setColor(color);
    label->enableOutline(Color4B::BLACK, 2);
    label->setPosition(visibleSize.width / 2, visibleSize.height / 2);
    this->addChild(label, 1000); // 放在最上层

    // 动画：出现 -> 停留 -> 上浮消失
    label->runAction(Sequence::create(
        FadeIn::create(0.2f),
        DelayTime::create(1.0f),
        Spawn::create(MoveBy::create(0.5f, Vec2(0, 50)), FadeOut::create(0.5f), nullptr),
        RemoveSelf::create(),
        nullptr
    ));
}

void GameScene::onManualSaveClicked() {
    // 只有在自己的基地时才允许存盘
    extern std::string g_currentUsername;
    if (this->_currentSceneOwner != g_currentUsername) {
        this->showToast("Cannot save while visiting!", Color3B::RED);
        return;
    }

    // 调用 SaveGame 的同步逻辑，并传入 Lambda 回调来显示 UI 反馈
    SaveGame::getInstance()->syncDataToCloud([this](bool success) {
        if (success) {
            this->showToast("Save Success!", Color3B::GREEN);
        }
        else {
            this->showToast("Save Failed!", Color3B::RED);
        }
        });
}

// 3. 定时器回调
void GameScene::onAutoSave(float dt) {
    log("Auto-saving...");
    SaveGame::getInstance()->syncDataToCloud([this](bool success) {
        if (success) {
            log("Auto save success.");
        }
        });
}

// GameScene.cpp

void GameScene::setupLeftPanel() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 1. 创建左侧背景条
    auto panel = LayerColor::create(Color4B(0, 0, 0, 150), 120, visibleSize.height - 100);
    panel->setPosition(origin.x, origin.y + 50);
    this->addChild(panel, 100);

    // 2. 标题
    auto title = Label::createWithTTF("Neighbors", "fonts/Marker Felt.ttf", 20);
    title->setPosition(60, panel->getContentSize().height - 20);
    panel->addChild(title);

    // 3. 从服务器获取列表
    auto request = new HttpRequest();
    request->setUrl("http://100.80.248.229:5000/users");
    request->setRequestType(HttpRequest::Type::GET);
    request->setResponseCallback([=](HttpClient* client, HttpResponse* response) {
        if (!response || !response->isSucceed()) return;

        std::vector<char>* buffer = response->getResponseData();
        std::string resultStr(buffer->begin(), buffer->end());

        rapidjson::Document doc;
        doc.Parse(resultStr.c_str());

        if (doc.HasMember("users") && doc["users"].IsArray()) {
            auto users = doc["users"].GetArray();

            // 简单创建一个菜单列表
            auto menu = Menu::create();
            float startY = panel->getContentSize().height - 60;

            // --- 新增：在列表最上方添加“回到自己家”的按钮 ---
            extern std::string g_currentUsername;
            auto homeLabel = Label::createWithTTF("MY HOME", "fonts/Marker Felt.ttf", 20);
            homeLabel->setColor(Color3B::YELLOW); // 用黄色区分
            auto homeItem = MenuItemLabel::create(homeLabel, [=](Ref* sender) {
                log("Returning to own home: %s", g_currentUsername.c_str());
                this->loadOtherPlayerData(g_currentUsername); // 加载自己的数据
                });
            homeItem->setPosition(60, startY);
            menu->addChild(homeItem);
            // ------------------------------------------

            for (int i = 0; i < users.Size(); i++) {
                std::string name = users[i].GetString();
                if (name == g_currentUsername) continue; // 列表里依然跳过自己

                auto label = Label::createWithTTF(name, "fonts/Marker Felt.ttf", 18);
                auto item = MenuItemLabel::create(label, [=](Ref* sender) {
                    this->loadOtherPlayerData(name);
                    });
                // i + 1 是为了给上面的 My Home 留出位置
                item->setPosition(60, startY - ((i + 1) * 30));
                menu->addChild(item);
            }
            menu->setPosition(Vec2::ZERO);
            panel->addChild(menu);
        }
        });
    HttpClient::getInstance()->send(request);
    request->release();
}

void GameScene::loadOtherPlayerData(std::string targetUsername) {
    // 请求那个人的数据
    auto request = new HttpRequest();
    request->setUrl("http://100.80.248.229:5000/get_user_data");
    request->setRequestType(HttpRequest::Type::POST);

    std::string postData = "{\"username\":\"" + targetUsername + "\"}";
    request->setRequestData(postData.c_str(), postData.length());
    request->setHeaders({ "Content-Type: application/json" });

    request->setResponseCallback([=](HttpClient* client, HttpResponse* response) {
        if (response && response->isSucceed()) {
            std::vector<char>* buffer = response->getResponseData();
            std::string res(buffer->begin(), buffer->end());
            rapidjson::Document doc;
            doc.Parse(res.c_str());

            if (doc.HasMember("data") && doc["data"].IsString()) {
                std::string saveData = doc["data"].GetString();

                extern std::string g_currentUsername;
                bool isMe = (targetUsername == g_currentUsername);

                // 直接跳转到 BattleScene，把好友的 JSON 传过去
                auto scene = BattleScene::createScene(0, saveData);
                Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));

                log("Switching to BattleScene to view %s's base", targetUsername.c_str());
                
            }
        }
        });
    HttpClient::getInstance()->send(request);
}

void GameScene::menuAttackCallback(Ref* pSender) {
    log("Attacking player: %s", _currentSceneOwner.c_str());

    // 1. 获取当前好友场景的建筑数据 (转为 JSON 字符串)
    // 这里的逻辑应类似于 SaveGame::getGameStateAsJsonString()，
    // 但它是针对当前 _allBuildings 容器中的实时建筑
    std::string pvpData = SaveGame::getInstance()->getGameStateAsJsonString();

    // 关键修复：检查数据是否有效，防止传给 BattleScene 一个空的 JSON
    if (pvpData.empty() || pvpData == "{\"buildings\":[]}") {
        log("Error: Target house has no buildings to attack!");
        return;
    }

    // 2. 停止背景音乐并切换到战斗场景
    AudioEngine::stopAll();

    // 传入 levelIndex 为 0 表示 PVP 模式，并传入好友的建筑数据
    auto scene = BattleScene::createScene(0, pvpData);
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
}