#include "ShopScene.h"
#include "GameScene.h"
#include "Building.h"
#include "DataManager.h"
#include "HelloWorldScene.h"
#include"AudioEngine.h"
USING_NS_CC;
 
// �����ⲿ����
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
    // 1. �����ȵ��ø���� onEnter�����򳡾��������ڻ���
    Scene::onEnter();

    // 2. ֹ֮ͣǰ�������������ǣ�����������
    AudioEngine::stopAll();
    
    log("Enter ShopeScene: Music Stoped.");
}
bool ShopScene::init() {

    if (!Scene::init()) {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    //����̳ǵı���
    auto bgSprite = Sprite::create("popup_bg.png");
    if (bgSprite) {
        // ��ȡͼƬԭʼ�ߴ� (131x144)
        Size bgSize = bgSprite->getContentSize();

        // ������Ҫ���ŵı���
        float scaleX = visibleSize.width / bgSize.width;
        float scaleY = visibleSize.height / bgSize.height;

        // ��������������������Ļ
        bgSprite->setScaleX(scaleX);
        bgSprite->setScaleY(scaleY);

        // ����λ��Ϊ��Ļ����
        bgSprite->setPosition(Vec2(origin.x + visibleSize.width / 2,
            origin.y + visibleSize.height / 2));

        // ����������������ѡ���������Ŵ��ͼƬ����رգ�
        // bgSprite->getTexture()->setAliasTexParameters();

        this->addChild(bgSprite, -1); // �ײ㱳��
    }
    else {
        // ���ͼƬ����ʧ�ܣ��û�ɫ�������ײ�����
        CCLOG("ERROR: Failed to load popup_bg.png");
        auto fallbackBg = LayerColor::create(Color4B(100, 100, 100, 255));
        fallbackBg->setContentSize(visibleSize);
        fallbackBg->setPosition(origin);
        this->addChild(fallbackBg, -1);
    }
    //��ӱ�������

    // �̳Ǳ���
    auto title = Label::createWithTTF("MARKET", "fonts/Marker Felt.ttf", 48);
    title->setPosition(Vec2(origin.x + visibleSize.width / 2,
        origin.y + visibleSize.height - 100));
    title->setColor(Color3B::YELLOW);
    this->addChild(title, 1);

    // ������Ʒ�˵�
    createMenu();

    // ������Դ��ʾ
    updateResourceLabels();

    // ���ذ�ť
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
    // ================= ������Ʒ�˵��� =================

    // 1. ��ǽ
    int wallRequiredLevel = 0;
    bool wallUnlocked = dataManager->isBuildingUnlocked(6, wallRequiredLevel);
    int wallMaxCount = dataManager->getBuildingMaxCount(6);
    int wallCurrentCount = 0; // ��Ҫʵ��ͳ��
    Sprite* wallNormal = Sprite::create("fence.png");
    Sprite* wallSelected = Sprite::create("fence.png");

    if (wallNormal && wallSelected) {
        if (!wallUnlocked) {
            // δ��������ʾ��ɫ
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

    // 2. ���
    int mineRequiredLevel = 0;
    bool mineUnlocked = dataManager->isBuildingUnlocked(2, mineRequiredLevel);
    int mineMaxCount = dataManager->getBuildingMaxCount(2);
    int mineCurrentCount = 0;

    // ͳ�Ƶ�ǰ���еĽ������
    for (auto& building : g_allPurchasedBuildings) {
        if (building && building->getType() == BuildingType::MINE) {
            mineCurrentCount++;
        }
    }
    Sprite* goldMineNormal = Sprite::create("Mine.png");
    Sprite* goldMineSelected = Sprite::create("Mine.png");

    if (goldMineNormal && goldMineSelected) {
        if (!mineUnlocked) {
            // δ��������ʾ��ɫ
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

    // 3. ʥˮ�ռ���
    int waterRequiredLevel = 0;
    bool waterUnlocked = dataManager->isBuildingUnlocked(3, waterRequiredLevel);
    int waterMaxCount = dataManager->getBuildingMaxCount(3);
    int waterCurrentCount = 0;

    // ͳ�Ƶ�ǰ���е�ʥˮ�ռ�������
    for (auto& building : g_allPurchasedBuildings) {
        if (building && building->getType() == BuildingType::WATER) {
            waterCurrentCount++;
        }
    }

    Sprite* waterCollectorNormal = Sprite::create("waterwell.png");
    Sprite* waterCollectorSelected = Sprite::create("waterwell.png");

    if (waterCollectorNormal && waterCollectorSelected) {
        if (!waterUnlocked) {
            // δ��������ʾ��ɫ
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

    // 4. ������
    int archerRequiredLevel = 0;
    bool archerUnlocked = dataManager->isBuildingUnlocked(4, archerRequiredLevel);
    int archerMaxCount = dataManager->getBuildingMaxCount(4);
    int archerCurrentCount = 0;

    // ͳ�Ƶ�ǰ���еĹ���������
    for (auto& building : g_allPurchasedBuildings) {
        if (building && building->getType() == BuildingType::DEFENSE) {
            archerCurrentCount++;
        }
    }
    Sprite* archerTowerNormal = Sprite::create("TilesetTowers.png");
    Sprite* archerTowerSelected = Sprite::create("TilesetTowers.png");

    if (archerTowerNormal && archerTowerSelected) {
        if (!archerUnlocked) {
            // δ��������ʾ��ɫ
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

    // 5. ��ũ��
    int cannonRequiredLevel = 0;
    bool cannonUnlocked = dataManager->isBuildingUnlocked(5, cannonRequiredLevel);
    int cannonMaxCount = dataManager->getBuildingMaxCount(5);
    int cannonCurrentCount = 0;

    for (auto& building : g_allPurchasedBuildings) {
        if (building && building->getType() == BuildingType::CANNON) {
            cannonCurrentCount++;
        }
    }
    Sprite* cannonNormal = Sprite::create("Cannon.png");
    Sprite* cannonSelected = Sprite::create("Cannon.png");

    if (cannonNormal && cannonSelected) {
        if (!cannonUnlocked) {
            // δ��������ʾ��ɫ
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

    // 6. ��Ӫ (����)
    int barracksRequiredLevel = 0;
    bool barracksUnlocked = dataManager->isBuildingUnlocked(1, barracksRequiredLevel);
    int barracksMaxCount = dataManager->getBuildingMaxCount(1);
    int barracksCurrentCount = 0;

    // ͳ�Ƶ�ǰ���еı�Ӫ����
    for (auto& building : g_allPurchasedBuildings) {
        if (building && building->getType() == BuildingType::BARRACKS) {
            barracksCurrentCount++;
        }
    }
    Sprite* barracksNormal = Sprite::create("junying.png");
    Sprite* barracksSelected = Sprite::create("junying.png");

    if (barracksNormal && barracksSelected) {
        if (!barracksUnlocked) {
            // δ������ﵽ���ޣ���ʾ��ɫ
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

    // 7. ��Ҵ洢�� (����)
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
            // δ��������ʾ��ɫ
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

    // 8. ʥˮ�洢�� (����)
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
            // δ��������ʾ��ɫ
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

    // ==================== ���ð�ťλ�ã��������в��֣�====================
    // ������Ʒ˳�򣨵�һ�У�
    std::vector<MenuItemSprite*> firstRowItems;
    firstRowItems.push_back(goldMineItem);       // ��1��
    firstRowItems.push_back(waterCollectorItem);  // ��2��
    firstRowItems.push_back(wallItem);           // ��3��
    firstRowItems.push_back(archerTowerItem);     // ��4��

    // ������Ʒ˳�򣨵ڶ��У�
    std::vector<MenuItemSprite*> secondRowItems;
    secondRowItems.push_back(cannonItem);        // ��1��
    secondRowItems.push_back(barracksItem);      // ��2��
    secondRowItems.push_back(goldStorageItem);   // ��3��
    secondRowItems.push_back(waterStorageItem);  // ��4��

    // ���㲼�ֲ���
    float totalItemsPerRow = 4.0f;               // ÿ��4����Ʒ
    float horizontalSpacing = visibleSize.width * 0.2f;  // ˮƽ���
    float verticalSpacing = visibleSize.height * 0.25f;  // ��ֱ���

    // ��һ��Y����
    float firstRowY = origin.y + visibleSize.height * 0.6f + 50;
    // �ڶ���Y����
    float secondRowY = origin.y + visibleSize.height * 0.3f + 50;

    // ������ʼX���꣬�����Ű�ť����Ļ��ˮƽ����
    float totalWidth = (totalItemsPerRow - 1) * horizontalSpacing;
    float startX = origin.x + (visibleSize.width - totalWidth) / 2.0f;

    // ���õ�һ�а�ťλ��
    for (int i = 0; i < firstRowItems.size(); ++i) {
        MenuItemSprite* item = firstRowItems[i];
        if (item) {
            float posX = startX + i * horizontalSpacing + 60;
            item->setPosition(Vec2(posX, firstRowY));
        }
    }

    // ���õڶ��а�ťλ��
    for (int i = 0; i < secondRowItems.size(); ++i) {
        MenuItemSprite* item = secondRowItems[i];
        if (item) {
            float posX = startX + i * horizontalSpacing + 60;
            item->setPosition(Vec2(posX, secondRowY));
        }
    }
    // ==================== ��ťλ�����ý��� ====================

    // ==================== ������Ʒ��Ϣ��ǩ�����水ťλ�ã�====================
    // ��Ʒ��Ϣ���ݣ���һ�У�
    std::vector<ShopItem> firstRowInfos = {
        {ShopItemType::GOLD_MINE, "Gold Mine", "Produce coins", 1500, 500, 0, false},
        {ShopItemType::WATER_COLLECTOR, "Water Collector", "Produce water", 500, 1500, 0, false},
        {ShopItemType::WALL, "Wall", "Protect your village", 1000, 0, 0, false},
        {ShopItemType::ARCHER_TOWER, "Archer Tower", "Defensive architecture", 2000, 1000, 0, false}
    };

    // ��Ʒ��Ϣ���ݣ��ڶ��У�
    std::vector<ShopItem> secondRowInfos = {
        {ShopItemType::CANNON, "Cannon", "Defensive architecture", 2500, 1500, 0, false},
        {ShopItemType::BARRACKS, "Barracks", "Train troops", 1200, 800, 0, false},        // ����
        {ShopItemType::GOLD_STORAGE, "Gold Storage", "Store more coins", 800, 0, 0, false},   // ����
        {ShopItemType::WATER_STORAGE, "Water Storage", "Store more water", 0, 800, 0, false}  // ����
    };

    // ���õ�һ����Ʒ��Ϣ��ǩ
    for (int i = 0; i < firstRowItems.size(); ++i) {
        MenuItemSprite* itemBtn = firstRowItems[i];
        ShopItem& info = firstRowInfos[i];

        if (itemBtn) {
            Vec2 btnPos = itemBtn->getPosition();

            // 1. ��Ʒ���Ʊ�ǩ (��ť�Ϸ�)
            auto nameLabel = Label::createWithTTF(info.name, "fonts/Marker Felt.ttf", 24);
            nameLabel->setPosition(Vec2(btnPos.x - 50, btnPos.y + 50)); // �Ϸ�ƫ��
            nameLabel->setColor(Color3B::WHITE);
            nameLabel->setAlignment(TextHAlignment::CENTER);
            this->addChild(nameLabel, 2);

            // 2. �۸��ǩ (��ť�·�)
            std::string priceText;
            if (info.coinCost > 0) priceText += "Coin:" + std::to_string(info.coinCost) + " ";
            if (info.waterCost > 0) priceText += "Water:" + std::to_string(info.waterCost) + " ";
            if (info.gemCost > 0) priceText += "Gem:" + std::to_string(info.gemCost);

            // �����������������Ϣ
            int currentBuildingCount = 0;
            int maxBuildingCount = 0;

            // ���ݽ������ͻ�ȡ������Ϣ
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

            // ����������ӽ���״̬������������Ϣ
            if (maxBuildingCount > 0) {
                priceText += "\n" + std::to_string(currentBuildingCount) + "/" + std::to_string(maxBuildingCount);
            }

            auto priceLabel = Label::createWithTTF(priceText, "fonts/Marker Felt.ttf", 18); // �����Сһ��
            priceLabel->setPosition(Vec2(btnPos.x - 50, btnPos.y - 180));

            // �����������ݽ���״̬����������������ɫ
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

            // 3. ������ǩ (�۸��·�)
            auto descLabel = Label::createWithTTF(info.description, "fonts/Marker Felt.ttf", 20);
            descLabel->setPosition(Vec2(btnPos.x - 50, btnPos.y - 210));
            descLabel->setColor(Color3B::YELLOW);
            descLabel->setAlignment(TextHAlignment::CENTER);
            this->addChild(descLabel, 2);
        }
    }

    // ���õڶ�����Ʒ��Ϣ��ǩ
    for (int i = 0; i < secondRowItems.size(); ++i) {
        MenuItemSprite* itemBtn = secondRowItems[i];
        ShopItem& info = secondRowInfos[i];

        if (itemBtn) {
            Vec2 btnPos = itemBtn->getPosition();

            // 1. ��Ʒ���Ʊ�ǩ (��ť�Ϸ�)
            auto nameLabel = Label::createWithTTF(info.name, "fonts/Marker Felt.ttf", 24);
            nameLabel->setPosition(Vec2(btnPos.x - 50, btnPos.y + 50)); // �Ϸ�ƫ��
            nameLabel->setColor(Color3B::WHITE);
            nameLabel->setAlignment(TextHAlignment::CENTER);
            this->addChild(nameLabel, 2);

            // 2. �۸��ǩ (��ť�·�)
            std::string priceText;
            if (info.coinCost > 0) priceText += "Coin:" + std::to_string(info.coinCost) + " ";
            if (info.waterCost > 0) priceText += "Water:" + std::to_string(info.waterCost) + " ";
            if (info.gemCost > 0) priceText += "Gem:" + std::to_string(info.gemCost);

            // �����������������Ϣ
            int currentBuildingCount = 0;
            int maxBuildingCount = 0;

            // ���ݽ������ͻ�ȡ������Ϣ
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

            // ���������������������Ϣ
            if (maxBuildingCount > 0) {
                priceText += "\n" + std::to_string(currentBuildingCount) + "/" + std::to_string(maxBuildingCount);
            }

            auto priceLabel = Label::createWithTTF(priceText, "fonts/Marker Felt.ttf", 18);
            priceLabel->setPosition(Vec2(btnPos.x - 50, btnPos.y - 180));

            // �����������ݽ���״̬����������������ɫ
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

            // 3. ������ǩ (�۸��·�)
            auto descLabel = Label::createWithTTF(info.description, "fonts/Marker Felt.ttf", 20);
            descLabel->setPosition(Vec2(btnPos.x - 50, btnPos.y - 210));
            descLabel->setColor(Color3B::YELLOW);
            descLabel->setAlignment(TextHAlignment::CENTER);
            this->addChild(descLabel, 2);
        }
    }
    // ==================== ��Ϣ��ǩ���ý��� ====================

    // �����˵���ֻ�����Ч�Ĳ˵��
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

// ��Ʒ����ص�����
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

    // ӳ�� ShopItemType �� buildingId
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

    // ����Ƿ����
    if (buildingId > 0) {
        isUnlocked = dataManager->isBuildingUnlocked(buildingId, requiredTHLevel);
    }

    // �������������������
    int maxCount = dataManager->getBuildingMaxCount(buildingId);
    int currentCount = 0;

    // ͳ�Ƶ�ǰ���еĸ����ͽ�������
    for (auto& building : g_allPurchasedBuildings) {
        if (building) {
            BuildingType bType = building->getType();
            if ((type == ShopItemType::WALL && bType == BuildingType::WALL) ||
                (type == ShopItemType::GOLD_MINE && bType == BuildingType::MINE) ||
                (type == ShopItemType::WATER_COLLECTOR && bType == BuildingType::WATER) ||
                (type == ShopItemType::ARCHER_TOWER && bType == BuildingType::DEFENSE) ||
                (type == ShopItemType::CANNON && bType == BuildingType::CANNON) ||
                (type == ShopItemType::BARRACKS && bType == BuildingType::BARRACKS) ||
                (type == ShopItemType::GOLD_STORAGE && bType == BuildingType::GOLD_STORAGE) ||
                (type == ShopItemType::WATER_STORAGE && bType == BuildingType::WATER_STORAGE)) {
                currentCount++;
            }
        }
    }

    // ��Ʒ��Ϣ����������8����Ʒ��
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

    // ������Ʒ
    for (auto& item : tempItems) {
        if (item.type == type) {
            // ������������������
            if (!isUnlocked) {
                std::string message = "Requires Town Hall Level " + std::to_string(requiredTHLevel) + "!";
                showPurchaseMessage(false, message);
                return false;
            }

            // �������������������
            if (currentCount >= maxCount) {
                std::string message = "Maximum " + std::to_string(maxCount) + " of this building allowed!";
                showPurchaseMessage(false, message);
                return false;
            }

            if (hasEnoughResources(item.coinCost, item.waterCost, item.gemCost)) {
                // �۳���Դ
                coin_count -= item.coinCost;
                water_count -= item.waterCost;
                gem_count -= item.gemCost;

                // ==== ����������Ƿ��Ǵ��������������� ====
                bool isStorageBuilding = false;

                if (type == ShopItemType::GOLD_STORAGE) {
                    coin_limit += 1500;  // ���ӽ������
                    isStorageBuilding = true;
                    CCLOG("[MARKET] Gold Storage purchased! Coin limit increased to: %d", coin_limit);
                }
                else if (type == ShopItemType::WATER_STORAGE) {
                    water_limit += 1500;  // ����ʥˮ����
                    isStorageBuilding = true;
                    CCLOG("[MARKET] Water Storage purchased! Water limit increased to: %d", water_limit);
                }

                // ==== ������������ ====
                Building* newBuilding = nullptr;
                std::string filename;
                cocos2d::Rect rect = Rect::ZERO;
                std::string buildingName;
                BuildingType buildingtype;
                int buildingbasecost = 300; // Ĭ����Ϊ300

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
                        buildingtype = BuildingType::TOWER;
                        buildingbasecost = 300;
                        break;
                    case ShopItemType::CANNON:
                        filename = "Cannon.png";
                        buildingName = "Cannon";
                        buildingtype = BuildingType::CANNON;
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
                        // ��ӵ�ȫ������
                        newBuilding->retain();
                        g_allPurchasedBuildings.pushBack(newBuilding);
                        newBuilding->release();
                    }
                }

                // ��ʾ����ɹ�
                showPurchaseMessage(true, item.name);

                // ������Դ��ʾ
                updateResourceLabels();

                // �ӳ�0.5��󷵻���Ϸ����
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

    // �Ƴ��ɵ���ʾ��������ڣ�
    if (_currentMessageLabel) {
        _currentMessageLabel->stopAllActions();
        _currentMessageLabel->removeFromParent();
        _currentMessageLabel = nullptr;
    }

    // �����µ���ʾ��ǩ
    auto label = Label::createWithTTF(message, "fonts/Marker Felt.ttf", 32);
    label->setPosition(Vec2(origin.x + visibleSize.width / 2,
        origin.y + visibleSize.height * 0.1f));
    label->setTextColor(color);
    this->addChild(label, 3);

    // �����±�ǩ��ָ��
    _currentMessageLabel = label;

    // �����±�ǩ3����Զ��Ƴ�
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

    // �Ƴ��ɵı�ǩ
    if (_coinLabel) _coinLabel->removeFromParent();
    if (_waterLabel) _waterLabel->removeFromParent();
    if (_gemLabel) _gemLabel->removeFromParent();

    // �����µ���Դ��ʾ
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

