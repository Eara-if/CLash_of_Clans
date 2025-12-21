#include "ShopScene.h"
#include "GameScene.h"
#include "Building.h"
#include "DataManager.h"
#include "HelloWorldScene.h"
#include"AudioEngine.h"
USING_NS_CC;

// 声明外部变量
extern int coin_count;
extern int water_count;
extern int gem_count;
extern int coin_limit;
extern int water_limit;
extern int gem_limit;

extern cocos2d::Vector<Building*> g_allPurchasedBuildings;

Scene* ShopScene::createScene() {
    return ShopScene::create();
}
void ShopScene::onEnter()
{
    // 1. 必须先调用父类的 onEnter，否则场景生命周期会乱
    Scene::onEnter();

    // 2. 停止之前场景（比如主城）的所有声音
    AudioEngine::stopAll();

    log("Enter ShopeScene: Music Stoped.");
}
bool ShopScene::init() {

    if (!Scene::init()) {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    //添加商城的背景
    auto bgSprite = Sprite::create("popup_bg.png");
    if (bgSprite) {
        // 获取图片原始尺寸 (131x144)
        Size bgSize = bgSprite->getContentSize();

        // 计算需要缩放的比例
        float scaleX = visibleSize.width / bgSize.width;
        float scaleY = visibleSize.height / bgSize.height;

        // 设置缩放以填满整个屏幕
        bgSprite->setScaleX(scaleX);
        bgSprite->setScaleY(scaleY);

        // 设置位置为屏幕中心
        bgSprite->setPosition(Vec2(origin.x + visibleSize.width / 2,
            origin.y + visibleSize.height / 2));

        // 保持像素清晰（可选，对于缩放大的图片建议关闭）
        // bgSprite->getTexture()->setAliasTexParameters();

        this->addChild(bgSprite, -1); // 底层背景
    }
    else {
        // 如果图片加载失败，用灰色背景兜底并报错
        CCLOG("ERROR: Failed to load popup_bg.png");
        auto fallbackBg = LayerColor::create(Color4B(100, 100, 100, 255));
        fallbackBg->setContentSize(visibleSize);
        fallbackBg->setPosition(origin);
        this->addChild(fallbackBg, -1);
    }
    //添加背景结束

    // 商城标题
    auto title = Label::createWithTTF("MARKET", "fonts/Marker Felt.ttf", 48);
    title->setPosition(Vec2(origin.x + visibleSize.width / 2,
        origin.y + visibleSize.height - 100));
    title->setColor(Color3B::YELLOW);
    this->addChild(title, 1);

    // 创建商品菜单
    createMenu();

    // 创建资源显示
    updateResourceLabels();

    // 返回按钮
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

void ShopScene::createMenu() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    auto dataManager = DataManager::getInstance();
    // ================= 创建商品菜单项 =================

    // 1. 城墙
    int wallRequiredLevel = 0;
    bool wallUnlocked = dataManager->isBuildingUnlocked(6, wallRequiredLevel);
    int wallMaxCount = dataManager->getBuildingMaxCount(6);
    int wallCurrentCount = 0; // 需要实际统计
    Sprite* wallNormal = Sprite::create("fence.png");
    Sprite* wallSelected = Sprite::create("fence.png");

    if (wallNormal && wallSelected) {
        if (!wallUnlocked) {
            // 未解锁，显示灰色
            wallNormal->setColor(Color3B(100, 100, 100));
            wallSelected->setColor(Color3B(100, 100, 100));
        }
        if (wallNormal->getTexture()) {
            wallNormal->getTexture()->setAliasTexParameters();
        }
        if (wallSelected->getTexture()) {
            wallSelected->getTexture()->setAliasTexParameters();
        }

        wallNormal->setScale(0.5f);
        wallSelected->setScale(0.55f);
        wallSelected->setColor(Color3B(200, 200, 200));
    }

    auto wallItem = MenuItemSprite::create(
        wallNormal,
        wallSelected,
        CC_CALLBACK_1(ShopScene::onWallPurchase, this)
    );

    // 2. 金矿
    int mineRequiredLevel = 0;
    bool mineUnlocked = dataManager->isBuildingUnlocked(2, mineRequiredLevel);
    int mineMaxCount = dataManager->getBuildingMaxCount(2);
    int mineCurrentCount = 0;

    // 统计当前已有的金矿数量
    for (auto& building : g_allPurchasedBuildings) {
        if (building && building->getType() == BuildingType::MINE) {
            mineCurrentCount++;
        }
    }
    Sprite* goldMineNormal = Sprite::create("Mine.png");
    Sprite* goldMineSelected = Sprite::create("Mine.png");

    if (goldMineNormal && goldMineSelected) {
        if (!mineUnlocked) {
            // 未解锁，显示灰色
            goldMineNormal->setColor(Color3B(100, 100, 100));
            goldMineSelected->setColor(Color3B(100, 100, 100));
        }

        if (goldMineNormal->getTexture()) {
            goldMineNormal->getTexture()->setAliasTexParameters();
        }
        if (goldMineSelected->getTexture()) {
            goldMineSelected->getTexture()->setAliasTexParameters();
        }

        goldMineNormal->setScale(0.5f);
        goldMineSelected->setScale(0.55f);
        goldMineSelected->setColor(Color3B(200, 200, 200));
    }

    auto goldMineItem = MenuItemSprite::create(
        goldMineNormal,
        goldMineSelected,
        CC_CALLBACK_1(ShopScene::onGoldMinePurchase, this)
    );

    // 3. 圣水收集器
    int waterRequiredLevel = 0;
    bool waterUnlocked = dataManager->isBuildingUnlocked(3, waterRequiredLevel);
    int waterMaxCount = dataManager->getBuildingMaxCount(3);
    int waterCurrentCount = 0;

    // 统计当前已有的圣水收集器数量
    for (auto& building : g_allPurchasedBuildings) {
        if (building && building->getType() == BuildingType::WATER) {
            waterCurrentCount++;
        }
    }

    Sprite* waterCollectorNormal = Sprite::create("waterwell.png");
    Sprite* waterCollectorSelected = Sprite::create("waterwell.png");

    if (waterCollectorNormal && waterCollectorSelected) {
        if (!waterUnlocked) {
            // 未解锁，显示灰色
            waterCollectorNormal->setColor(Color3B(100, 100, 100));
            waterCollectorSelected->setColor(Color3B(100, 100, 100));
        }
        if (waterCollectorNormal->getTexture()) {
            waterCollectorNormal->getTexture()->setAliasTexParameters();
        }
        if (waterCollectorSelected->getTexture()) {
            waterCollectorSelected->getTexture()->setAliasTexParameters();
        }

        waterCollectorNormal->setScale(0.5f);
        waterCollectorSelected->setScale(0.55f);
        waterCollectorSelected->setColor(Color3B(200, 200, 200));
    }

    auto waterCollectorItem = MenuItemSprite::create(
        waterCollectorNormal,
        waterCollectorSelected,
        CC_CALLBACK_1(ShopScene::onWaterCollectorPurchase, this)
    );

    // 4. 弓箭塔
    int archerRequiredLevel = 0;
    bool archerUnlocked = dataManager->isBuildingUnlocked(4, archerRequiredLevel);
    int archerMaxCount = dataManager->getBuildingMaxCount(4);
    int archerCurrentCount = 0;

    // 统计当前已有的弓箭塔数量
    for (auto& building : g_allPurchasedBuildings) {
        if (building && building->getType() == BuildingType::DEFENSE) {
            archerCurrentCount++;
        }
    }
    Sprite* archerTowerNormal = Sprite::create("TilesetTowers.png");
    Sprite* archerTowerSelected = Sprite::create("TilesetTowers.png");

    if (archerTowerNormal && archerTowerSelected) {
        if (!archerUnlocked) {
            // 未解锁，显示灰色
            archerTowerNormal->setColor(Color3B(100, 100, 100));
            archerTowerSelected->setColor(Color3B(100, 100, 100));
        }
        if (archerTowerNormal->getTexture()) {
            archerTowerNormal->getTexture()->setAliasTexParameters();
        }
        if (archerTowerSelected->getTexture()) {
            archerTowerSelected->getTexture()->setAliasTexParameters();
        }

        archerTowerNormal->setScale(0.5f);
        archerTowerSelected->setScale(0.55f);
        archerTowerSelected->setColor(Color3B(200, 200, 200));
    }

    auto archerTowerItem = MenuItemSprite::create(
        archerTowerNormal,
        archerTowerSelected,
        CC_CALLBACK_1(ShopScene::onArcherTowerPurchase, this)
    );

    // 5. 加农炮
    int cannonRequiredLevel = 0;
    bool cannonUnlocked = dataManager->isBuildingUnlocked(5, cannonRequiredLevel);
    int cannonMaxCount = dataManager->getBuildingMaxCount(5);
    int cannonCurrentCount = 0;

    for (auto& building : g_allPurchasedBuildings) {
        if (building && building->getType() == BuildingType::DEFENSE) {
            cannonCurrentCount++;
        }
    }
    Sprite* cannonNormal = Sprite::create("Cannon.png");
    Sprite* cannonSelected = Sprite::create("Cannon.png");

    if (cannonNormal && cannonSelected) {
        if (!cannonUnlocked) {
            // 未解锁，显示灰色
            cannonNormal->setColor(Color3B(100, 100, 100));
            cannonSelected->setColor(Color3B(100, 100, 100));
        }
        if (cannonNormal->getTexture()) {
            cannonNormal->getTexture()->setAliasTexParameters();
        }
        if (cannonSelected->getTexture()) {
            cannonSelected->getTexture()->setAliasTexParameters();
        }

        cannonNormal->setScale(0.5f);
        cannonSelected->setScale(0.55f);
        cannonSelected->setColor(Color3B(200, 200, 200));
    }

    auto cannonItem = MenuItemSprite::create(
        cannonNormal,
        cannonSelected,
        CC_CALLBACK_1(ShopScene::onCannonPurchase, this)
    );

    // 6. 军营 (新增)
    int barracksRequiredLevel = 0;
    bool barracksUnlocked = dataManager->isBuildingUnlocked(1, barracksRequiredLevel);
    int barracksMaxCount = dataManager->getBuildingMaxCount(1);
    int barracksCurrentCount = 0;

    // 统计当前已有的兵营数量
    for (auto& building : g_allPurchasedBuildings) {
        if (building && building->getType() == BuildingType::BARRACKS) {
            barracksCurrentCount++;
        }
    }
    Sprite* barracksNormal = Sprite::create("junying.png");
    Sprite* barracksSelected = Sprite::create("junying.png");

    if (barracksNormal && barracksSelected) {
        if (!barracksUnlocked) {
            // 未解锁或达到上限，显示灰色
            barracksNormal->setColor(Color3B(100, 100, 100));
            barracksSelected->setColor(Color3B(100, 100, 100));
        }
        if (barracksNormal->getTexture()) {
            barracksNormal->getTexture()->setAliasTexParameters();
        }
        if (barracksSelected->getTexture()) {
            barracksSelected->getTexture()->setAliasTexParameters();
        }

        barracksNormal->setScale(0.5f);
        barracksSelected->setScale(0.55f);
        barracksSelected->setColor(Color3B(200, 200, 200));
    }

    auto barracksItem = MenuItemSprite::create(
        barracksNormal,
        barracksSelected,
        CC_CALLBACK_1(ShopScene::onBarracksPurchase, this)
    );

    // 7. 金币存储器 (新增)
    int goldStorageRequiredLevel = 0;
    bool goldStorageUnlocked = dataManager->isBuildingUnlocked(8, goldStorageRequiredLevel);
    int goldStorageMaxCount = dataManager->getBuildingMaxCount(8);
    int goldStorageCurrentCount = 0;
    for (auto& building : g_allPurchasedBuildings) {
        if (building && building->getType() == BuildingType::GOLD_STORAGE) {
            goldStorageCurrentCount++;
        }
    }
    Sprite* goldStorageNormal = Sprite::create("BarGold.png");
    Sprite* goldStorageSelected = Sprite::create("BarGold.png");

    if (goldStorageNormal && goldStorageSelected) {
        if (!goldStorageUnlocked) {
            // 未解锁，显示灰色
            goldStorageNormal->setColor(Color3B(100, 100, 100));
            goldStorageSelected->setColor(Color3B(100, 100, 100));
        }
        if (goldStorageNormal->getTexture()) {
            goldStorageNormal->getTexture()->setAliasTexParameters();
        }
        if (goldStorageSelected->getTexture()) {
            goldStorageSelected->getTexture()->setAliasTexParameters();
        }

        goldStorageNormal->setScale(0.5f);
        goldStorageSelected->setScale(0.55f);
        goldStorageSelected->setColor(Color3B(200, 200, 200));
    }

    auto goldStorageItem = MenuItemSprite::create(
        goldStorageNormal,
        goldStorageSelected,
        CC_CALLBACK_1(ShopScene::onGoldStoragePurchase, this)
    );

    // 8. 圣水存储器 (新增)
    int waterStorageRequiredLevel = 0;
    bool waterStorageUnlocked = dataManager->isBuildingUnlocked(9, waterStorageRequiredLevel);
    int waterStorageMaxCount = dataManager->getBuildingMaxCount(9);
    int waterStorageCurrentCount = 0;
    for (auto& building : g_allPurchasedBuildings) {
        if (building && building->getType() == BuildingType::WATER_STORAGE) {
            waterStorageCurrentCount++;
        }
    }
    Sprite* waterStorageNormal = Sprite::create("Water.png");
    Sprite* waterStorageSelected = Sprite::create("Water.png");

    if (waterStorageNormal && waterStorageSelected) {
        if (!waterStorageUnlocked) {
            // 未解锁，显示灰色
            waterStorageNormal->setColor(Color3B(100, 100, 100));
            waterStorageSelected->setColor(Color3B(100, 100, 100));
        }
        if (waterStorageNormal->getTexture()) {
            waterStorageNormal->getTexture()->setAliasTexParameters();
        }
        if (waterStorageSelected->getTexture()) {
            waterStorageSelected->getTexture()->setAliasTexParameters();
        }

        waterStorageNormal->setScale(0.5f);
        waterStorageSelected->setScale(0.55f);
        waterStorageSelected->setColor(Color3B(200, 200, 200));
    }

    auto waterStorageItem = MenuItemSprite::create(
        waterStorageNormal,
        waterStorageSelected,
        CC_CALLBACK_1(ShopScene::onWaterStoragePurchase, this)
    );

    // ==================== 设置按钮位置（两行四列布局）====================
    // 定义商品顺序（第一行）
    std::vector<MenuItemSprite*> firstRowItems;
    firstRowItems.push_back(goldMineItem);       // 第1列
    firstRowItems.push_back(waterCollectorItem);  // 第2列
    firstRowItems.push_back(wallItem);           // 第3列
    firstRowItems.push_back(archerTowerItem);     // 第4列

    // 定义商品顺序（第二行）
    std::vector<MenuItemSprite*> secondRowItems;
    secondRowItems.push_back(cannonItem);        // 第1列
    secondRowItems.push_back(barracksItem);      // 第2列
    secondRowItems.push_back(goldStorageItem);   // 第3列
    secondRowItems.push_back(waterStorageItem);  // 第4列

    // 计算布局参数
    float totalItemsPerRow = 4.0f;               // 每行4个商品
    float horizontalSpacing = visibleSize.width * 0.2f;  // 水平间距
    float verticalSpacing = visibleSize.height * 0.25f;  // 垂直间距

    // 第一行Y坐标
    float firstRowY = origin.y + visibleSize.height * 0.6f + 50;
    // 第二行Y坐标
    float secondRowY = origin.y + visibleSize.height * 0.3f + 50;

    // 计算起始X坐标，让整排按钮在屏幕中水平居中
    float totalWidth = (totalItemsPerRow - 1) * horizontalSpacing;
    float startX = origin.x + (visibleSize.width - totalWidth) / 2.0f;

    // 设置第一行按钮位置
    for (int i = 0; i < firstRowItems.size(); ++i) {
        MenuItemSprite* item = firstRowItems[i];
        if (item) {
            float posX = startX + i * horizontalSpacing + 60;
            item->setPosition(Vec2(posX, firstRowY));
        }
    }

    // 设置第二行按钮位置
    for (int i = 0; i < secondRowItems.size(); ++i) {
        MenuItemSprite* item = secondRowItems[i];
        if (item) {
            float posX = startX + i * horizontalSpacing + 60;
            item->setPosition(Vec2(posX, secondRowY));
        }
    }
    // ==================== 按钮位置设置结束 ====================

    // ==================== 设置商品信息标签（跟随按钮位置）====================
    // 商品信息数据（第一行）
    std::vector<ShopItem> firstRowInfos = {
        {ShopItemType::GOLD_MINE, "Gold Mine", "Produce coins", 1500, 500, 0, false},
        {ShopItemType::WATER_COLLECTOR, "Water Collector", "Produce water", 500, 1500, 0, false},
        {ShopItemType::WALL, "Wall", "Protect your village", 1000, 0, 0, false},
        {ShopItemType::ARCHER_TOWER, "Archer Tower", "Defensive architecture", 2000, 1000, 0, false}
    };

    // 商品信息数据（第二行）
    std::vector<ShopItem> secondRowInfos = {
        {ShopItemType::CANNON, "Cannon", "Defensive architecture", 2500, 1500, 0, false},
        {ShopItemType::BARRACKS, "Barracks", "Train troops", 1200, 800, 0, false},        // 新增
        {ShopItemType::GOLD_STORAGE, "Gold Storage", "Store more coins", 800, 0, 0, false},   // 新增
        {ShopItemType::WATER_STORAGE, "Water Storage", "Store more water", 0, 800, 0, false}  // 新增
    };

    // 设置第一行商品信息标签
    for (int i = 0; i < firstRowItems.size(); ++i) {
        MenuItemSprite* itemBtn = firstRowItems[i];
        ShopItem& info = firstRowInfos[i];

        if (itemBtn) {
            Vec2 btnPos = itemBtn->getPosition();

            // 1. 商品名称标签 (按钮上方)
            auto nameLabel = Label::createWithTTF(info.name, "fonts/Marker Felt.ttf", 24);
            nameLabel->setPosition(Vec2(btnPos.x - 50, btnPos.y + 50)); // 上方偏移
            nameLabel->setColor(Color3B::WHITE);
            nameLabel->setAlignment(TextHAlignment::CENTER);
            this->addChild(nameLabel, 2);

            // 2. 价格标签 (按钮下方)
            std::string priceText;
            if (info.coinCost > 0) priceText += "Coin:" + std::to_string(info.coinCost) + " ";
            if (info.waterCost > 0) priceText += "Water:" + std::to_string(info.waterCost) + " ";
            if (info.gemCost > 0) priceText += "Gem:" + std::to_string(info.gemCost);

            // 【新增】添加数量信息
            int currentBuildingCount = 0;
            int maxBuildingCount = 0;

            // 根据建筑类型获取数量信息
            switch (info.type) {
                case ShopItemType::WALL:
                    currentBuildingCount = wallCurrentCount;
                    maxBuildingCount = wallMaxCount;
                    break;
                case ShopItemType::GOLD_MINE:
                    currentBuildingCount = mineCurrentCount;
                    maxBuildingCount = mineMaxCount;
                    break;
                case ShopItemType::WATER_COLLECTOR:
                    currentBuildingCount = waterCurrentCount;
                    maxBuildingCount = waterMaxCount;
                    break;
                case ShopItemType::ARCHER_TOWER:
                    currentBuildingCount = archerCurrentCount;
                    maxBuildingCount = archerMaxCount;
                    break;
                default:
                    break;
            }

            // 【新增】添加解锁状态和数量限制信息
            if (maxBuildingCount > 0) {
                priceText += "\n" + std::to_string(currentBuildingCount) + "/" + std::to_string(maxBuildingCount);
            }

            auto priceLabel = Label::createWithTTF(priceText, "fonts/Marker Felt.ttf", 18); // 字体调小一点
            priceLabel->setPosition(Vec2(btnPos.x - 50, btnPos.y - 180));

            // 【新增】根据解锁状态和数量限制设置颜色
            bool isUnlocked = true;
            switch (info.type) {
                case ShopItemType::WALL:
                    isUnlocked = wallUnlocked && (currentBuildingCount < maxBuildingCount);
                    break;
                case ShopItemType::GOLD_MINE:
                    isUnlocked = mineUnlocked && (currentBuildingCount < maxBuildingCount);
                    break;
                case ShopItemType::WATER_COLLECTOR:
                    isUnlocked = waterUnlocked && (currentBuildingCount < maxBuildingCount);
                    break;
                case ShopItemType::ARCHER_TOWER:
                    isUnlocked = archerUnlocked && (currentBuildingCount < maxBuildingCount);
                    break;
                default:
                    break;
            }

            if (!isUnlocked) {
                priceLabel->setColor(Color3B::GRAY);
            }
            else {
                priceLabel->setColor(Color3B::GREEN);
            }

            priceLabel->setAlignment(TextHAlignment::CENTER);
            this->addChild(priceLabel, 2);

            // 3. 描述标签 (价格下方)
            auto descLabel = Label::createWithTTF(info.description, "fonts/Marker Felt.ttf", 20);
            descLabel->setPosition(Vec2(btnPos.x - 50, btnPos.y - 210));
            descLabel->setColor(Color3B::YELLOW);
            descLabel->setAlignment(TextHAlignment::CENTER);
            this->addChild(descLabel, 2);
        }
    }

    // 设置第二行商品信息标签
    for (int i = 0; i < secondRowItems.size(); ++i) {
        MenuItemSprite* itemBtn = secondRowItems[i];
        ShopItem& info = secondRowInfos[i];

        if (itemBtn) {
            Vec2 btnPos = itemBtn->getPosition();

            // 1. 商品名称标签 (按钮上方)
            auto nameLabel = Label::createWithTTF(info.name, "fonts/Marker Felt.ttf", 24);
            nameLabel->setPosition(Vec2(btnPos.x - 50, btnPos.y + 50)); // 上方偏移
            nameLabel->setColor(Color3B::WHITE);
            nameLabel->setAlignment(TextHAlignment::CENTER);
            this->addChild(nameLabel, 2);

            // 2. 价格标签 (按钮下方)
            std::string priceText;
            if (info.coinCost > 0) priceText += "Coin:" + std::to_string(info.coinCost) + " ";
            if (info.waterCost > 0) priceText += "Water:" + std::to_string(info.waterCost) + " ";
            if (info.gemCost > 0) priceText += "Gem:" + std::to_string(info.gemCost);

            // 【新增】添加数量信息
            int currentBuildingCount = 0;
            int maxBuildingCount = 0;

            // 根据建筑类型获取数量信息
            switch (info.type) {
                case ShopItemType::CANNON:
                    currentBuildingCount = cannonCurrentCount;
                    maxBuildingCount = cannonMaxCount;
                    break;
                case ShopItemType::BARRACKS:
                    currentBuildingCount = barracksCurrentCount;
                    maxBuildingCount = barracksMaxCount;
                    break;
                case ShopItemType::GOLD_STORAGE:
                    currentBuildingCount = goldStorageCurrentCount;
                    maxBuildingCount = goldStorageMaxCount;
                    break;
                case ShopItemType::WATER_STORAGE:
                    currentBuildingCount = waterStorageCurrentCount;
                    maxBuildingCount = waterStorageMaxCount;
                    break;
                default:
                    break;
            }

            // 【新增】添加数量限制信息
            if (maxBuildingCount > 0) {
                priceText += "\n" + std::to_string(currentBuildingCount) + "/" + std::to_string(maxBuildingCount);
            }

            auto priceLabel = Label::createWithTTF(priceText, "fonts/Marker Felt.ttf", 18);
            priceLabel->setPosition(Vec2(btnPos.x - 50, btnPos.y - 180));

            // 【新增】根据解锁状态和数量限制设置颜色
            bool isUnlocked = true;
            switch (info.type) {
                case ShopItemType::CANNON:
                    isUnlocked = cannonUnlocked && (currentBuildingCount < maxBuildingCount);
                    break;
                case ShopItemType::BARRACKS:
                    isUnlocked = barracksUnlocked && (currentBuildingCount < maxBuildingCount);
                    break;
                case ShopItemType::GOLD_STORAGE:
                    isUnlocked = goldStorageUnlocked && (currentBuildingCount < maxBuildingCount);
                    break;
                case ShopItemType::WATER_STORAGE:
                    isUnlocked = waterStorageUnlocked && (currentBuildingCount < maxBuildingCount);
                    break;
                default:
                    break;
            }

            if (!isUnlocked) {
                priceLabel->setColor(Color3B::GRAY);
            }
            else {
                priceLabel->setColor(Color3B::GREEN);
            }

            priceLabel->setAlignment(TextHAlignment::CENTER);
            this->addChild(priceLabel, 2);

            // 3. 描述标签 (价格下方)
            auto descLabel = Label::createWithTTF(info.description, "fonts/Marker Felt.ttf", 20);
            descLabel->setPosition(Vec2(btnPos.x - 50, btnPos.y - 210));
            descLabel->setColor(Color3B::YELLOW);
            descLabel->setAlignment(TextHAlignment::CENTER);
            this->addChild(descLabel, 2);
        }
    }
    // ==================== 信息标签设置结束 ====================

    // 创建菜单（只添加有效的菜单项）
    auto menu = Menu::create();
    if (wallItem) menu->addChild(wallItem);
    if (goldMineItem) menu->addChild(goldMineItem);
    if (waterCollectorItem) menu->addChild(waterCollectorItem);
    if (archerTowerItem) menu->addChild(archerTowerItem);
    if (cannonItem) menu->addChild(cannonItem);
    if (barracksItem) menu->addChild(barracksItem);
    if (goldStorageItem) menu->addChild(goldStorageItem);
    if (waterStorageItem) menu->addChild(waterStorageItem);

    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);
}

// 商品购买回调函数
void ShopScene::onWallPurchase(Ref* pSender) {
    purchaseItem(ShopItemType::WALL);
}

void ShopScene::onGoldMinePurchase(Ref* pSender) {
    purchaseItem(ShopItemType::GOLD_MINE);
}

void ShopScene::onWaterCollectorPurchase(Ref* pSender) {
    purchaseItem(ShopItemType::WATER_COLLECTOR);
}

void ShopScene::onArcherTowerPurchase(Ref* pSender) {
    purchaseItem(ShopItemType::ARCHER_TOWER);
}

void ShopScene::onCannonPurchase(Ref* pSender) {
    purchaseItem(ShopItemType::CANNON);
}

void ShopScene::onBarracksPurchase(Ref* pSender) {
    purchaseItem(ShopItemType::BARRACKS);
}

void ShopScene::onGoldStoragePurchase(Ref* pSender) {
    purchaseItem(ShopItemType::GOLD_STORAGE);
}

void ShopScene::onWaterStoragePurchase(Ref* pSender) {
    purchaseItem(ShopItemType::WATER_STORAGE);
}

bool ShopScene::purchaseItem(ShopItemType type) {

    auto dataManager = DataManager::getInstance();
    int requiredTHLevel = 0;
    bool isUnlocked = false;
    int buildingId = 0;

    // 映射 ShopItemType 到 buildingId
    switch (type) {
        case ShopItemType::WALL:
            buildingId = 6; break;
        case ShopItemType::GOLD_MINE:
            buildingId = 2; break;
        case ShopItemType::WATER_COLLECTOR:
            buildingId = 3; break;
        case ShopItemType::ARCHER_TOWER:
            buildingId = 4; break;
        case ShopItemType::CANNON:
            buildingId = 5; break;
        case ShopItemType::BARRACKS:
            buildingId = 1; break;
        case ShopItemType::GOLD_STORAGE:
            buildingId = 8; break;
        case ShopItemType::WATER_STORAGE:
            buildingId = 9; break;
        default:
            buildingId = 0; break;
    }

    // 检查是否解锁
    if (buildingId > 0) {
        isUnlocked = dataManager->isBuildingUnlocked(buildingId, requiredTHLevel);
    }

    // 【新增】检查数量限制
    int maxCount = dataManager->getBuildingMaxCount(buildingId);
    int currentCount = 0;

    // 统计当前已有的该类型建筑数量
    for (auto& building : g_allPurchasedBuildings) {
        if (building) {
            BuildingType bType = building->getType();
            if ((type == ShopItemType::WALL && bType == BuildingType::WALL) ||
                (type == ShopItemType::GOLD_MINE && bType == BuildingType::MINE) ||
                (type == ShopItemType::WATER_COLLECTOR && bType == BuildingType::WATER) ||
                (type == ShopItemType::ARCHER_TOWER && bType == BuildingType::DEFENSE) ||
                (type == ShopItemType::CANNON && bType == BuildingType::DEFENSE) ||
                (type == ShopItemType::BARRACKS && bType == BuildingType::BARRACKS) ||
                (type == ShopItemType::GOLD_STORAGE && bType == BuildingType::GOLD_STORAGE) ||
                (type == ShopItemType::WATER_STORAGE && bType == BuildingType::WATER_STORAGE)) {
                currentCount++;
            }
        }
    }

    // 商品信息（包含所有8个商品）
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

    // 查找商品
    for (auto& item : tempItems) {
        if (item.type == type) {
            // 【新增】检查解锁条件
            if (!isUnlocked) {
                std::string message = "Requires Town Hall Level " + std::to_string(requiredTHLevel) + "!";
                showPurchaseMessage(false, message);
                return false;
            }

            // 【新增】检查数量限制
            if (currentCount >= maxCount) {
                std::string message = "Maximum " + std::to_string(maxCount) + " of this building allowed!";
                showPurchaseMessage(false, message);
                return false;
            }

            if (hasEnoughResources(item.coinCost, item.waterCost, item.gemCost)) {
                // 扣除资源
                coin_count -= item.coinCost;
                water_count -= item.waterCost;
                gem_count -= item.gemCost;

                // ==== 新增：检查是否是储存器并增加上限 ====
                bool isStorageBuilding = false;

                if (type == ShopItemType::GOLD_STORAGE) {
                    coin_limit += 1500;  // 增加金币上限
                    isStorageBuilding = true;
                    CCLOG("[MARKET] Gold Storage purchased! Coin limit increased to: %d", coin_limit);
                }
                else if (type == ShopItemType::WATER_STORAGE) {
                    water_limit += 1500;  // 增加圣水上限
                    isStorageBuilding = true;
                    CCLOG("[MARKET] Water Storage purchased! Water limit increased to: %d", water_limit);
                }

                // ==== 创建建筑对象 ====
                Building* newBuilding = nullptr;
                std::string filename;
                cocos2d::Rect rect = Rect::ZERO;
                std::string buildingName;
                BuildingType buildingtype;
                int buildingbasecost = 300; // 默认设为300

                switch (type) {
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
                        buildingbasecost = 300;
                        break;
                    case ShopItemType::WATER_COLLECTOR:
                        filename = "waterwell.png";
                        buildingName = "Water Collector";
                        buildingtype = BuildingType::WATER;
                        buildingbasecost = 300;
                        break;
                    case ShopItemType::ARCHER_TOWER:
                        filename = "TilesetTowers.png";
                        buildingName = "Archer Tower";
                        buildingtype = BuildingType::DEFENSE;
                        buildingbasecost = 300;
                        break;
                    case ShopItemType::CANNON:
                        filename = "Cannon.png";
                        buildingName = "Cannon";
                        buildingtype = BuildingType::DEFENSE;
                        buildingbasecost = 300;
                        break;
                    case ShopItemType::BARRACKS:
                        filename = "junying.png";
                        buildingName = "Barracks";
                        buildingtype = BuildingType::BARRACKS;
                        buildingbasecost = 300;
                        break;
                    case ShopItemType::GOLD_STORAGE:
                        filename = "BarGold.png";
                        buildingName = "Gold Storage";
                        buildingtype = BuildingType::GOLD_STORAGE;
                        buildingbasecost = 300;
                        break;
                    case ShopItemType::WATER_STORAGE:
                        filename = "Water.png";
                        buildingName = "Water Storage";
                        buildingtype = BuildingType::WATER_STORAGE;
                        buildingbasecost = 300;
                        break;
                }

                if (!filename.empty()) {
                    newBuilding = Building::create(filename, rect, buildingName, buildingbasecost, buildingtype);
                    if (newBuilding) {
                        newBuilding->setScale(0.5f);
                        newBuilding->getTexture()->setAliasTexParameters();
                        // 添加到全局容器
                        newBuilding->retain();
                        g_allPurchasedBuildings.pushBack(newBuilding);
                        newBuilding->release();
                    }
                }

                // 显示购买成功
                showPurchaseMessage(true, item.name);

                // 更新资源显示
                updateResourceLabels();

                // 延迟0.5秒后返回游戏场景
                this->runAction(Sequence::create(
                    DelayTime::create(0.5f),
                    CallFunc::create([this]() {
                        Director::getInstance()->popScene();
                        }),
                    nullptr
                ));
                return true;
            }
            else {
                showPurchaseMessage(false, item.name);
                return false;
            }
        }
    }
    return false;
}

bool ShopScene::hasEnoughResources(int coinCost, int waterCost, int gemCost) {
    return coin_count >= coinCost && water_count >= waterCost && gem_count >= gemCost;
}

void ShopScene::showPurchaseMessage(bool success, const std::string& itemName) {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    std::string message;
    Color4B color;

    if (success) {
        message = "Successful purchase " + itemName + "!";
        color = Color4B::GREEN;
    }
    else {
        message = "Insufficient resources, unable to purchase " + itemName + "!";
        color = Color4B::RED;
    }

    // 移除旧的提示（如果存在）
    if (_currentMessageLabel) {
        _currentMessageLabel->stopAllActions();
        _currentMessageLabel->removeFromParent();
        _currentMessageLabel = nullptr;
    }

    // 创建新的提示标签
    auto label = Label::createWithTTF(message, "fonts/Marker Felt.ttf", 32);
    label->setPosition(Vec2(origin.x + visibleSize.width / 2,
        origin.y + visibleSize.height * 0.1f));
    label->setTextColor(color);
    this->addChild(label, 3);

    // 保存新标签的指针
    _currentMessageLabel = label;

    // 设置新标签3秒后自动移除
    label->runAction(Sequence::create(
        DelayTime::create(3.0f),
        CallFunc::create([this, label]() {
            if (_currentMessageLabel == label) {
                label->removeFromParent();
                _currentMessageLabel = nullptr;
            }
            }),
        nullptr
    ));
}

void ShopScene::updateResourceLabels() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 移除旧的标签
    if (_coinLabel) _coinLabel->removeFromParent();
    if (_waterLabel) _waterLabel->removeFromParent();
    if (_gemLabel) _gemLabel->removeFromParent();

    // 创建新的资源显示
    _coinLabel = Label::createWithTTF("Coin: " + std::to_string(coin_count) + "/" + std::to_string(coin_limit),
        "fonts/Marker Felt.ttf", 28);
    _waterLabel = Label::createWithTTF("Water: " + std::to_string(water_count) + "/" + std::to_string(water_limit),
        "fonts/Marker Felt.ttf", 28);
    _gemLabel = Label::createWithTTF("Gem: " + std::to_string(gem_count),
        "fonts/Marker Felt.ttf", 28);

    _coinLabel->setPosition(Vec2(origin.x + visibleSize.width / 2,
        origin.y + visibleSize.height - 150));
    _waterLabel->setPosition(Vec2(origin.x + visibleSize.width / 2,
        origin.y + visibleSize.height - 190));
    _gemLabel->setPosition(Vec2(origin.x + visibleSize.width / 2,
        origin.y + visibleSize.height - 230));

    _coinLabel->setColor(Color3B::YELLOW);
    _waterLabel->setColor(Color3B::BLUE);
    _gemLabel->setColor(Color3B::MAGENTA);

    this->addChild(_coinLabel, 2);
    this->addChild(_waterLabel, 2);
    this->addChild(_gemLabel, 2);
}

void ShopScene::menuBackCallback(Ref* pSender) {
    Director::getInstance()->popScene();
}

