//商店场景
#include "ShopScene.h"
#include "GameScene.h"
#include "Building.h"
#include "DataManager.h"
#include "HelloWorldScene.h"
#include"AudioEngine.h"
USING_NS_CC;

// 外部变量声明（全局资源变量）
extern int coin_count;        // 当前金币数量
extern int water_count;       // 当前圣水数量
extern int gem_count;         // 当前宝石数量
extern int coin_limit;        // 金币存储上限
extern int water_limit;       // 圣水存储上限
extern int gem_limit;         // 宝石存储上限

// 外部变量声明（已购买建筑列表）
extern cocos2d::Vector<Building*> g_allPurchasedBuildings;

// 静态常量初始化
const cocos2d::Color3B ShopScene::LOCKED_COLOR = cocos2d::Color3B(100, 100, 100);    // 锁定状态颜色（灰色）
const cocos2d::Color3B ShopScene::SELECTED_COLOR = cocos2d::Color3B(200, 200, 200);  // 选中状态颜色（亮灰色）

// 创建场景静态方法
Scene* ShopScene::createScene()
{
    return ShopScene::create();
}

// 进入场景回调函数
void ShopScene::onEnter()
{
    // 1. 首先调用父类的onEnter方法，确保场景正确初始化
    Scene::onEnter();

    // 2. 停止之前播放的所有音乐/音效
    AudioEngine::stopAll();

    // 3. 输出日志信息
    log("Enter ShopScene: Music Stopped.");
}

// 场景初始化方法
bool ShopScene::init()
{
    // 1. 调用父类初始化方法
    if (!Scene::init())
    {
        return false;
    }

    // 2. 获取可见区域大小和原点坐标
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 3. 创建并设置背景精灵
    auto bgSprite = Sprite::create("popup_bg.png");
    if (bgSprite)
    {
        // 获取图片原始尺寸
        Size bgSize = bgSprite->getContentSize();

        // 计算缩放比例
        float scaleX = static_cast<float>(visibleSize.width) / static_cast<float>(bgSize.width);
        float scaleY = static_cast<float>(visibleSize.height) / static_cast<float>(bgSize.height);

        // 设置缩放，使背景填满屏幕
        bgSprite->setScaleX(scaleX);
        bgSprite->setScaleY(scaleY);

        // 设置位置为屏幕中心
        bgSprite->setPosition(Vec2(origin.x + visibleSize.width / 2,
            origin.y + visibleSize.height / 2));

        // 添加到场景底部
        this->addChild(bgSprite, -1);
    }
    else
    {
        // 如果背景图片加载失败，使用灰色图层作为后备
        CCLOG("ERROR: Failed to load popup_bg.png");
        auto fallbackBg = LayerColor::create(Color4B(100, 100, 100, 255));
        fallbackBg->setContentSize(visibleSize);
        fallbackBg->setPosition(origin);
        this->addChild(fallbackBg, -1);
    }

    // 4. 添加商店标题
    auto title = Label::createWithTTF("MARKET", "fonts/Marker Felt.ttf", 48);
    title->setPosition(Vec2(origin.x + visibleSize.width / 2,
        origin.y + visibleSize.height - 100));
    title->setColor(Color3B::YELLOW);
    this->addChild(title, 1);

    // 5. 创建商品菜单
    createMenu();

    // 6. 更新资源显示标签
    updateResourceLabels();

    // 7. 创建返回按钮
    auto backLabel = Label::createWithTTF("Back", "fonts/Marker Felt.ttf", 36);
    backLabel->setTextColor(Color4B::WHITE);

    auto backItem = MenuItemLabel::create(backLabel,
        CC_CALLBACK_1(ShopScene::menuBackCallback, this));
    backItem->setPosition(Vec2(origin.x + visibleSize.width - 100,
        origin.y + 50));

    auto menu = Menu::create(backItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 2);

    return true;
}

// 城墙购买回调函数
void ShopScene::onWallPurchase(Ref* p_sender)
{
    purchaseItem(ShopItemType::WALL);
}

// 金矿购买回调函数
void ShopScene::onGoldMinePurchase(Ref* p_sender)
{
    purchaseItem(ShopItemType::GOLD_MINE);
}

// 圣水收集器购买回调函数
void ShopScene::onWaterCollectorPurchase(Ref* p_sender)
{
    purchaseItem(ShopItemType::WATER_COLLECTOR);
}

// 箭塔购买回调函数
void ShopScene::onArcherTowerPurchase(Ref* p_sender)
{
    purchaseItem(ShopItemType::ARCHER_TOWER);
}

// 加农炮购买回调函数
void ShopScene::onCannonPurchase(Ref* p_sender)
{
    purchaseItem(ShopItemType::CANNON);
}

// 兵营购买回调函数
void ShopScene::onBarracksPurchase(Ref* p_sender)
{
    purchaseItem(ShopItemType::BARRACKS);
}

// 金币存储器购买回调函数
void ShopScene::onGoldStoragePurchase(Ref* p_sender)
{
    purchaseItem(ShopItemType::GOLD_STORAGE);
}

// 圣水存储器购买回调函数
void ShopScene::onWaterStoragePurchase(Ref* p_sender)
{
    purchaseItem(ShopItemType::WATER_STORAGE);
}

// 检查是否有足够资源
bool ShopScene::hasEnoughResources(int coin_cost, int water_cost, int gem_cost) const
{
    return coin_count >= coin_cost && water_count >= water_cost && gem_count >= gem_cost;
}

// 显示购买消息（成功/失败）
void ShopScene::showPurchaseMessage(bool success, const std::string& item_name)
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    std::string message;
    Color4B color;

    // 根据购买结果设置消息内容和颜色
    if (success)
    {
        message = "Successful purchase " + item_name + "!";
        color = Color4B::GREEN;
    }
    else
    {
        message = "Insufficient resources, unable to purchase " + item_name + "!";
        color = Color4B::RED;
    }

    // 移除之前显示的消息标签（如果存在）
    if (current_message_label_)
    {
        current_message_label_->stopAllActions();
        current_message_label_->removeFromParent();
        current_message_label_ = nullptr;
    }

    // 创建新的消息标签
    auto label = Label::createWithTTF(message, "fonts/Marker Felt.ttf", 32);
    label->setPosition(Vec2(origin.x + visibleSize.width / 2,
        origin.y + visibleSize.height * 0.1f));
    label->setTextColor(color);
    this->addChild(label, 3);

    // 保存当前消息标签的指针
    current_message_label_ = label;

    // 设置3秒后自动移除消息标签
    label->runAction(Sequence::create(
        DelayTime::create(3.0f),
        CallFunc::create([this, label]()
            {
                if (current_message_label_ == label)
                {
                    label->removeFromParent();
                    current_message_label_ = nullptr;
                }
            }),
        nullptr
    ));
}

// 更新资源显示标签
void ShopScene::updateResourceLabels()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 移除旧的标签（如果存在）
    if (coin_label_) 
        coin_label_->removeFromParent();
    if (water_label_) 
        water_label_->removeFromParent();
    if (gem_label_) 
        gem_label_->removeFromParent();

    // 创建新的资源显示标签
    coin_label_ = Label::createWithTTF("Coin: " + std::to_string(coin_count) + "/" + std::to_string(coin_limit),
        "fonts/Marker Felt.ttf", 28);
    water_label_ = Label::createWithTTF("Water: " + std::to_string(water_count) + "/" + std::to_string(water_limit),
        "fonts/Marker Felt.ttf", 28);
    gem_label_ = Label::createWithTTF("Gem: " + std::to_string(gem_count),
        "fonts/Marker Felt.ttf", 28);

    // 设置标签位置
    coin_label_->setPosition(Vec2(origin.x + visibleSize.width / 2,
        origin.y + visibleSize.height - 150));
    water_label_->setPosition(Vec2(origin.x + visibleSize.width / 2,
        origin.y + visibleSize.height - 190));
    gem_label_->setPosition(Vec2(origin.x + visibleSize.width / 2,
        origin.y + visibleSize.height - 230));

    // 设置标签颜色
    coin_label_->setColor(Color3B::YELLOW);
    water_label_->setColor(Color3B::BLUE);
    gem_label_->setColor(Color3B::MAGENTA);

    // 添加到场景
    this->addChild(coin_label_, 2);
    this->addChild(water_label_, 2);
    this->addChild(gem_label_, 2);
}

// 返回按钮回调函数
void ShopScene::menuBackCallback(Ref* p_sender)
{
    Director::getInstance()->popScene();
}

// 创建商店菜单项
cocos2d::MenuItemSprite* ShopScene::createShopMenuItem(
    const std::string& image_path,
    bool is_unlocked,
    const cocos2d::ccMenuCallback& callback)
{
    // 1. 创建精灵（正常状态和选中状态）
    auto normal_sprite = cocos2d::Sprite::create(image_path);
    auto selected_sprite = cocos2d::Sprite::create(image_path);

    if (!normal_sprite || !selected_sprite)
    {
        CCLOG("ERROR: Failed to load image: %s", image_path.c_str());
        return nullptr;
    }

    // 2. 设置缩放
    normal_sprite->setScale(ITEM_SCALE_NORMAL);
    selected_sprite->setScale(ITEM_SCALE_SELECTED);

    // 3. 设置纹理抗锯齿（保持像素风格）
    if (normal_sprite->getTexture())
    {
        normal_sprite->getTexture()->setAliasTexParameters();
    }
    if (selected_sprite->getTexture())
    {
        selected_sprite->getTexture()->setAliasTexParameters();
    }

    // 4. 根据解锁状态设置颜色
    if (!is_unlocked)
    {
        normal_sprite->setColor(LOCKED_COLOR);
        selected_sprite->setColor(LOCKED_COLOR);
    }

    // 5. 设置选中状态的颜色
    selected_sprite->setColor(SELECTED_COLOR);

    // 6. 创建并返回菜单项
    return cocos2d::MenuItemSprite::create(normal_sprite, selected_sprite, callback);
}

// 统计指定类型的建筑数量
int ShopScene::countBuildingsByType(BuildingType type) const
{
    int count = 0;
    for (auto& building : g_allPurchasedBuildings)
    {
        if (building && building->getType() == type)
        {
            count++;
        }
    }
    return count;
}

// 创建建筑信息标签
void ShopScene::createBuildingInfoLabels(cocos2d::MenuItemSprite* itemBtn,
    const ShopItem& info,
    int currentCount,
    int maxCount,
    bool isUnlocked)
{
    if (!itemBtn) return;

    Vec2 btnPos = itemBtn->getPosition();

    // 1. 建筑名称标签
    auto nameLabel = Label::createWithTTF(info.name, "fonts/Marker Felt.ttf", 24);
    nameLabel->setPosition(Vec2(btnPos.x - 50, btnPos.y + 50));
    nameLabel->setColor(Color3B::WHITE);
    nameLabel->setAlignment(TextHAlignment::CENTER);
    this->addChild(nameLabel, 2);

    // 2. 价格标签
    std::string priceText;
    if (info.coin_cost > 0)
        priceText += "Coin:" + std::to_string(info.coin_cost) + " ";
    if (info.water_cost > 0) 
        priceText += "Water:" + std::to_string(info.water_cost) + " ";
    if (info.gem_cost > 0) 
        priceText += "Gem:" + std::to_string(info.gem_cost);

    // 添加数量信息（如果有限制）
    if (maxCount > 0)
    {
        priceText += "\n" + std::to_string(currentCount) + "/" + std::to_string(maxCount);
    }

    auto priceLabel = Label::createWithTTF(priceText, "fonts/Marker Felt.ttf", 18);
    priceLabel->setPosition(Vec2(btnPos.x - 50, btnPos.y - 180));

    // 根据是否解锁或达到上限设置颜色
    if (!isUnlocked || (maxCount > 0 && currentCount >= maxCount))
    {
        priceLabel->setColor(Color3B::GRAY);
    }
    else
    {
        priceLabel->setColor(Color3B::GREEN);
    }

    priceLabel->setAlignment(TextHAlignment::CENTER);
    this->addChild(priceLabel, 2);

    // 3. 描述标签
    auto descLabel = Label::createWithTTF(info.description, "fonts/Marker Felt.ttf", 20);
    descLabel->setPosition(Vec2(btnPos.x - 50, btnPos.y - 210));
    descLabel->setColor(Color3B::YELLOW);
    descLabel->setAlignment(TextHAlignment::CENTER);
    this->addChild(descLabel, 2);
}

// 创建第一行建筑菜单项
std::vector<MenuItemSprite*> ShopScene::createFirstRowMenuItems()
{
    auto dataManager = DataManager::getInstance();
    std::vector<MenuItemSprite*> items;

    // 1. 金矿
    int mineRequiredLevel = 0;
    bool mineUnlocked = dataManager->isBuildingUnlocked(2, mineRequiredLevel);
    auto goldMineItem = createShopMenuItem("Mine.png", mineUnlocked,
        CC_CALLBACK_1(ShopScene::onGoldMinePurchase, this));

    // 2. 圣水收集器
    int waterRequiredLevel = 0;
    bool waterUnlocked = dataManager->isBuildingUnlocked(3, waterRequiredLevel);
    auto waterCollectorItem = createShopMenuItem("waterwell.png", waterUnlocked,
        CC_CALLBACK_1(ShopScene::onWaterCollectorPurchase, this));

    // 3. 围墙
    int wallRequiredLevel = 0;
    bool wallUnlocked = dataManager->isBuildingUnlocked(6, wallRequiredLevel);
    auto wallItem = createShopMenuItem("fence.png", wallUnlocked,
        CC_CALLBACK_1(ShopScene::onWallPurchase, this));

    // 4. 箭塔
    int archerRequiredLevel = 0;
    bool archerUnlocked = dataManager->isBuildingUnlocked(4, archerRequiredLevel);
    auto archerTowerItem = createShopMenuItem("TilesetTowers.png", archerUnlocked,
        CC_CALLBACK_1(ShopScene::onArcherTowerPurchase, this));

    // 添加有效的菜单项到列表
    if (goldMineItem) 
        items.push_back(goldMineItem);
    if (waterCollectorItem)
        items.push_back(waterCollectorItem);
    if (wallItem) 
        items.push_back(wallItem);
    if (archerTowerItem) 
        items.push_back(archerTowerItem);

    return items;
}

// 创建第二行建筑菜单项
std::vector<MenuItemSprite*> ShopScene::createSecondRowMenuItems()
{
    auto dataManager = DataManager::getInstance();
    std::vector<MenuItemSprite*> items;

    // 5. 加农炮
    int cannonRequiredLevel = 0;
    bool cannonUnlocked = dataManager->isBuildingUnlocked(5, cannonRequiredLevel);
    auto cannonItem = createShopMenuItem("Cannon.png", cannonUnlocked,
        CC_CALLBACK_1(ShopScene::onCannonPurchase, this));

    // 6. 兵营
    int barracksRequiredLevel = 0;
    bool barracksUnlocked = dataManager->isBuildingUnlocked(1, barracksRequiredLevel);
    auto barracksItem = createShopMenuItem("junying.png", barracksUnlocked,
        CC_CALLBACK_1(ShopScene::onBarracksPurchase, this));

    // 7. 金币存储器
    int goldStorageRequiredLevel = 0;
    bool goldStorageUnlocked = dataManager->isBuildingUnlocked(8, goldStorageRequiredLevel);
    auto goldStorageItem = createShopMenuItem("BarGold.png", goldStorageUnlocked,
        CC_CALLBACK_1(ShopScene::onGoldStoragePurchase, this));

    // 8. 圣水存储器
    int waterStorageRequiredLevel = 0;
    bool waterStorageUnlocked = dataManager->isBuildingUnlocked(9, waterStorageRequiredLevel);
    auto waterStorageItem = createShopMenuItem("Water.png", waterStorageUnlocked,
        CC_CALLBACK_1(ShopScene::onWaterStoragePurchase, this));

    // 添加有效的菜单项到列表
    if (cannonItem) 
        items.push_back(cannonItem);
    if (barracksItem) 
        items.push_back(barracksItem);
    if (goldStorageItem) 
        items.push_back(goldStorageItem);
    if (waterStorageItem) 
        items.push_back(waterStorageItem);

    return items;
}

// 获取建筑数量信息
void ShopScene::getBuildingCounts(int& mineCount, int& waterCount, int& wallCount,
    int& archerCount, int& cannonCount, int& barracksCount,
    int& goldStorageCount, int& waterStorageCount)
{
    mineCount = countBuildingsByType(BuildingType::MINE);
    waterCount = countBuildingsByType(BuildingType::WATER);
    wallCount = countBuildingsByType(BuildingType::WALL);
    archerCount = countBuildingsByType(BuildingType::TOWER);
    cannonCount = countBuildingsByType(BuildingType::CANNON);
    barracksCount = countBuildingsByType(BuildingType::BARRACKS);
    goldStorageCount = countBuildingsByType(BuildingType::GOLD_STORAGE);
    waterStorageCount = countBuildingsByType(BuildingType::WATER_STORAGE);
}

// 获取建筑最大数量限制
void ShopScene::getBuildingMaxCounts(int& mineMax, int& waterMax, int& wallMax,
    int& archerMax, int& cannonMax, int& barracksMax,
    int& goldStorageMax, int& waterStorageMax)
{
    auto dataManager = DataManager::getInstance();
    mineMax = dataManager->getBuildingMaxCount(2);
    waterMax = dataManager->getBuildingMaxCount(3);
    wallMax = dataManager->getBuildingMaxCount(6);
    archerMax = dataManager->getBuildingMaxCount(4);
    cannonMax = dataManager->getBuildingMaxCount(5);
    barracksMax = dataManager->getBuildingMaxCount(1);
    goldStorageMax = dataManager->getBuildingMaxCount(8);
    waterStorageMax = dataManager->getBuildingMaxCount(9);
}

// 创建第一行建筑信息标签
void ShopScene::createFirstRowInfoLabels(const std::vector<MenuItemSprite*>& items,
    const std::vector<ShopItem>& infos,
    int mineCount, int mineMax, bool mineUnlocked,
    int waterCount, int waterMax, bool waterUnlocked,
    int wallCount, int wallMax, bool wallUnlocked,
    int archerCount, int archerMax, bool archerUnlocked)
{
    for (int i = 0; i < items.size(); ++i)
    {
        MenuItemSprite* itemBtn = items[i];
        const ShopItem& info = infos[i];

        int currentCount = 0;
        int maxCount = 0;
        bool isUnlocked = false;

        // 根据商品类型设置对应的计数和解锁状态
        switch (info.type)
        {
            case ShopItemType::GOLD_MINE:
                currentCount = mineCount;
                maxCount = mineMax;
                isUnlocked = mineUnlocked;
                break;
            case ShopItemType::WATER_COLLECTOR:
                currentCount = waterCount;
                maxCount = waterMax;
                isUnlocked = waterUnlocked;
                break;
            case ShopItemType::WALL:
                currentCount = wallCount;
                maxCount = wallMax;
                isUnlocked = wallUnlocked;
                break;
            case ShopItemType::ARCHER_TOWER:
                currentCount = archerCount;
                maxCount = archerMax;
                isUnlocked = archerUnlocked;
                break;
        }

        // 创建建筑信息标签
        this->createBuildingInfoLabels(itemBtn, info, currentCount, maxCount, isUnlocked);
    }
}

// 创建第二行建筑信息标签
void ShopScene::createSecondRowInfoLabels(const std::vector<MenuItemSprite*>& items,
    const std::vector<ShopItem>& infos,
    int cannonCount, int cannonMax, bool cannonUnlocked,
    int barracksCount, int barracksMax, bool barracksUnlocked,
    int goldStorageCount, int goldStorageMax, bool goldStorageUnlocked,
    int waterStorageCount, int waterStorageMax, bool waterStorageUnlocked)
{
    for (int i = 0; i < items.size(); ++i)
    {
        MenuItemSprite* itemBtn = items[i];
        const ShopItem& info = infos[i];

        int currentCount = 0;
        int maxCount = 0;
        bool isUnlocked = false;

        // 根据商品类型设置对应的计数和解锁状态
        switch (info.type)
        {
            case ShopItemType::CANNON:
                currentCount = cannonCount;
                maxCount = cannonMax;
                isUnlocked = cannonUnlocked;
                break;
            case ShopItemType::BARRACKS:
                currentCount = barracksCount;
                maxCount = barracksMax;
                isUnlocked = barracksUnlocked;
                break;
            case ShopItemType::GOLD_STORAGE:
                currentCount = goldStorageCount;
                maxCount = goldStorageMax;
                isUnlocked = goldStorageUnlocked;
                break;
            case ShopItemType::WATER_STORAGE:
                currentCount = waterStorageCount;
                maxCount = waterStorageMax;
                isUnlocked = waterStorageUnlocked;
                break;
        }

        // 创建建筑信息标签
        this->createBuildingInfoLabels(itemBtn, info, currentCount, maxCount, isUnlocked);
    }
}

// 设置菜单项位置
void ShopScene::setMenuItemsPosition(const std::vector<MenuItemSprite*>& items,
    float startX, float yPos, float spacing)
{
    for (int i = 0; i < items.size(); ++i)
    {
        MenuItemSprite* item = items[i];
        if (item)
        {
            float posX = startX + i * spacing + 60;
            item->setPosition(Vec2(posX, yPos));
        }
    }
}

// 重构后的createMenu函数（主菜单创建函数）
void ShopScene::createMenu()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    auto dataManager = DataManager::getInstance();

    // ================= 获取建筑数量信息 =================
    int mineCount, waterCount, wallCount, archerCount;
    int cannonCount, barracksCount, goldStorageCount, waterStorageCount;
    getBuildingCounts(mineCount, waterCount, wallCount, archerCount,
        cannonCount, barracksCount, goldStorageCount, waterStorageCount);

    // ================= 获取建筑最大数量 =================
    int mineMax, waterMax, wallMax, archerMax;
    int cannonMax, barracksMax, goldStorageMax, waterStorageMax;
    getBuildingMaxCounts(mineMax, waterMax, wallMax, archerMax,
        cannonMax, barracksMax, goldStorageMax, waterStorageMax);

    // ================= 获取解锁状态 =================
    int requiredLevel = 0;
    bool mineUnlocked = dataManager->isBuildingUnlocked(2, requiredLevel);
    bool waterUnlocked = dataManager->isBuildingUnlocked(3, requiredLevel);
    bool wallUnlocked = dataManager->isBuildingUnlocked(6, requiredLevel);
    bool archerUnlocked = dataManager->isBuildingUnlocked(4, requiredLevel);
    bool cannonUnlocked = dataManager->isBuildingUnlocked(5, requiredLevel);
    bool barracksUnlocked = dataManager->isBuildingUnlocked(1, requiredLevel);
    bool goldStorageUnlocked = dataManager->isBuildingUnlocked(8, requiredLevel);
    bool waterStorageUnlocked = dataManager->isBuildingUnlocked(9, requiredLevel);

    // ================= 创建菜单项 =================
    auto firstRowItems = createFirstRowMenuItems();
    auto secondRowItems = createSecondRowMenuItems();

    // ================= 设置菜单位置 =================
    float totalItemsPerRow = 4.0f;
    float horizontalSpacing = visibleSize.width * 0.2f;
    float totalWidth = (totalItemsPerRow - 1) * horizontalSpacing;
    float startX = origin.x + (visibleSize.width - totalWidth) / 2.0f;

    float firstRowY = origin.y + visibleSize.height * 0.6f + 50;
    float secondRowY = origin.y + visibleSize.height * 0.3f + 50;

    setMenuItemsPosition(firstRowItems, startX, firstRowY, horizontalSpacing);
    setMenuItemsPosition(secondRowItems, startX, secondRowY, horizontalSpacing);

    // ================= 创建信息标签 =================
    std::vector<ShopItem> firstRowInfos = {
        {ShopItemType::GOLD_MINE, "Gold Mine", "Produce coins", 1500, 500, 0, false},
        {ShopItemType::WATER_COLLECTOR, "Water Collector", "Produce water", 500, 1500, 0, false},
        {ShopItemType::WALL, "Wall", "Protect your village", 1000, 0, 0, false},
        {ShopItemType::ARCHER_TOWER, "Archer Tower", "Defensive architecture", 2000, 1000, 0, false}
    };

    std::vector<ShopItem> secondRowInfos = {
        {ShopItemType::CANNON, "Cannon", "Defensive architecture", 2500, 1500, 0, false},
        {ShopItemType::BARRACKS, "Barracks", "Train troops", 1200, 800, 0, false},
        {ShopItemType::GOLD_STORAGE, "Gold Storage", "Store more coins", 800, 0, 0, false},
        {ShopItemType::WATER_STORAGE, "Water Storage", "Store more water", 0, 800, 0, false}
    };

    createFirstRowInfoLabels(firstRowItems, firstRowInfos,
        mineCount, mineMax, mineUnlocked,
        waterCount, waterMax, waterUnlocked,
        wallCount, wallMax, wallUnlocked,
        archerCount, archerMax, archerUnlocked);

    createSecondRowInfoLabels(secondRowItems, secondRowInfos,
        cannonCount, cannonMax, cannonUnlocked,
        barracksCount, barracksMax, barracksUnlocked,
        goldStorageCount, goldStorageMax, goldStorageUnlocked,
        waterStorageCount, waterStorageMax, waterStorageUnlocked);

    // ================= 创建菜单并添加到场景 =================
    auto menu = Menu::create();
    for (auto& item : firstRowItems)
    {
        if (item) menu->addChild(item);
    }
    for (auto& item : secondRowItems)
    {
        if (item) menu->addChild(item);
    }

    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);
}

// 获取建筑ID（根据商品类型）
int ShopScene::getBuildingId(ShopItemType type)
{
    switch (type)
    {
        case ShopItemType::WALL:
            return 6;
        case ShopItemType::GOLD_MINE:
            return 2;
        case ShopItemType::WATER_COLLECTOR: 
            return 3;
        case ShopItemType::ARCHER_TOWER: 
            return 4;
        case ShopItemType::CANNON:
            return 5;
        case ShopItemType::BARRACKS:
            return 1;
        case ShopItemType::GOLD_STORAGE: 
            return 8;
        case ShopItemType::WATER_STORAGE: 
            return 9;
        default: return 0;
    }
}

// 获取商品信息
ShopItem ShopScene::getShopItemInfo(ShopItemType type)
{
    // 创建临时的商品信息列表
    std::vector<ShopItem> tempItems = {
        {ShopItemType::GOLD_MINE, "Gold Mine", "Produce coins every hour", 1500, 500, 0, false},
        {ShopItemType::WATER_COLLECTOR, "Water Collector", "Produce water every hour", 500, 1500, 0, false},
        {ShopItemType::WALL, "Wall", "Protect your village", 1000, 0, 0, false},
        {ShopItemType::ARCHER_TOWER, "Archer Tower", "Defensive architecture", 2000, 1000, 0, false},
        {ShopItemType::CANNON, "Cannon", "Defensive architecture", 2500, 1500, 0, false},
        {ShopItemType::BARRACKS, "Barracks", "Train troops", 1200, 800, 0, false},
        {ShopItemType::GOLD_STORAGE, "Gold Storage", "Store more coins", 800, 0, 0, false},
        {ShopItemType::WATER_STORAGE, "Water Storage", "Store more water", 0, 800, 0, false}
    };

    // 查找指定类型的商品信息
    for (auto& item : tempItems)
    {
        if (item.type == type)
        {
            return item;
        }
    }

    // 如果找不到，返回第一个商品信息（默认值）
    return tempItems[0];
}

// 检查是否可以购买商品
bool ShopScene::canPurchaseItem(ShopItemType type, int& buildingId,
    bool& isUnlocked, int& requiredTHLevel,
    int& maxCount, int& currentCount)
{
    auto dataManager = DataManager::getInstance();

    // 获取建筑ID
    buildingId = getBuildingId(type);
    if (buildingId <= 0) return false;

    // 检查是否解锁
    isUnlocked = dataManager->isBuildingUnlocked(buildingId, requiredTHLevel);

    // 获取最大数量限制
    maxCount = dataManager->getBuildingMaxCount(buildingId);

    // 统计当前数量
    currentCount = 0;
    for (auto& building : g_allPurchasedBuildings)
    {
        if (building)
        {
            BuildingType bType = building->getType();
            // 根据商品类型匹配建筑类型
            if ((type == ShopItemType::WALL && bType == BuildingType::WALL) ||
                (type == ShopItemType::GOLD_MINE && bType == BuildingType::MINE) ||
                (type == ShopItemType::WATER_COLLECTOR && bType == BuildingType::WATER) ||
                (type == ShopItemType::ARCHER_TOWER && bType == BuildingType::TOWER)||
                (type == ShopItemType::CANNON && bType == BuildingType::CANNON) ||
                (type == ShopItemType::BARRACKS && bType == BuildingType::BARRACKS) ||
                (type == ShopItemType::GOLD_STORAGE && bType == BuildingType::GOLD_STORAGE) ||
                (type == ShopItemType::WATER_STORAGE && bType == BuildingType::WATER_STORAGE))
            {
                currentCount++;
            }
        }
    }

    return true;
}

// 扣除资源
void ShopScene::deductResources(int coinCost, int waterCost, int gemCost)
{
    coin_count -= coinCost;
    water_count -= waterCost;
    gem_count -= gemCost;
}

// 创建新建筑
Building* ShopScene::createNewBuilding(ShopItemType type)
{
    Building* newBuilding = nullptr;
    std::string filename;
    cocos2d::Rect rect = Rect::ZERO;
    std::string buildingName;
    BuildingType buildingtype;
    int buildingbasecost = 300;

    // 根据商品类型设置建筑属性
    switch (type)
    {
        case ShopItemType::WALL:
            filename = "fence.png";
            buildingName = "Wall";
            buildingtype = BuildingType::WALL;
            buildingbasecost = 0;
            break;
        case ShopItemType::GOLD_MINE:
            filename = "Mine.png";
            buildingName = "Gold Mine";
            buildingtype = BuildingType::MINE;
            break;
        case ShopItemType::WATER_COLLECTOR:
            filename = "waterwell.png";
            buildingName = "Water Collector";
            buildingtype = BuildingType::WATER;
            break;
        case ShopItemType::ARCHER_TOWER:
            filename = "TilesetTowers.png";
            buildingName = "Archer Tower";
            buildingtype = BuildingType::TOWER;
            break;
        case ShopItemType::CANNON:
            filename = "Cannon.png";
            buildingName = "Cannon";
            buildingtype = BuildingType::CANNON;
            break;
        case ShopItemType::BARRACKS:
            filename = "junying.png";
            buildingName = "Barracks";
            buildingtype = BuildingType::BARRACKS;
            break;
        case ShopItemType::GOLD_STORAGE:
            filename = "BarGold.png";
            buildingName = "Gold Storage";
            buildingtype = BuildingType::GOLD_STORAGE;
            break;
        case ShopItemType::WATER_STORAGE:
            filename = "Water.png";
            buildingName = "Water Storage";
            buildingtype = BuildingType::WATER_STORAGE;
            break;
    }

    // 如果找到了对应的图片文件，创建建筑对象
    if (!filename.empty())
    {
        newBuilding = Building::create(filename, rect, buildingName, buildingbasecost, buildingtype);
        if (newBuilding)
        {
            // 设置建筑缩放和纹理参数
            newBuilding->setScale(0.5f);
            newBuilding->getTexture()->setAliasTexParameters();

            // 添加到全局列表（注意内存管理）
            newBuilding->retain();
            g_allPurchasedBuildings.pushBack(newBuilding);
            newBuilding->release();
        }
    }

    return newBuilding;
}

// 处理存储建筑的特殊效果（增加资源上限）
void ShopScene::handleStorageBuildingEffect(ShopItemType type)
{
    if (type == ShopItemType::GOLD_STORAGE)
    {
        coin_limit += 1500;
        CCLOG("[MARKET] Gold Storage purchased! Coin limit increased to: %d", coin_limit);
    }
    else if (type == ShopItemType::WATER_STORAGE)
    {
        water_limit += 1500;
        CCLOG("[MARKET] Water Storage purchased! Water limit increased to: %d", water_limit);
    }
}

// 返回游戏场景
void ShopScene::returnToGameScene()
{
    // 延迟0.5秒后返回游戏场景
    this->runAction(Sequence::create(
        DelayTime::create(0.5f),
        CallFunc::create([]()
            {
                Director::getInstance()->popScene();
            }),
        nullptr
    ));
}

// 重构后的purchaseItem函数（核心购买逻辑）
bool ShopScene::purchaseItem(ShopItemType type)
{
    // 1. 获取物品信息
    ShopItem item = getShopItemInfo(type);

    // 2. 检查购买条件
    int buildingId, requiredTHLevel, maxCount, currentCount;
    bool isUnlocked;

    if (!canPurchaseItem(type, buildingId, isUnlocked, requiredTHLevel, maxCount, currentCount))
    {
        return false;
    }

    // 3. 检查解锁状态
    if (!isUnlocked)
    {
        std::string message = "Requires Town Hall Level " + std::to_string(requiredTHLevel) + "!";
        showPurchaseMessage(false, message);
        return false;
    }

    // 4. 检查数量限制
    if (currentCount >= maxCount)
    {
        std::string message = "Maximum " + std::to_string(maxCount) + " of this building allowed!";
        showPurchaseMessage(false, message);
        return false;
    }

    // 5. 检查资源是否足够
    if (!hasEnoughResources(item.coin_cost, item.water_cost, item.gem_cost))
    {
        showPurchaseMessage(false, item.name);
        return false;
    }

    // 6. 扣除资源
    deductResources(item.coin_cost, item.water_cost, item.gem_cost);

    // 7. 处理存储建筑的特殊效果
    handleStorageBuildingEffect(type);

    // 8. 创建新建筑
    Building* newBuilding = createNewBuilding(type);
    if (!newBuilding)
    {
        CCLOG("ERROR: Failed to create building for type %d", static_cast<int>(type));
        return false;
    }

    // 9. 显示购买成功消息
    showPurchaseMessage(true, item.name);

    // 10. 更新资源显示
    updateResourceLabels();

    // 11. 延迟返回游戏场景
    returnToGameScene();

    return true;
}