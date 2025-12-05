#include "ShopScene.h"
#include "GameScene.h"
#include "Building.h"
#include "HelloWorldScene.h"

USING_NS_CC;

// 声明外部变量
extern int coin_count;
extern int water_count;
extern int gem_count;
extern int coin_limit;
extern int water_limit;
extern int gem_limit;

Scene* ShopScene::createScene() {
    return ShopScene::create();
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

    // ================= 创建商品菜单项 =================

    // 1. 城墙 (225x88) - 直接显示整张图片
    Sprite* wallNormal = Sprite::create("city_wall.png");
    Sprite* wallSelected = Sprite::create("city_wall.png");

    if (wallNormal && wallSelected) {

        // 直接使用Sprite对象，可以调用getTexture()
        // 保持像素清晰
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
     
    // 2. 金矿 (910x1024) - 直接显示整张图片
    Sprite* goldMineNormal = Sprite::create("Mine.png");
    Sprite* goldMineSelected = Sprite::create("Mine.png");

    if (goldMineNormal && goldMineSelected) {

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

    // 3. 圣水收集器 (1013x916) - 直接显示整张图片
    Sprite* waterCollectorNormal = Sprite::create("waterwell.png");
    Sprite* waterCollectorSelected = Sprite::create("waterwell.png");

    if (waterCollectorNormal && waterCollectorSelected) {
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

    // 4. 弓箭塔 (781x1024) - 图集，使用setTextureRect截取特定部分
    // 首先创建精灵但不设置纹理
    Sprite* archerTowerNormal = Sprite::create("TilesetTowers.png");
    Sprite* archerTowerSelected = Sprite::create("TilesetTowers.png");

    if (archerTowerNormal && archerTowerSelected) {
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

    // 5. 加农炮 (824x862) - 直接显示整张图片
    Sprite* cannonNormal = Sprite::create("Cannon.png");
    Sprite* cannonSelected = Sprite::create("Cannon.png");

    if (cannonNormal && cannonSelected) {
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

    // ==================== 设置按钮位置（水平居中排列）====================
// 1. 定义排列顺序（从左到右）：金矿、圣水、城墙、弓箭塔、加农炮
    std::vector<MenuItemSprite*> itemOrder;
    itemOrder.push_back(goldMineItem);      // 最左边
    itemOrder.push_back(waterCollectorItem); // 左二
    itemOrder.push_back(wallItem);           // 中间
    itemOrder.push_back(archerTowerItem);    // 右二
    itemOrder.push_back(cannonItem);         // 最右边

    // 2. 计算水平居中布局
    float centerY = origin.y + visibleSize.height *0.5; // 屏幕垂直中心
    float totalItems = (float)itemOrder.size();           // 商品总数 (5)
    float horizontalSpacing = visibleSize.width * 0.15f;  // 按钮间距 (占屏宽15%)
    // 计算起始X坐标，让整排按钮在屏幕中水平居中
    float totalWidth = (totalItems - 1) * horizontalSpacing;
    float startX = origin.x + (visibleSize.width - totalWidth) / 2.0f+50;

    // 3. 循环设置每个按钮的位置
    for (int i = 0; i < itemOrder.size(); ++i) {
        MenuItemSprite* item = itemOrder[i];
        if (item) {
            float posX = startX + i * horizontalSpacing;
            item->setPosition(Vec2(posX, centerY));
        }
    }
    // ==================== 按钮位置设置结束 ====================
    
   // ==================== 设置商品信息标签（跟随按钮位置）====================
// 商品信息数据（必须与itemOrder的顺序严格对应！）
    std::vector<ShopItem> itemInfos = {
        {ShopItemType::GOLD_MINE, "Gold Mine", "Produce coins every hour", 1500, 500, 0, false},
        {ShopItemType::WATER_COLLECTOR, "Water Collector", "Produce water every hour", 500, 1500, 0, false},
        {ShopItemType::WALL, "Wall", "Protect your village", 1000, 0, 0, false},
        {ShopItemType::ARCHER_TOWER, "Archer Tower", "Defensive architecture", 2000, 1000, 0, false},
        {ShopItemType::CANNON, "Cannon", "Defensive architecture", 2500, 1500, 0, false}
    };

    // 确保商品数量一致
    if (itemOrder.size() == itemInfos.size()) {
        for (int i = 0; i < itemOrder.size(); ++i) {
            MenuItemSprite* itemBtn = itemOrder[i]; // 获取按钮
            ShopItem& info = itemInfos[i];          // 获取对应信息

            if (itemBtn) {
                Vec2 btnPos = itemBtn->getPosition(); // 关键：直接获取按钮的当前位置

                // 1. 商品名称标签 (按钮上方)
                auto nameLabel = Label::createWithTTF(info.name, "fonts/Marker Felt.ttf", 28);
                nameLabel->setPosition(Vec2(btnPos.x-50, btnPos.y + 80)); // 上方偏移
                nameLabel->setColor(Color3B::WHITE);
                this->addChild(nameLabel, 2);

                // 2. 价格标签 (按钮下方)
                std::string priceText;
                if (info.coinCost > 0) priceText += "Coin:" + std::to_string(info.coinCost) + " ";
                if (info.waterCost > 0) priceText += "Water:" + std::to_string(info.waterCost) + " ";
                if (info.gemCost > 0) priceText += "Gem:" + std::to_string(info.gemCost);

                auto priceLabel = Label::createWithTTF(priceText, "fonts/Marker Felt.ttf", 22);
                priceLabel->setPosition(Vec2(btnPos.x - 50, btnPos.y - 180));
                priceLabel->setColor(Color3B::GREEN);
                this->addChild(priceLabel, 2);

                // 3. 描述标签 (价格下方)
                auto descLabel = Label::createWithTTF(info.description, "fonts/Marker Felt.ttf", 18);
                descLabel->setPosition(Vec2(btnPos.x - 50, btnPos.y - 210));
                descLabel->setColor(Color3B::YELLOW);
                this->addChild(descLabel, 2);
            }
        }
    }
    else {
        CCLOG("[MARKET] ERROR:The quantity of the product buttons does not match the information.!");
    }
    // ==================== 信息标签设置结束 ====================

    // 创建菜单（只添加有效的菜单项）
    auto menu = Menu::create();
    if (wallItem) menu->addChild(wallItem);
    if (goldMineItem) menu->addChild(goldMineItem);
    if (waterCollectorItem) menu->addChild(waterCollectorItem);
    if (archerTowerItem) menu->addChild(archerTowerItem);
    if (cannonItem) menu->addChild(cannonItem);

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

bool ShopScene::purchaseItem(ShopItemType type) {
    // 查找商品信息
    std::vector<ShopItem> tempItems = {
        {ShopItemType::GOLD_MINE, "Gold Mine", "Produce coins every hour", 1500, 500, 0, false},
        {ShopItemType::WATER_COLLECTOR, "Water Collector", "Produce water every hour", 500, 1500, 0, false},
        {ShopItemType::WALL, "Wall", "Protect your village", 1000, 0, 0, false},
        {ShopItemType::ARCHER_TOWER, "Archer Tower", "Defensive architecture", 2000, 1000, 0, false},
        {ShopItemType::CANNON, "Cannon", "Defensive architecture", 2500, 1500, 0, false}
    };

    // 查找商品
    for (auto& item : tempItems) {
        if (item.type == type) {
            if (hasEnoughResources(item.coinCost, item.waterCost, item.gemCost)) {
                // 扣除资源
                coin_count -= item.coinCost;
                water_count -= item.waterCost;
                gem_count -= item.gemCost;

                // 显示购买成功
                showPurchaseMessage(true, item.name);

                // 更新资源显示
                updateResourceLabels();

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
        message = "Insufficient resources, unable to purchase " + itemName +"!";
        color = Color4B::RED;
    }

    // ==== 核心修改开始 ====
    // 1. 移除旧的提示（如果存在）
    if (_currentMessageLabel) {
        // 停止该标签上所有计划中的动作（非常重要，防止旧的“移除自身”动作误伤新标签）
        _currentMessageLabel->stopAllActions();
        // 从父节点（当前场景）中移除并清理
        _currentMessageLabel->removeFromParent();
        _currentMessageLabel = nullptr; // 指针置空
    }

    // 2. 创建新的提示标签
    auto label = Label::createWithTTF(message, "fonts/Marker Felt.ttf", 32);
    label->setPosition(Vec2(origin.x + visibleSize.width / 2,
        origin.y + visibleSize.height * 0.2f));
    label->setTextColor(color);
    this->addChild(label, 3); // 注意这里的Z-order (3) 要高于其他界面元素

    // 3. 保存新标签的指针
    _currentMessageLabel = label;

    // 4. 设置新标签3秒后自动移除，并在移除后清空指针
    label->runAction(Sequence::create(
        DelayTime::create(3.0f),
        CallFunc::create([this, label]() {
            // 回调函数：在动作序列中检查并移除
            // 这是一个安全措施，确保只移除我们指向的标签，防止指针已指向新标签
            if (_currentMessageLabel == label) {
                label->removeFromParent();
                _currentMessageLabel = nullptr;
            }
            }),
        nullptr
    ));
    // ==== 核心修改结束 ====
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
    auto scene = GameScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
}