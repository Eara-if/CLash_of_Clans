//游戏主场景（家园场景）及主场景上的各类功能的实现
#include "GameScene.h"
#include "HelloWorldScene.h"
#include "BuildingInfoLayer.h"
#include "BattleScene.h"
#include "Building.h" 
#include "ShopScene.h"
#include "SaveGame.h"
#include "AudioEngine.h"
#include "network/HttpClient.h" // 网络请求必须
#include "json/document.h"
#include "json/writer.h"
#include "json/stringbuffer.h"
#include"Resource.h"
#include"PlayerListLayer.h"
using namespace cocos2d::network;
USING_NS_CC;

// 全局变量：当前用户名
extern std::string g_currentUsername;

// 全局变量：所有已购买的建筑容器
cocos2d::Vector<Building*> g_allPurchasedBuildings;

// 全局变量：军队上限
int army_limit = 0;

// 创建游戏场景的静态方法
Scene* GameScene::createScene(Building* purchased_building)
{
    // 创建GameScene实例
    auto scene = GameScene::create();

    // 如果场景创建成功且有购买的建筑传入，则添加到全局建筑容器中
    if (scene && purchased_building)
    {
        // 增加引用计数以防止被意外释放
        purchased_building->retain();
        // 添加到全局建筑容器
        g_allPurchasedBuildings.pushBack(purchased_building);
        // 释放引用计数，GameScene会持有该建筑
        purchased_building->release();
    }
    return scene;
}

// 前往战斗场景的回调函数
void GameScene::menuGotoBattleCallback(Ref* p_sender)
{
    log("Starting Battle...");
    // 停止所有背景音乐
    AudioEngine::stopAll();

    // 创建战斗场景（参数1表示关卡索引，空字符串表示没有额外数据）
    auto scene = BattleScene::createScene(1, "");
    // 使用淡入淡出效果切换场景
    Director::getInstance()->pushScene(TransitionFade::create(0.5f, scene));
}

// 游戏场景初始化函数
bool GameScene::init()
{
    // 调用父类Scene的初始化
    if (!Scene::init())
    {
        return false;
    }

    // 声明外部全局变量
    extern std::string g_currentUsername;

    // 1. 【所有权初始化】如果当前场景所有者未设置，则设置为当前用户
    if (current_scene_owner_.empty())
    {
        current_scene_owner_ = g_currentUsername;
    }

    // 2. 【加载地图】加载TMX地图文件
    if (!loadMap())
    {
        return false;
    }

    // 3. 【加载地图装饰物/障碍物】从地图对象层加载装饰物
    loadMapDecorations();

    // 4. 【生成/加载建筑】根据用户身份加载建筑
    loadBuildings();

    // 5. 【UI 资源条显示】初始化资源显示UI
    initResourceDisplay();

    // 6. 【功能按钮逻辑分支】初始化UI按钮
    initUIButtons();

    // 7. 【交互监听】初始化触摸和鼠标输入监听
    initInputListeners();

    // 8. 【社交与自动保存逻辑】初始化社交功能和自动保存
    initSocialAndAutoSave();

    return true;
}

// 加载地图函数
bool GameScene::loadMap()
{
    // 获取屏幕可见区域大小和原点
    const auto visible_size = Director::getInstance()->getVisibleSize();
    const Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 创建TMX瓦片地图
    tiled_map_ = TMXTiledMap::create("Grass.tmx");
    // 检查地图是否加载成功
    if (!tiled_map_)
    {
        log("Error: Failed to load map!");
        return false;
    }

    // 将地图添加到场景中，层级为1
    this->addChild(tiled_map_, 1);

    // 适配屏幕缩放：计算地图缩放比例
    const Size map_size = tiled_map_->getContentSize();
    float scale_x = visible_size.width / map_size.width;
    float scale_y = visible_size.height / map_size.height;
    float min_scale = std::min(scale_x, scale_y);
    // 设置地图缩放比例
    tiled_map_->setScale(min_scale);

    // 检查并限制地图位置，防止地图超出屏幕
    this->checkAndClampMapPosition();

    // 重新计算军队上限
    this->recalculateArmyLimit();

    return true;
}

// 加载地图装饰物函数
void GameScene::loadMapDecorations()
{
    // 从TMX地图获取对象层
    auto object_group = tiled_map_->getObjectGroup("Objects");
    if (!object_group)
    {
        return;
    }

    // 获取对象层中的所有对象
    const ValueVector& objects = object_group->getObjects();
    // 遍历所有对象
    for (const auto& v : objects)
    {
        // 将Value对象转换为ValueMap字典
        ValueMap dict = v.asValueMap();
        // 检查字典中是否有fileName键（表示装饰物图片路径）
        if (dict.find("fileName") != dict.end())
        {
            // 获取图片路径
            std::string path = dict["fileName"].asString();
            // 创建精灵
            auto sprite = Sprite::create(path);
            if (sprite)
            {
                // 将精灵添加到地图上，层级为1
                tiled_map_->addChild(sprite, 1);
                // 将精灵添加到障碍物列表
                this->addObstacle(sprite);
                // 设置锚点为左下角
                sprite->setAnchorPoint(Vec2::ZERO);
                // 获取对象在TMX中的位置
                float x = dict["x"].asFloat();
                float y = dict["y"].asFloat();
                // 设置精灵位置（Y坐标加150作为调整）
                sprite->setPosition(x, y + 150);

                // 如果有宽度和高度信息，则按比例缩放精灵
                if (dict.find("width") != dict.end() && dict.find("height") != dict.end())
                {
                    float width = dict["width"].asFloat();
                    float height = dict["height"].asFloat();
                    sprite->setScaleX(width / sprite->getContentSize().width);
                    sprite->setScaleY(height / sprite->getContentSize().height);
                }

                // 根据Y坐标设置局部渲染顺序，实现近大远小的视觉效果
                sprite->setLocalZOrder(10000 - (int)y);
            }
        }
    }
}

// 加载建筑函数
void GameScene::loadBuildings()
{
    // 声明外部全局变量
    extern std::string g_currentUsername;
    // 如果当前场景所有者是当前用户，则从存档重新加载建筑
    if (current_scene_owner_ == g_currentUsername)
    {
        this->reloadBuildingsFromSave();
    }
}

// 初始化资源显示函数
void GameScene::initResourceDisplay()
{
    // 获取屏幕可见区域大小和原点
    const auto visible_size = Director::getInstance()->getVisibleSize();
    const Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 创建金币对象并显示
    my_coin_ = new goldcoin();
    my_coin_->print(this);
    // 显示金币数量文本标签
    coin_text_label_ = this->showText("Coin " + std::to_string(coin_count),
        origin.x + visible_size.width - 370,
        origin.y + visible_size.height - 50,
        Color4B::WHITE);

    // 创建圣水对象并显示
    my_water_ = new water();
    my_water_->print(this);
    // 显示圣水数量文本标签
    water_text_label_ = this->showText("Water " + std::to_string(water_count),
        origin.x + visible_size.width - 370,
        origin.y + visible_size.height - 120,
        Color4B::WHITE);

    // 创建宝石对象并显示
    my_gem_ = new Gem();
    my_gem_->print(this);
    // 显示宝石数量文本标签
    gem_text_label_ = this->showText("Gem " + std::to_string(gem_count),
        origin.x + visible_size.width - 370,
        origin.y + visible_size.height - 182,
        Color4B::WHITE);
}

// 初始化UI按钮函数
void GameScene::initUIButtons()
{
    // 延迟一帧执行UI按钮创建，避免与其他初始化冲突
    this->scheduleOnce([=](float dt)
        {
            // 声明外部全局变量
            extern std::string g_currentUsername;
            const auto visible_size = Director::getInstance()->getVisibleSize();
            const auto origin = Director::getInstance()->getVisibleOrigin();

            // 如果当前场景所有者是当前用户，则添加商店按钮、保存按钮和返回按钮
            if (current_scene_owner_ == g_currentUsername)
            {
                // 添加商店按钮
                this->addShopButton();
                // 添加保存按钮
                this->addSaveButton();

                // 创建返回按钮（带云保存功能）
                if (g_currentUsername != "LocalPlayer")
                {
                    auto back_item = MenuItemFont::create("Back(Save to Cloud)",
                        CC_CALLBACK_1(GameScene::menuBackCallback, this));
                    back_item->setColor(Color3B::YELLOW);
                    back_item->setPosition(Vec2(origin.x + back_item->getContentSize().width / 2 + 20,
                        origin.y + back_item->getContentSize().height / 2 + 20));

                    // 创建菜单并添加返回按钮
                    auto menu = Menu::create(back_item, NULL);
                    menu->setPosition(Vec2::ZERO);
                    this->addChild(menu, 100);
                }
            }
        }, 0, "setup_ui_key");
}

// 初始化输入监听函数
void GameScene::initInputListeners()
{
    // 初始化地图拖动标志为false
    is_map_dragging_ = false;

    // 创建单点触摸事件监听器
    auto touch_listener = EventListenerTouchOneByOne::create();
    // 设置是否吞噬触摸事件
    touch_listener->setSwallowTouches(true);
    // 设置触摸开始回调
    touch_listener->onTouchBegan = CC_CALLBACK_2(GameScene::onTouchBegan, this);
    // 设置触摸移动回调
    touch_listener->onTouchMoved = CC_CALLBACK_2(GameScene::onTouchMoved, this);
    // 设置触摸结束回调
    touch_listener->onTouchEnded = CC_CALLBACK_2(GameScene::onTouchEnded, this);
    // 将触摸监听器添加到事件分发器中，优先级设置为地图
    _eventDispatcher->addEventListenerWithSceneGraphPriority(touch_listener, tiled_map_);

    // 创建鼠标事件监听器
    auto mouse_listener = EventListenerMouse::create();
    // 设置鼠标滚轮滚动回调
    mouse_listener->onMouseScroll = CC_CALLBACK_1(GameScene::onMouseScroll, this);
    // 将鼠标监听器添加到事件分发器中，优先级设置为当前场景
    _eventDispatcher->addEventListenerWithSceneGraphPriority(mouse_listener, this);
}

// 初始化社交和自动保存函数
void GameScene::initSocialAndAutoSave()
{
    // 声明外部全局变量
    extern std::string g_currentUsername;
    const auto visible_size = Director::getInstance()->getVisibleSize();
    const Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 如果当前用户不是本地玩家（离线模式），则显示好友列表按钮
    if (g_currentUsername != "LocalPlayer")
    {
        // 创建"FRIENDS"文本标签
        auto label = Label::createWithTTF("FRIENDS", "fonts/Marker Felt.ttf", 30);
        label->setColor(Color3B::WHITE);
        label->enableOutline(Color4B::BLACK, 2);

        // 创建社交按钮菜单项
        auto social_item = MenuItemLabel::create(label, [=](Ref* sender)
            {
                // 创建玩家列表层
                auto player_layer = PlayerListLayer::create();

                // 设置访问回调函数，当点击访问其他玩家时触发
                player_layer->setOnVisitCallback([=](std::string targetName)
                    {
                        // 如果目标玩家不是当前用户，则加载该玩家的数据
                        if (targetName != g_currentUsername)
                        {
                            log("Visiting neighbor: %s", targetName.c_str());
                            this->loadOtherPlayerData(targetName);
                        }
                    });

                // 将玩家列表层添加到场景中
                this->addChild(player_layer, 1000);
                // 显示玩家列表层
                player_layer->show();
            });

        // 设置社交按钮位置
        social_item->setPosition(Vec2(origin.x + 120, origin.y + visible_size.height - 200));
        // 创建菜单并添加社交按钮
        auto menu = Menu::create(social_item, NULL);
        menu->setPosition(Vec2::ZERO);
        this->addChild(menu, 200);
    }

    // 如果当前场景所有者是当前用户，则启用自动保存定时器
    if (current_scene_owner_ == g_currentUsername)
    {
        // 每10秒执行一次自动保存
        this->schedule(CC_CALLBACK_1(GameScene::onAutoSave, this), 10.0f, "auto_save_key");
        log("Auto-save ENABLED for: %s", current_scene_owner_.c_str());
    }
    else
    {
        // 如果是访问模式，则不启用自动保存
        log("Visiting Mode: Auto-save DISABLED.");
    }
}

// 显示文本函数
Label* GameScene::showText(const std::string& content, float x, float y, const cocos2d::Color4B& color)
{
    // 创建文本标签
    auto label = Label::createWithTTF(content, "fonts/Marker Felt.ttf", 24);
    if (label)
    {
        // 设置标签位置
        label->setPosition(Vec2(x, y));
        // 设置文本颜色
        label->setTextColor(color);
        // 将标签添加到场景中，层级为100
        this->addChild(label, 100);
    }
    return label;
}

// 从存档重新加载建筑函数
void GameScene::reloadBuildingsFromSave()
{
    // 如果地图不存在，直接返回
    if (!tiled_map_)
    {
        return;
    }

    // 1. 清理当前地图上旧的建筑
    for (auto b : all_buildings_)
    {
        if (b->getParent())
        {
            b->removeFromParent();
        }
    }
    // 清空当前场景的建筑容器
    all_buildings_.clear();

    // 【新增】重置资源上限为初始值（假设初始值为5000）
    extern int coin_limit, water_limit;
    coin_limit = 5000;
    water_limit = 5000;

    // 2. 核心判断：存档容器到底有没有货？
    if (g_allPurchasedBuildings.empty())
    {
        // --- 情况 A：真的是新玩家（或存档还没加载到） ---
        const Size map_size = tiled_map_->getContentSize();
        Vec2 center = Vec2(map_size.width / 2, map_size.height / 2);

        // 创建初始建筑
        this->setbuilding("House.png", Rect::ZERO, "My House", 500, BuildingType::BASE, center);
        this->setbuilding("junying.png", Rect::ZERO, "My junying", 500, BuildingType::BARRACKS, center + Vec2(200, 0));
        log("reload: No data found, showing default buildings.");
    }
    else
    {
        // --- 情况 B：老玩家，且存档数据已经就位 ---
        for (auto& building : g_allPurchasedBuildings)
        {
            if (building)
            {
                // 如果建筑已有父节点，先移除
                if (building->getParent())
                {
                    building->removeFromParent();
                }
                // 将建筑添加到地图上，层级为2
                tiled_map_->addChild(building, 2);
                // 添加到当前场景的建筑容器
                all_buildings_.pushBack(building);
                // 将建筑添加到障碍物列表
                this->addObstacle(building);
                // 设置建筑位置
                building->setPosition(building->getPosition());

                // 【新增】根据建筑等级初始化资源上限
                BuildingType type = building->getType();
                int level = building->getLevel();

                if (type == BuildingType::BASE)
                {
                    // 大本营1级不加，2级开始每级+1500
                    coin_limit += 1500 * (level - 1);
                    water_limit += 1500 * (level - 1);
                }
                else if (type == BuildingType::GOLD_STORAGE)
                {
                    // 金币储存器每级都加
                    coin_limit += 1500 * level;
                }
                else if (type == BuildingType::WATER_STORAGE)
                {
                    // 圣水储存器每级都加
                    water_limit += 1500 * level;
                }
            }
        }
        log("reload: Successfully restored %d buildings.", (int)g_allPurchasedBuildings.size());
    }

    // 重新计算军队上限
    this->recalculateArmyLimit();
    // 更新资源显示
    this->updateResourceDisplay();
}

// 设置建筑函数
void GameScene::setbuilding(const std::string& filename, const cocos2d::Rect& rect, const std::string& name, int cost, BuildingType type, const cocos2d::Vec2& position)
{
    // 创建建筑对象
    auto building = Building::create(filename, rect, name, cost, type);

    // 设置建筑缩放比例为0.5
    building->setScale(0.5f);
    // 设置纹理抗锯齿参数
    building->getTexture()->setAliasTexParameters();
    // 设置建筑位置
    building->setPosition(position);

    // 设置建筑升级回调函数
    building->setOnUpgradeCallback([=]()
        {
            // 根据建筑类型处理不同的升级效果
            switch (type)
            {
                case BuildingType::BASE:
                {
                    // 大本营：增加两种资源上限
                    extern int coin_limit, water_limit;
                    coin_limit += 1500;
                    water_limit += 1500;
                    CCLOG("=== GameScene: Town Hall upgraded, coin+%d water+%d ===",
                        coin_limit, water_limit);
                    break;
                }

                case BuildingType::BARRACKS:
                {
                    // 兵营：重新计算军队上限
                    this->recalculateArmyLimit();
                    CCLOG("=== GameScene: Barracks upgraded, army limit recalculated ===");
                    break;
                }

                case BuildingType::GOLD_STORAGE:
                {
                    // 金币储存器：增加金币上限
                    extern int coin_limit;
                    coin_limit += 1500 * building->getLevel(); // 当前等级的总容量
                    CCLOG("=== GameScene: Gold Storage upgraded, coin limit +%d ===",
                        1500 * building->getLevel());
                    break;
                }

                case BuildingType::WATER_STORAGE:
                {
                    // 圣水储存器：增加圣水上限
                    extern int water_limit;
                    water_limit += 1500 * building->getLevel(); // 当前等级的总容量
                    CCLOG("=== GameScene: Water Storage upgraded, water limit +%d ===",
                        1500 * building->getLevel());
                    break;
                }

                case BuildingType::MINE:
                case BuildingType::WATER:
                {
                    // 生产建筑：升级时不需要特殊处理，生产量在 Building 类中已更新
                    CCLOG("=== GameScene: Production building upgraded ===");
                    break;
                }
            }

            // 更新UI显示
            this->updateResourceDisplay();
        });

    // 4. 将建筑添加到障碍物列表
    this->addObstacle(building);
    // 将建筑添加到地图上，层级为1
    tiled_map_->addChild(building, 1);
    // 将建筑添加到当前场景的建筑容器
    all_buildings_.pushBack(building);

    // 如果是兵营建筑，重新计算军队上限
    if (type == BuildingType::BARRACKS)
    {
        this->recalculateArmyLimit();
    }

    // 【新增】如果是储存器，初始化资源上限
    if (type == BuildingType::GOLD_STORAGE)
    {
        extern int coin_limit;
        coin_limit += 1500; // 初始1级增加1500
        CCLOG("=== GameScene: Gold Storage created, initial coin limit +1500 ===");
    }
    else if (type == BuildingType::WATER_STORAGE)
    {
        extern int water_limit;
        water_limit += 1500; // 初始1级增加1500
        CCLOG("=== GameScene: Water Storage created, initial water limit +1500 ===");
    }

    // 如果建筑不在全局建筑容器中，则添加到全局容器
    if (!g_allPurchasedBuildings.contains(building))
    {
        // 增加引用计数
        building->retain();
        // 添加到全局建筑容器
        g_allPurchasedBuildings.pushBack(building);
        // 释放引用计数
        building->release();

        CCLOG("=== GameScene: Default building added to global container: %s (type: %d) ===",
            name.c_str(), (int)type);
    }
}

// 检查并限制地图位置函数
void GameScene::checkAndClampMapPosition()
{
    // 获取屏幕可见区域大小
    const auto visible_size = Director::getInstance()->getVisibleSize();

    // 获取地图当前缩放后的实际尺寸
    const float map_width = tiled_map_->getContentSize().width * tiled_map_->getScale();
    const float map_height = tiled_map_->getContentSize().height * tiled_map_->getScale();

    // 获取地图当前位置
    Vec2 current_pos = tiled_map_->getPosition();

    // ==========================================
    // 边界算法：确保 X 和 Y 坐标在合理范围内
    // ==========================================

    // 原则：
    // 1. 如果地图比屏幕小(缩放后小)，则居中显示。
    // 2. 如果地图比屏幕大(缩放后大)，则位置 x 限定在 [minX, 0] 之间。
    //    如果 x > 0，左边就会漏出黑边。
    //    如果 x < minX，右边就会漏出黑边。

    // --- X 坐标边界 ---
    float minX = visible_size.width - map_width; // 左边界
    float maxX = 0;

    if (map_width < visible_size.width)
    {
        // 如果地图比屏幕窄，直接居中
        current_pos.x = (visible_size.width - map_width) / 2;
    }
    else
    {
        // 限定在 [minX, 0] 之间
        if (current_pos.x > maxX)
        {
            current_pos.x = maxX;
        }
        if (current_pos.x < minX)
        {
            current_pos.x = minX;
        }
    }

    // --- Y 坐标边界 ---
    float minY = visible_size.height - map_height; // 下边界
    float maxY = 0;

    if (map_height < visible_size.height)
    {
        // 如果地图比屏幕矮，直接居中
        current_pos.y = (visible_size.height - map_height) / 2;
    }
    else
    {
        // 限定在 [minY, 0] 之间
        if (current_pos.y > maxY)
        {
            current_pos.y = maxY;
        }
        if (current_pos.y < minY)
        {
            current_pos.y = minY;
        }
    }

    // 应用修正后的位置
    tiled_map_->setPosition(current_pos);
}

// 鼠标滚轮滚动事件处理函数
void GameScene::onMouseScroll(Event* event)
{
    // 将事件转换为鼠标事件
    EventMouse* e = static_cast<EventMouse*>(event);
    // 获取滚轮Y轴滚动值
    float scrollY = e->getScrollY();

    // 1. 计算新的缩放值
    float factor = 0.1f;
    float newScale = tiled_map_->getScale() + (scrollY > 0 ? factor : -factor);

    // 获取屏幕可见区域大小和地图原始尺寸
    const auto visible_size = Director::getInstance()->getVisibleSize();
    const Size map_size = tiled_map_->getContentSize();

    // 计算地图完全适应屏幕的最小缩放比例
    float scale_x = visible_size.width / map_size.width;
    float scale_y = visible_size.height / map_size.height;
    float minLimit = std::min(scale_x, scale_y); // 这是"全图显示"的最小值

    // 限制缩放范围
    if (newScale < minLimit)
    {
        newScale = minLimit; // 不能比全图显示更小
    }
    if (newScale > 2.0f)
    {
        newScale = 2.0f;     // 最大放大2倍
    }

    // 应用新的缩放比例
    tiled_map_->setScale(newScale);

    // 修正位置（保证缩放时中心点不变）
    checkAndClampMapPosition();
}

// 触摸开始事件处理函数
bool GameScene::onTouchBegan(Touch* touch, Event* event)
{
    // 记录开始状态，第一次触摸可能是拖动地图，也可能是点击建筑
    is_map_dragging_ = false;
    return true; // 必须返回 true 才能接收到后续的 Moved 和 Ended 事件
}

// 触摸移动事件处理函数
void GameScene::onTouchMoved(Touch* touch, Event* event)
{
    // 1. 获取移动的距离 (Delta)
    Vec2 delta = touch->getDelta();

    // 2. 如果移动距离极小且不是拖动模式，则认为是误触
    if (delta.getLength() < 2.0f && !is_map_dragging_)
    {
        return;
    }

    // 3. 确认为地图拖动模式
    is_map_dragging_ = true;

    // 4. 计算新的地图位置：当前位置 + 偏移量
    Vec2 new_pos = tiled_map_->getPosition() + delta;
    tiled_map_->setPosition(new_pos);

    // 5. 实时修正地图位置，防止超出边界
    checkAndClampMapPosition();
}

// 触摸结束事件处理函数
void GameScene::onTouchEnded(Touch* touch, Event* event)
{
    // 触摸结束，重置拖动标志
    is_map_dragging_ = false;
}

// 返回按钮回调函数
void GameScene::menuBackCallback(Ref* p_sender)
{
    log("Back button clicked! Syncing to cloud before exit...");
    // 声明外部全局变量
    extern std::string g_currentUsername;

    // 如果当前场景所有者是当前用户，执行本地保存
    if (this->current_scene_owner_ == g_currentUsername)
    {
        SaveGame::getInstance()->saveGameState();
        log("Local save executed.");
    }

    // 2. [新增] 检查是否为离线模式
    // 如果是我们在 HelloWorld 里设置的 "LocalPlayer"，直接退出，不联网
    if (g_currentUsername == "LocalPlayer")
    {
        log("Offline Mode: Skipping cloud sync.");

        // 清理全局建筑容器内存
        g_allPurchasedBuildings.clear();
        // 创建主菜单场景
        auto scene = HelloWorld::createScene();
        // 切换回主菜单场景
        Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
        return;
    }

    // --- 权限检查 (防止访问别人家时点返回把别人家的数据存到自己名下) ---
    extern std::string g_currentUsername;
    if (this->current_scene_owner_ != g_currentUsername)
    {
        log("Visiting mode: Skip cloud save, just return.");
        // 如果是访问模式，直接清空并返回，不触发保存
        g_allPurchasedBuildings.clear();
        auto scene = HelloWorld::createScene();
        Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
        return;
    }

    // 1. 获取当前游戏状态为JSON字符串
    std::string current_data = SaveGame::getInstance()->getGameStateAsJsonString();

    // 2. 准备网络请求
    auto request = new (std::nothrow) cocos2d::network::HttpRequest();
    // 设置服务器URL
    request->setUrl("http://100.80.248.229:5000/save");
    // 设置请求类型为POST
    request->setRequestType(cocos2d::network::HttpRequest::Type::POST);

    // 设置请求头，告诉服务器我们要发JSON
    std::vector<std::string> headers;
    headers.push_back("Content-Type: application/json; charset=utf-8");
    request->setHeaders(headers);

    // 3. 构造发送给服务器的完整包 (包含用户名和存档数据)
    extern std::string g_currentUsername;

    // 构造一个合法的JSON字符串
    std::string post_data = "{\"username\":\"" + g_currentUsername + "\", \"gameData\":" + current_data + "}";
    request->setRequestData(post_data.c_str(), post_data.length());

    // 4. 设置回调：无论成功失败，最后都要切换场景
    request->setResponseCallback([=](cocos2d::network::HttpClient* client, cocos2d::network::HttpResponse* response)
        {
            if (response && response->isSucceed())
            {
                log("Cloud Save Success!");
            }
            else
            {
                log("Cloud Save Failed!");
            }

            // 【关键修复点】：在离开场景前，必须彻底清空全局容器
            // 否则下次登录或切换回来时，旧的指针还在vector里，会导致建筑重叠或报错
            g_allPurchasedBuildings.clear();

            // 切换场景
            auto scene = HelloWorld::createScene();
            Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
        });

    // 发送HTTP请求
    cocos2d::network::HttpClient::getInstance()->send(request);
    // 释放请求对象
    request->release();
}

// 添加商店按钮函数
void GameScene::addShopButton()
{
    // --- 核心修改：如果是访客模式，直接返回，不创建商店按钮 ---
    if (current_scene_owner_ != g_currentUsername)
    {
        return;
    }

    const auto visible_size = Director::getInstance()->getVisibleSize();
    const Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 1. 创建商店按钮精灵（正常状态和选中状态）
    auto shop_normal = Sprite::create("ui/shop_normal.png");
    auto shop_selected = Sprite::create("ui/shop_selected.png");

    // 2. 容错处理：如果图片资源丢失，创建备用的彩色矩形按钮
    if (!shop_normal)
    {
        shop_normal = Sprite::create();
        shop_normal->setTextureRect(Rect(0, 0, 150, 80));
        shop_normal->setColor(Color3B(255, 200, 0));

        auto shop_label = Label::createWithTTF("MARKET", "fonts/Marker Felt.ttf", 30);
        shop_label->setPosition(Vec2(75, 40));
        shop_label->setColor(Color3B::WHITE);
        shop_normal->addChild(shop_label);
    }

    if (!shop_selected)
    {
        shop_selected = Sprite::create();
        shop_selected->setTextureRect(Rect(0, 0, 150, 80)); // 建议大小与Normal保持一致
        shop_selected->setColor(Color3B(255, 150, 0));
    }

    // 3. 创建菜单项
    auto shop_item = MenuItemSprite::create(shop_normal, shop_selected,
        [](Ref* p_sender)
        {
            auto scene = ShopScene::createScene();
            Director::getInstance()->pushScene(TransitionFade::create(0.5f, scene));
        }
    );

    // 4. 设置按钮位置 (左上角)
    float x = origin.x + 150;
    float y = origin.y + visible_size.height - 100;
    shop_item->setPosition(Vec2(x, y));

    // 5. 将按钮加入菜单并添加到层中
    auto shop_menu = Menu::create(shop_item, NULL);
    shop_menu->setPosition(Vec2::ZERO);
    this->addChild(shop_menu, 200);
}

// 添加所有已购买建筑函数
void GameScene::addAllPurchasedBuildings()
{
    const auto visible_size = Director::getInstance()->getVisibleSize();
    const Size map_size = tiled_map_->getContentSize();

    // 遍历全局建筑容器中的所有建筑
    for (auto& building : g_allPurchasedBuildings)
    {
        if (building && !building->getParent())
        {
            // 将建筑添加到地图上，层级为10
            tiled_map_->addChild(building, 10);

            // 如果建筑位置为(0,0)，则自动寻找最近空闲位置
            if (building->getPositionX() == 0 && building->getPositionY() == 0)
            {
                Vec2 mapCenter = Vec2(map_size.width / 2, map_size.height / 2);

                // 从地图中心开始寻找最近空闲位置
                Vec2 pos = getNearestFreePosition(building, mapCenter);

                building->setPosition(pos);
            }

            // 设置建筑升级回调
            building->setOnUpgradeCallback([=]()
                {
                    // 如果是兵营建筑，重新计算军队上限
                    if (building->getType() == BuildingType::BARRACKS)
                    {
                        this->recalculateArmyLimit();
                    }

                    // 更新资源显示
                    if (coin_text_label_)
                    {
                        std::string txt = "Coin " + std::to_string(coin_count) + "/" + std::to_string(coin_limit);
                        coin_text_label_->setString(txt);
                    }
                    if (water_text_label_)
                    {
                        std::string txt = "Water " + std::to_string(water_count) + "/" + std::to_string(water_limit);
                        water_text_label_->setString(txt);
                    }
                    if (gem_text_label_)
                    {
                        std::string txt = "Gem " + std::to_string(gem_count) + "/" + std::to_string(gem_limit);
                        gem_text_label_->setString(txt);
                    }

                    // 更新资源显示
                    this->updateResourceDisplay();
                });

            // 将建筑添加到当前场景的建筑容器
            all_buildings_.pushBack(building);
        }
    }

    // 遍历完所有建筑后重新计算军队上限
    this->recalculateArmyLimit();
}

// 进入场景回调函数
void GameScene::onEnter()
{
    // 调用父类的onEnter方法
    cocos2d::Scene::onEnter();

    // 声明外部全局变量
    extern std::string g_currentUsername;
    // 停止所有背景音乐
    AudioEngine::stopAll();

    // 进入场景前，清空当前场景的逻辑容器，防止重复添加
    all_buildings_.clear();

    // 【核心渲染逻辑】
    for (auto& building : g_allPurchasedBuildings)
    {
        if (building)
        {
            // 强行断开旧父节点。
            // 只有先执行这个，addChild到新地图才不会报错
            if (building->getParent())
            {
                building->removeFromParent();
            }

            // 添加到当前地图，层级为10
            tiled_map_->addChild(building, 10);
            // 启动建筑的更新调度
            building->scheduleUpdate();

            // 只有真正坐标为0的新建筑才自动找位置
            // 已经保存过坐标的老建筑不要去动它的Position
            if (building->getPositionX() == 0 && building->getPositionY() == 0)
            {
                const Size map_size = tiled_map_->getContentSize();
                Vec2 center = Vec2(map_size.width / 2, map_size.height / 2);
                Vec2 pos = getNearestFreePosition(building, center);
                building->setPosition(pos);
            }

            // 重新绑定碰撞逻辑（障碍物列表）
            this->addObstacle(building);

            // 重新绑定回调（确保UI刷新逻辑指向当前最新的场景实例）
            building->setOnUpgradeCallback([=]()
                {
                    // 如果是兵营建筑，重新计算军队上限
                    if (building->getType() == BuildingType::BARRACKS)
                    {
                        this->recalculateArmyLimit();
                    }
                    // 更新资源显示
                    this->updateResourceDisplay();
                });

            // 加入当前场景的活跃建筑容器
            all_buildings_.pushBack(building);
        }
    }

    // 刷新显示
    this->recalculateArmyLimit();
    this->updateResourceDisplay();
    // 播放背景音乐
    AudioEngine::play2d("music/1.ogg", true, 0.5f);
}

// 更新资源显示函数
void GameScene::updateResourceDisplay()
{
    const auto visible_size = Director::getInstance()->getVisibleSize();
    const Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 更新金币显示
    if (coin_text_label_)
    {
        std::string txt = "Coin " + std::to_string(coin_count) + "/" + std::to_string(coin_limit);
        coin_text_label_->setString(txt);
    }
    if (my_coin_)
    {
        my_coin_->refresh();
    }

    // 更新圣水显示
    if (water_text_label_)
    {
        std::string txt = "Water " + std::to_string(water_count) + "/" + std::to_string(water_limit);
        water_text_label_->setString(txt);
    }
    if (my_water_)
    {
        my_water_->refresh();
    }

    // 更新宝石显示
    if (gem_text_label_)
    {
        std::string txt = "Gem " + std::to_string(gem_count) + "/" + std::to_string(gem_limit);
        gem_text_label_->setString(txt);
    }
    if (my_gem_)
    {
        my_gem_->refresh();
    }
}

// 移除障碍物函数
void GameScene::removeObstacle(Node* node)
{
    // 如果障碍物列表包含该节点，则移除
    if (obstacles_.contains(node))
    {
        obstacles_.eraseObject(node);
    }
}

// 添加障碍物函数
void GameScene::addObstacle(Node* node)
{
    // 如果节点有效且不在障碍物列表中，则添加
    if (node && !obstacles_.contains(node))
    {
        obstacles_.pushBack(node);
    }
}

// 获取节点在世界坐标系中的边界框
Rect GameScene::getWorldBoundingBox(Node* node) const
{
    // 如果节点没有父节点，返回空矩形
    if (!node->getParent())
    {
        return Rect::ZERO;
    }

    // 获取节点在本地坐标系中的边界框
    Rect rect = node->getBoundingBox();
    // 将本地坐标系的原点转换为世界坐标系
    Vec2 worldOrigin = node->getParent()->convertToWorldSpace(rect.origin);
    // 创建世界坐标系中的矩形
    Rect worldRect = Rect(worldOrigin.x, worldOrigin.y, rect.size.width, rect.size.height);
    return worldRect;
}

// 检查碰撞函数
bool GameScene::checkCollision(const Rect& target_rect, Node* ignore_node) const
{
    // 遍历所有障碍物
    for (auto obstacle : obstacles_)
    {
        // 如果是忽略的节点，则跳过
        if (obstacle == ignore_node)
        {
            continue;
        }

        // 如果障碍物没有父节点，则跳过
        if (!obstacle->getParent())
        {
            continue;
        }

        // 获取障碍物的边界框
        Rect obstacleRect = obstacle->getBoundingBox();

        // 检查目标矩形与障碍物矩形是否相交
        if (target_rect.intersectsRect(obstacleRect))
        {
            return true; // 发生碰撞
        }
    }
    return false; // 没有碰撞
}

// 获取最近空闲位置函数
Vec2 GameScene::getNearestFreePosition(Building* building, Vec2 target_map_pos) const
{
    // 1. 获取建筑的实际尺寸
    Size size = building->getContentSize();
    float w = size.width * building->getScaleX();
    float h = size.height * building->getScaleY();

    // 2. 设置搜索步长
    int step = (int)(w / 2);
    if (step < 10)
    {
        step = 10;
    }

    // 设置最大搜索半径
    int max_radius = 30;

    // 从内向外螺旋搜索空闲位置
    for (int r = 1; r <= max_radius; r++)
    {
        for (int x = -r; x <= r; x++)
        {
            for (int y = -r; y <= r; y++)
            {
                // 只搜索当前半径的外围点
                if (std::abs(x) != r && std::abs(y) != r)
                {
                    continue;
                }

                // 计算候选位置
                Vec2 candidate_map_pos = target_map_pos + Vec2(x * step, y * step);

                // ====================================================
                // 需要转WorldSpace，直接用Local点！
                // ====================================================

                // A. 将candidateMapPos转换为Local坐标
                // B. w和h也是Local尺寸(Building的原始大小)
                Rect test_local_rect = Rect(
                    candidate_map_pos.x - w * 0.5f,
                    candidate_map_pos.y - h * 0.5f,
                    w, h
                );

                // C. 直接传入Local Rect进行碰撞检测
                if (!checkCollision(test_local_rect, nullptr))
                {
                    return candidate_map_pos; // 找到空闲位置
                }
            }
        }
    }

    // 没有找到空闲位置，返回零向量
    return Vec2::ZERO;
}

// 添加保存按钮函数
void GameScene::addSaveButton()
{
    if (g_currentUsername != "LocalPlayer")
	{
			return;
	}
    const auto visible_size = Director::getInstance()->getVisibleSize();
    const Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 创建保存按钮标签
    auto save_label = Label::createWithTTF("SAVE GAME \n(Save to local device)", "fonts/Marker Felt.ttf", 28);
    save_label->setColor(Color3B::GREEN);
    save_label->enableOutline(Color4B::BLACK, 2);

    // 创建保存按钮菜单项
    auto save_item = MenuItemLabel::create(save_label,
        CC_CALLBACK_1(GameScene::menuSaveGameCallback, this));

    // 设置按钮位置（右下角）
    float x = origin.x + visible_size.width - 150;
    float y = origin.y + 100;
    save_item->setPosition(Vec2(x, y));

    // 创建调试按钮标签
    auto debug_label = Label::createWithTTF("DEBUG SAVE", "fonts/Marker Felt.ttf", 20);
    debug_label->setColor(Color3B::YELLOW);

    // 创建调试按钮菜单项
    auto debug_item = MenuItemLabel::create(debug_label,
        [](Ref* p_sender)
        {
            CCLOG("=== Debug Save System ===");
            // 调试可写路径
            SaveGame::getInstance()->debugWritablePath();

            // 测试保存和加载功能
            auto saveGame = SaveGame::getInstance();
            bool saveResult = saveGame->saveGameState("test_save.json");
            CCLOG("Test save result: %s", saveResult ? "SUCCESS" : "FAILED");

            bool loadResult = saveGame->loadGameState("test_save.json");
            CCLOG("Test load result: %s", loadResult ? "SUCCESS" : "FAILED");
        });

    // 创建菜单并添加按钮
    auto save_menu = Menu::create(save_item, NULL);
    save_menu->setPosition(Vec2::ZERO);
    this->addChild(save_menu, 200);
}

// 保存游戏回调函数
void GameScene::menuSaveGameCallback(Ref* p_sender)
{
    log("Saving game...");

    // 调用SaveGame单例保存游戏状态
    SaveGame::getInstance()->saveGameState();
}

// 计算军队上限函数
int GameScene::calculateArmyLimit() const
{
    int total_limit = 0; // 总人口上限

    // 遍历所有已购买建筑，找到兵营并计算人口上限
    for (auto& building : g_allPurchasedBuildings)
    {
        if (building && building->getType() == BuildingType::BARRACKS)
        {
            // 每个兵营提供的基础人口 + 每个等级增加的人口
            // 例如：1级兵营提供10人口，每升一级加10人口
            total_limit += 10 * building->getLevel();
        }
    }

    CCLOG("=== GameScene: Calculated army limit: %d ===", total_limit);
    return total_limit;
}

// 重新计算军队上限函数
void GameScene::recalculateArmyLimit()
{
    // 更新全局军队上限变量
    army_limit = calculateArmyLimit();
    CCLOG("=== GameScene: Army limit recalculated to: %d ===", army_limit);

    // 更新UI显示（如果需要）
    this->updateResourceDisplay();
}

// 显示Toast消息函数
void GameScene::showToast(const std::string& message, const Color3B& color)
{
    const auto visible_size = Director::getInstance()->getVisibleSize();
    // 创建Toast消息标签
    auto label = Label::createWithTTF(message, "fonts/Marker Felt.ttf", 30);
    label->setColor(color);
    label->enableOutline(Color4B::BLACK, 2);
    label->setPosition(visible_size.width / 2, visible_size.height / 2);
    // 将标签添加到场景最上层
    this->addChild(label, 1000);

    // 动画序列：淡入 -> 停留 -> 上浮淡出 -> 移除
    label->runAction(Sequence::create(
        FadeIn::create(0.2f),
        DelayTime::create(1.0f),
        Spawn::create(MoveBy::create(0.5f, Vec2(0, 50)), FadeOut::create(0.5f), nullptr),
        RemoveSelf::create(),
        nullptr
    ));
}

// 手动保存点击回调函数
void GameScene::onManualSaveClicked()
{
    // 只有在自己的基地时才允许存盘
    extern std::string g_currentUsername;
    if (this->current_scene_owner_ != g_currentUsername)
    {
        this->showToast("Cannot save while visiting!", Color3B::RED);
        return;
    }

    // 调用SaveGame的同步逻辑，并传入Lambda回调来显示UI反馈
    SaveGame::getInstance()->syncDataToCloud([this](bool success)
        {
            if (success)
            {
                this->showToast("Save Success!", Color3B::GREEN);
            }
            else
            {
                this->showToast("Save Failed!", Color3B::RED);
            }
        });
}

// 自动保存回调函数
void GameScene::onAutoSave(float dt)
{
    log("Auto-saving...");
    // 调用SaveGame的同步逻辑
    SaveGame::getInstance()->syncDataToCloud([this](bool success)
        {
            if (success)
            {
                log("Auto save success.");
            }
        });
}

// 设置左侧面板函数
void GameScene::setupLeftPanel()
{
    const auto visible_size = Director::getInstance()->getVisibleSize();
    const Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 1. 创建左侧背景条
    auto panel = LayerColor::create(Color4B(0, 0, 0, 150), 120, visible_size.height - 100);
    panel->setPosition(origin.x, origin.y + 50);
    this->addChild(panel, 100);

    // 2. 标题
    auto title = Label::createWithTTF("Neighbors", "fonts/Marker Felt.ttf", 20);
    title->setPosition(60, panel->getContentSize().height - 20);
    panel->addChild(title);

    // 3. 从服务器获取好友列表
    auto request = new HttpRequest();
    request->setUrl("http://100.80.248.229:5000/users");
    request->setRequestType(HttpRequest::Type::GET);
    request->setResponseCallback([=](HttpClient* client, HttpResponse* response)
        {
            if (!response || !response->isSucceed())
            {
                return;
            }

            std::vector<char>* buffer = response->getResponseData();
            std::string resultStr(buffer->begin(), buffer->end());

            rapidjson::Document doc;
            doc.Parse(resultStr.c_str());

            if (doc.HasMember("users") && doc["users"].IsArray())
            {
                auto users = doc["users"].GetArray();

                // 创建菜单列表
                auto menu = Menu::create();
                float start_y = panel->getContentSize().height - 60;

                // --- 新增：在列表最上方添加"回到自己家"的按钮 ---
                extern std::string g_currentUsername;
                auto home_label = Label::createWithTTF("MY HOME", "fonts/Marker Felt.ttf", 20);
                home_label->setColor(Color3B::YELLOW); // 用黄色区分
                auto home_item = MenuItemLabel::create(home_label, [=](Ref* sender)
                    {
                        log("Returning to own home: %s", g_currentUsername.c_str());
                        this->loadOtherPlayerData(g_currentUsername); // 加载自己的数据
                    });
                home_item->setPosition(60, start_y);
                menu->addChild(home_item);
                // ------------------------------------------

                // 遍历用户列表，为每个用户创建菜单项
                for (int i = 0; i < users.Size(); i++)
                {
                    std::string name = users[i].GetString();
                    // 跳过自己
                    if (name == g_currentUsername)
                    {
                        continue;
                    }

                    auto label = Label::createWithTTF(name, "fonts/Marker Felt.ttf", 18);
                    auto item = MenuItemLabel::create(label, [=](Ref* sender)
                        {
                            this->loadOtherPlayerData(name);
                        });
                    // i + 1 是为了给上面的My Home留出位置
                    item->setPosition(60, start_y - ((i + 1) * 30));
                    menu->addChild(item);
                }
                menu->setPosition(Vec2::ZERO);
                panel->addChild(menu);
            }
        });
    // 发送HTTP请求
    HttpClient::getInstance()->send(request);
    // 释放请求对象
    request->release();
}

// 加载其他玩家数据函数
void GameScene::loadOtherPlayerData(std::string target_username)
{
    // 创建HTTP请求获取目标用户数据
    auto request = new HttpRequest();
    request->setUrl("http://100.80.248.229:5000/get_user_data");
    request->setRequestType(HttpRequest::Type::POST);

    // 构造POST数据
    std::string post_data = "{\"username\":\"" + target_username + "\"}";
    request->setRequestData(post_data.c_str(), post_data.length());
    request->setHeaders({ "Content-Type: application/json" });

    // 设置响应回调
    request->setResponseCallback([=](HttpClient* client, HttpResponse* response)
        {
            if (response && response->isSucceed())
            {
                std::vector<char>* buffer = response->getResponseData();
                std::string res(buffer->begin(), buffer->end());
                rapidjson::Document doc;
                doc.Parse(res.c_str());

                if (doc.HasMember("data") && doc["data"].IsString())
                {
                    std::string saveData = doc["data"].GetString();

                    extern std::string g_currentUsername;
                    bool isMe = (target_username == g_currentUsername);

                    // 直接跳转到BattleScene，把好友的JSON传过去
                    auto scene = BattleScene::createScene(0, saveData);
                    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));

                    log("Switching to BattleScene to view %s's base", target_username.c_str());
                }
            }
        });
    // 发送HTTP请求
    HttpClient::getInstance()->send(request);
}

// 攻击按钮回调函数
void GameScene::menuAttackCallback(Ref* p_sender)
{
    log("Attacking player: %s", current_scene_owner_.c_str());

    // 1. 获取当前好友场景的建筑数据 (转为JSON字符串)
    std::string pvp_data = SaveGame::getInstance()->getGameStateAsJsonString();

    // 关键修复：检查数据是否有效，防止传给BattleScene一个空的JSON
    if (pvp_data.empty() || pvp_data == "{\"buildings\":[]}")
    {
        log("Error: Target house has no buildings to attack!");
        return;
    }

    // 2. 停止背景音乐并切换到战斗场景
    AudioEngine::stopAll();

    // 传入levelIndex为0表示PVP模式，并传入好友的建筑数据
    auto scene = BattleScene::createScene(0, pvp_data);
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
}