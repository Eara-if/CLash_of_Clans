#include "Building.h"
#include "BuildingInfoLayer.h" // һ��Ҫ����֮ǰд�ĵ�����
#include "BuildingUpgradeLimits.h"
#include "GameScene.h"
USING_NS_CC;

// ����ȫ�ֱ��� (���߱�����ȥ��ĵط����������)
extern int coin_count;
extern int water_count;
extern int gem_count;

extern int coin_limit;
extern int water_limit;
extern int gem_limit;


Building* Building::create(const std::string& filename, const Rect& rect, const std::string& name, int baseCost, BuildingType type)
{
    Building* ret = new (std::nothrow) Building();
    if (ret && ret->init(filename, rect, name, baseCost, type)) // ���� type
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool Building::init(const std::string& filename, const Rect& rect, const std::string& name, int baseCost, BuildingType type1)
{

    if (rect.equals(Rect::ZERO))
    {
        // initWithFile ֻ���ļ��������Ǽ�������ͼ
        if (!Sprite::initWithFile(filename)) return false;
    }
    else
    {
        // ���򣬰��� rect ���вü�
        if (!Sprite::initWithFile(filename, rect)) return false;
        // ע�⣺��Щ�汾�� Cocos �ڴ� rect ��ʼ��ʱ����Ҫ�� setTextureRect��
        // ��Ϊ�˱��������������״�����ʵ����ʾ����
    }

    type = type1; // ���������͡�
    a_level = 1;
    buildingName = name;
    baseCost1 = baseCost;
    state = BuildingState::IDLE;
    timeLeft = 0;

    // ��ʼ��������ر���
    productionTimeLeft = 0;
    productionAmount = 50; // Ĭ������50��Դ
    isReadyToCollect = false;
    readyIndicator = nullptr;

    // ����ǽ���ʥˮ�ռ�������ʼ����
    if (type == BuildingType::MINE || type == BuildingType::WATER) {
        startProduction();
    }

    this->initTouchListener();
    this->scheduleUpdate();

    return true;
}
// Building.cpp

void Building::onEnter()
{
    // 1. ������ø��� onEnter
    Sprite::onEnter();

    // 2. ����ʱ�����ػ�
    // ��Ϊ�Ǽ����Լ����ϣ�����������԰�ȫ�����ᵼ�� Crash
    this->createGroundEffect();
}
void Building::setOnUpgradeCallback(std::function<void()> callback)
{
    UpgradeCallback_coin = callback;
}

int Building::getNextLevelCost()
{
    // �������߼���������ȼ�����
    // ���磺1����2��=500��2����3��=1000��3����4��=1500
    return baseCost1 * a_level;

    // ������ָ�������� return _baseCost * std::pow(2, _level - 1);
}
// ÿ֡�Զ�����
void Building::update(float dt)
{
    if (state == BuildingState::UPGRADING)
    {
        timeLeft -= dt; // �۳�ʱ��

        if (timeLeft <= 0) {
            this->finishUpgrade(); // ʱ�䵽�ˣ���ɣ�
        }
    }

    // ��������ʱ
    if (type == BuildingType::MINE || type == BuildingType::WATER) {
        if (state == BuildingState::PRODUCING) {
            productionTimeLeft -= dt;

            if (productionTimeLeft <= 0) {
                finishProduction();
            }
        }
    }

}
// ����������ʱ (���磺�ȼ� * 5��)
int Building::getUpgradeTime() {
    return a_level * 5; // 1����2��Ҫ5�룬������
}

// ���������Ҫ�ı�ʯ (���磺1�� = 1��ʯ�����߹̶� 5 ��ʯ)
int Building::getSpeedUpCost() {
    return std::ceil(timeLeft / 60); // �򵥴ֱ���ʣ�¼���ͼ�����ʯ
}


float Building::getTimeLeft()
{
    // ����ʣ�µ�ʱ��
    // ���С��0���ͷ���0����ֹ��ʾ����
    return (timeLeft > 0) ? timeLeft : 0;
}

// ��ʼ����
void Building::startProduction()
{
    // ����Ƿ���������
    if (state == BuildingState::UPGRADING) {
        log("%s is upgrading, cannot start production!", buildingName.c_str());
        return;
    }

    // ����Ƿ��Ѿ������ռ�
    if (state == BuildingState::READY) {
        log("%s has resources ready to collect!", buildingName.c_str());
        return;
    }

    state = BuildingState::PRODUCING;
    productionTimeLeft = 5.0f; // 5����������
    isReadyToCollect = false;

    // ���ݵȼ�������������ÿ������50��Դ
    productionAmount = 50 * a_level;

    // �Ƴ����ռ���ʾ
    if (readyIndicator) {
        readyIndicator->removeFromParent();
        readyIndicator = nullptr;
    }

    log("%s started production, time left: %f, amount: %d",
        buildingName.c_str(), productionTimeLeft, productionAmount);
}

// �������
void Building::finishProduction()
{
    state = BuildingState::READY;
    isReadyToCollect = true;

    // ��ӿ��ռ���ʾ
    if (!readyIndicator) {
        readyIndicator = Sprite::create("ui/ready_indicator.png");
        if (!readyIndicator) {
            readyIndicator = Sprite::create();
            auto label = Label::createWithTTF("!", "fonts/Marker Felt.ttf", 72);
            label->setColor(Color3B::RED);
            label->setPosition(Vec2(15, 50));
            readyIndicator->addChild(label);
            readyIndicator->setContentSize(Size(30, 30));
        }
        readyIndicator->setPosition(Vec2(this->getContentSize().width / 2,
            this->getContentSize().height + 20));
        this->addChild(readyIndicator, 100);
    }

    log("%s production ready! Collect %d resources.",
        buildingName.c_str(), productionAmount);
}

// �ռ���Դ
void Building::collectResources()
{
    if (state == BuildingState::READY && isReadyToCollect) {
        if (type == BuildingType::MINE) {
            coin_count += productionAmount;
            if (coin_count > coin_limit) coin_count = coin_limit;
            log("Collected %d gold from Gold Mine. Total: %d",
                productionAmount, coin_count);
        }
        else if (type == BuildingType::WATER) {
            water_count += productionAmount;
            if (water_count > water_limit) water_count = water_limit;
            log("Collected %d water from Water Collector. Total: %d",
                productionAmount, water_count);
        }

        // ����UI��ʾ
        auto gamescene = dynamic_cast<GameScene*>(Director::getInstance()->getRunningScene());
        if (gamescene) {
            gamescene->updateResourceDisplay();
        }

        // �ռ���ص�IDLE״̬�������ѡ����һ���ж�
        state = BuildingState::IDLE;
        isReadyToCollect = false;

        // �Ƴ����ռ���ʾ
        if (readyIndicator) {
            readyIndicator->removeFromParent();
            readyIndicator = nullptr;
        }

        log("%s resources collected, now in IDLE state.", buildingName.c_str());
    }
}

// ��ȡ����ʣ��ʱ��
float Building::getProductionTimeLeft() {
    return productionTimeLeft;
}

// ��ȡ���ռ�����Դ��
int Building::getProducedAmount() {
    return productionAmount;
}

// ��ʼ���� (ֻ��Ǯ�����ӵȼ�)
// ����ȫ�ֱ��� (����Ҫ�������ʵ�ʱ������޸�����)
extern int coin_count;
extern int water_count;

void Building::startUpgrade()
{
    // 1. ������飺����Ѿ����������������ˣ��Ͳ�������
    if (state != BuildingState::IDLE) return;

    // ������������Ӫ��ߵȼ����ƣ�10����
    if (type == BuildingType::BASE && a_level >= 10) {
        log("Town Hall has reached maximum level (10)!");

        // ��ʾ��ʾ��Ϣ
        auto scene = Director::getInstance()->getRunningScene();
        auto visibleSize = Director::getInstance()->getVisibleSize();
        auto label = Label::createWithTTF("Town Hall is at max level!", "fonts/Marker Felt.ttf", 28);
        label->setPosition(visibleSize.width / 2, visibleSize.height / 2);
        label->setColor(Color3B::ORANGE);
        scene->addChild(label, 1000);
        label->runAction(Sequence::create(
            DelayTime::create(2.0f),
            RemoveSelf::create(),
            nullptr
        ));
        return;
    }

    // ����������齨����ߵȼ����ƣ���Ӫ���10�����������ط������
    int nextLevel = a_level + 1;

    // ��ȡ��ǰ��Ӫ�ȼ�
    int townHallLevel = 1;
    for (auto& building : g_allPurchasedBuildings) {
        if (building && building->getType() == BuildingType::BASE) {
            townHallLevel = building->getLevel();
            break;
        }
    }

    // ����Ƿ������������һ��
    auto upgradeLimits = BuildingUpgradeLimits::getInstance();
    int maxLevelForThisTH = upgradeLimits->getMaxLevelForBuilding(type, townHallLevel);

    if (nextLevel > maxLevelForThisTH&& type != BuildingType::BASE) {
        log("Cannot upgrade %s to level %d: Maximum level for TH%d is %d",
            buildingName.c_str(), nextLevel, townHallLevel, maxLevelForThisTH);

        // ��ʾ��ʾ��Ϣ
        auto scene = Director::getInstance()->getRunningScene();
        auto visibleSize = Director::getInstance()->getVisibleSize();

        std::string message = buildingName + " cannot be upgraded beyond level " +
            std::to_string(maxLevelForThisTH) +
            " at Town Hall level " + std::to_string(townHallLevel);

        auto label = Label::createWithTTF(message, "fonts/Marker Felt.ttf", 28);
        label->setPosition(visibleSize.width / 2, visibleSize.height / 2);
        label->setColor(Color3B::ORANGE);
        scene->addChild(label, 1000);
        label->runAction(Sequence::create(
            DelayTime::create(2.5f),
            RemoveSelf::create(),
            nullptr
        ));

        // ����������ʾ��Ӫ������ʾ
        if (townHallLevel < 10) {
            std::string unlockInfo = upgradeLimits->getUnlockInfoForNextTownHallLevel(townHallLevel);
            auto infoLabel = Label::createWithTTF(unlockInfo, "fonts/Marker Felt.ttf", 24);
            infoLabel->setPosition(visibleSize.width / 2, visibleSize.height / 2 - 50);
            infoLabel->setColor(Color3B::YELLOW);
            infoLabel->setDimensions(400, 0);
            infoLabel->setAlignment(TextHAlignment::CENTER);
            scene->addChild(infoLabel, 1000);
            infoLabel->runAction(Sequence::create(
                DelayTime::create(3.0f),
                RemoveSelf::create(),
                nullptr
            ));
        }

        return;
    }

    // 2. ���㱾��������Ҫ�Ļ���
    // (��������һ�� getNextLevelCost �������㻨��)
    int cost = this->getNextLevelCost();

    // 3. �������޸������ݽ������Ϳ۳���Ӧ����Դ
    // ���裺��Ӫ�ͷ���������ң���Ӫ��ˮ
    bool isEnough = false;

    if (type == BuildingType::BARRACKS) {
        // ��Ӫ����ˮ
        if (water_count >= cost) {
            water_count -= cost; // �۷�
            isEnough = true;
            log("Spent %d Water for upgrade.", cost);
        }
        else {
            log("Not enough Water!");
        }
    }
    else {
        // ����(��Ӫ/����)���۽��
        if (coin_count >= cost) {
            coin_count -= cost; // �۷�
            isEnough = true;
            log("Spent %d Coin for upgrade.", cost);
        }
        else {
            log("Not enough Coin!");
        }
    }

    // 4. ֻ��Ǯ���ˣ��ſ�ʼ��������ʱ
    if (isEnough) {
        state = BuildingState::UPGRADING;

        // ����ʱ�� (���� ����ʱ�� * �ȼ�)
        timeLeft = 5.0f * a_level;

        log("Upgrade started... Time left: %f", timeLeft);
    }
    else {
        // Ǯ���������������һ��������ʾ���߲���һ��������Ч
        log("Cannot upgrade: Insufficient resources.");
    }
}

// ��ʯ����
void Building::speedUp()
{
    int cost = getSpeedUpCost();
    if (gem_count >= cost) {
        gem_count -= cost;
        finishUpgrade(); // ˲�����
    }
    else {
        log("Not enough gems!");
    }
}
// Building.cpp

// Building.cpp

void Building::initTouchListener()
{
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);

    // --- ������ʼ (������) ---
    listener->onTouchBegan = [=](Touch* touch, Event* event) {
        Vec2 touchPos = touch->getLocation();
        Vec2 nodePos = this->getParent()->convertToNodeSpace(touchPos);

        if (this->getBoundingBox().containsPoint(nodePos))
        {
            // 1. ��¼����
            touchOffset = this->getPosition() - nodePos;
            originalPos = this->getPosition();
            isDragging = false;

            // 2. �Ӿ����������
            this->setScale(0.55f);

            // 3. ���ؼ���������ʱ���Ƴ��ػ���
            // ����ԭ����λ�þͻ�¶���ݵأ����������ǻָ���ԭɫ
            this->removeGroundEffect();

            return true;
        }
        return false;
        };

    // --- �����ƶ� (���ֲ���) ---
    listener->onTouchMoved = [=](Touch* touch, Event* event) {
        if (touch->getStartLocation().distance(touch->getLocation()) > 10.0f) {
            isDragging = true;
        }
        Vec2 worldNewPos = touch->getLocation() + touchOffset;
        Vec2 nodePos = this->getParent()->convertToNodeSpace(worldNewPos);
        this->setPosition(nodePos);
        };

    // --- �������� (���½���) ---
    listener->onTouchEnded = [=](Touch* touch, Event* event) {
        this->setScale(0.5f); // �ָ���С

        if (isDragging) {
            auto gameScene = dynamic_cast<GameScene*>(Director::getInstance()->getRunningScene());

            if (gameScene) {
                // ��ȡ��ײ���� (ʹ�� Local ����)
                Rect myLocalRect = this->getBoundingBox();
                // ��΢��С�ж���Χ�Ż��ָ�
                myLocalRect.origin.x += 10;
                myLocalRect.origin.y += 10;
                myLocalRect.size.width -= 20;
                myLocalRect.size.height -= 20;

                // �����ײ
                if (gameScene->checkCollision(myLocalRect, this)) {
                    // === ������ײ������ԭ�� ===
                    log("COLLISION! Back to origin.");

                    auto seq = Sequence::create(
                        MoveTo::create(0.1f, originalPos),
                        CallFunc::create([=]() {
                            // ����������ǿ�ƹ�λ
                            this->setPosition(originalPos);
                            this->setLocalZOrder(10000 - (int)originalPos.y);

                            // ���ؼ����ص�ԭλ�����´����ػ�
                            this->createGroundEffect();
                            }),
                        NULL
                    );
                    this->runAction(seq);
                }
                else {
                    // === ���óɹ� ===
                    log("Placed OK.");
                    originalPos = this->getPosition();
                    this->setLocalZOrder(10000 - (int)this->getPositionY());

                    // ���ؼ�������λ�ô����ػ���
                    // ������λ�õĵ���ͱ�ɫ��
                    this->createGroundEffect();
                }
            }
            isDragging = false;
        }
        else {
            // === ����¼� (������ק) ===
            // �����Ȼû��λ�ã���Ϊ�˱��գ�����Ϊ���Ӿ�һ���ԣ�Ҳ��������ˢһ�µػ�
            this->createGroundEffect();

            this->setLocalZOrder(10000 - (int)this->getPositionY());
            if (this->type == BuildingType::WALL)
            {
                log("Clicked on a Wall - No popup.");
                return; // ֱ�ӽ�������ִ�к���ĵ�������
            }
            // ������Ϣ����
            auto infoLayer = BuildingInfoLayer::create();
            infoLayer->setBuilding(this);
            Director::getInstance()->getRunningScene()->addChild(infoLayer, 999);
        }
        };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}
void Building::finishUpgrade()
{
    // 1. ֻ�������������ܽ���
    if (state != BuildingState::UPGRADING) return;

    // 2. ״̬��ؿ���
    state = BuildingState::IDLE;
    timeLeft = 0;

    // 3. �ȼ� +1
    a_level++;
    
    if (type == BuildingType::BARRACKS) {
        // ����Ҫ������ֱ������ army_limit�������� GameScene ���¼���
        auto gamescene = dynamic_cast<GameScene*>(Director::getInstance()->getRunningScene());
        if (gamescene) {
            gamescene->recalculateArmyLimit();
            CCLOG("=== Building: Barracks upgraded to level %d, army limit recalculated ===", a_level);
        }
    }
    // 4. �����ġ���������ûص���
    // ��ʱ��Ż�ִ�� GameScene ��д�� coin_limit += 1500 ����
    if (UpgradeCallback_coin) {
        UpgradeCallback_coin();
    }

    log("Upgrade finished! Level is now %d", a_level);
}


void Building::createGroundEffect()
{
    // 1. ����Ѿ�������Ч�����Ƴ�����ֹ����
    if (groundEffectNode) {
        groundEffectNode->removeFromParent();
        groundEffectNode = nullptr;
    }
    auto groundSprite = Sprite::create("map/dirt_patch.png");
    if (this->type == BuildingType::WALL)
        return;
    if (groundSprite) {
        // 3. ��ȡ�ߴ���Ϣ
        Size buildingSize = this->getContentSize(); // ������ԭʼ��С
        Size spriteSize = groundSprite->getContentSize(); // ͼƬ��ԭʼ��С

        // 4. ���ؼ����Զ�����ͼƬ����Ӧ��ͬ��С�Ľ���
        // ����������Ľ����Ǵ�Ӫ���Ǳ�Ӫ�����ŵػ�ͼ�����Զ������ײ�
        groundSprite->setScaleX(buildingSize.width / (spriteSize.width * 0.8));
        groundSprite->setScaleY(buildingSize.height / (spriteSize.height * 1.5));

        // 5. ����λ�ã�����
        // ��Ϊ�Ǽ��ڽ���(this)���ϵģ�����λ���ǽ��������ĵ�
        groundSprite->setPosition(Vec2(buildingSize.width / 2, buildingSize.height / 2 - 50));
        groundSprite->getTexture()->setAliasTexParameters();
        // 6. ��ӵ���������
        // ZOrder -1 ��֤��ʾ�ڽ���ͼƬ�ĵײ�
        this->addChild(groundSprite, -100);

        // 7. ����ָ��
        groundEffectNode = groundSprite;
    }
    else {
        log("Error: Failed to load ground effect image: map/dirt.png");
    }
}

void Building::removeGroundEffect()
{
    if (groundEffectNode) {
        groundEffectNode->removeFromParent();
        groundEffectNode = nullptr;
    }
}

// ��������ֱ�����õȼ�����������Դ�����ڼ��ش浵��
void Building::setLevelDirectly(int level)
{
    if (level < 1) level = 1;

    // ��Ӫ���10������
    if (type == BuildingType::BASE && level > 10) {
        level = 10;
        CCLOG("=== Building: Town Hall max level is 10, setting to 10 ===");
    }

    // ���õȼ�
    a_level = level;

    CCLOG("=== Building: %s level set to %d (no resource cost) ===", buildingName.c_str(), a_level);
}

// ��������ֱ������״̬
void Building::setStateDirectly(BuildingState newState)
{
    state = newState;

    // ����ǽ���ʥˮ�ռ�����״̬ΪREADY����ʾ���ռ����
    if ((type == BuildingType::MINE || type == BuildingType::WATER) && state == BuildingState::READY) {
        if (!readyIndicator) {
            readyIndicator = Sprite::create("ui/ready_indicator.png");
            if (!readyIndicator) {
                // ���ͼƬ�����ڣ�����һ���򵥵ĺ�ɫ��̾��
                readyIndicator = Sprite::create();
                auto label = Label::createWithTTF("!", "fonts/Marker Felt.ttf", 72);
                label->setColor(Color3B::RED);
                label->setPosition(Vec2(15, 50));
                readyIndicator->addChild(label);
                readyIndicator->setContentSize(Size(30, 30));
            }
            readyIndicator->setPosition(Vec2(this->getContentSize().width / 2, this->getContentSize().height + 20));
            this->addChild(readyIndicator, 100);
        }
    }
    else if (readyIndicator) {
        // ���״̬����READY���Ƴ����ռ����
        readyIndicator->removeFromParent();
        readyIndicator = nullptr;
    }

    CCLOG("=== Building: %s state set to %d ===", buildingName.c_str(), (int)state);
}

// ����������������ʣ��ʱ��
void Building::setUpgradeTimeLeft(float time)
{
    timeLeft = time;
    if (timeLeft > 0) {
        state = BuildingState::UPGRADING;
    }
    CCLOG("=== Building: %s upgrade time left set to %.2f ===", buildingName.c_str(), timeLeft);
}

// ����������������ʣ��ʱ��
void Building::setProductionTimeLeft(float time)
{
    productionTimeLeft = time;
    if (productionTimeLeft > 0) {
        state = BuildingState::PRODUCING;
    }
    CCLOG("=== Building: %s production time left set to %.2f ===", buildingName.c_str(), productionTimeLeft);
}

// �������������ݳ�ʼ�����������ڼ��ش浵��
void Building::initFromSaveData(int level, BuildingState savedState, float upgradeTimeLeft, float productionTimeLeft)
{
    // ���õȼ�
    setLevelDirectly(level);

    // ����״̬
    setStateDirectly(savedState);

    // ��������ʣ��ʱ��
    if (savedState == BuildingState::UPGRADING) {
        setUpgradeTimeLeft(upgradeTimeLeft);
    }

    // ��������ʣ��ʱ��
    if (type == BuildingType::MINE || type == BuildingType::WATER) {
        if (savedState == BuildingState::PRODUCING) {
            setProductionTimeLeft(productionTimeLeft);
        }
        else if (savedState == BuildingState::READY) {
            // ����ǿ��ռ�״̬��ȷ���п��ռ����
            finishProduction(); // ��ᴴ�����ռ����
        }
    }

    CCLOG("=== Building: %s initialized from save data - Level:%d, State:%d ===",
        buildingName.c_str(), level, (int)savedState);
}