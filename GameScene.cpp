#include "GameScene.h"
#include "HelloWorldScene.h"
#include "BuildingInfoLayer.h"
#include "BattleScene.h"
#include "Building.h" 
#include "ShopScene.h"
#include "SaveGame.h"
#include "AudioEngine.h"
USING_NS_CC;
extern int coin_count = 5000;
extern int water_count = 5000;
extern int gem_count = 500;

extern int coin_limit = 5000;
extern int water_limit = 5000;
extern int gem_limit = 5000;

cocos2d::Vector<Building*> g_allPurchasedBuildings;// ????????????????????

int army_limit = 0;
class resource
{
public:
    std::string filename = "";
    int count = 0;

    // ????????? 1??????????????????????????????????
    // ??????????????????????????????????? Node*
    virtual void print(Node* parentNode) {}; // ?�D????????????��
    virtual void modify() {};
    resource(int data) :count(data) {};
    virtual ~resource() {}; // ?????��??????????????????�D??
};

class goldcoin : public resource
{
private:
    cocos2d::Sprite* _displaySprite = nullptr;
    Rect calculateRect() {
        float start_x = 0;
        float start_y = 63;
        float tileWidth = 43;
        float tileHeight = 10;

        // ?????????????
        if (coin_count > 0 && coin_count <= coin_limit / 4) {
            start_x = 137;
        }
        else if (coin_count == 0) {
            start_x = 182;
        }
        else if (coin_count > coin_limit / 4 && coin_count <= coin_limit / 2) {
            start_x = 92;
        }
        else if (coin_count > coin_limit / 2 && coin_count <= coin_limit * 0.75) {
            start_x = 47;
        }
        else if (coin_count > coin_limit * 0.75 && coin_count <= coin_limit) {
            start_x = 2;
        }

        return Rect(start_x, start_y, tileWidth, tileHeight);
    }
public:

    goldcoin() : resource(5000) {};
    ~goldcoin() {};

    void modify() override { // ?????��???? override ???????????????????
        // ??????
    }

    // ?????????????????????????��????????????? Node*
    void print(Node* parentNode) override {

        // ?????? 2???????????????????
        auto visibleSize = Director::getInstance()->getVisibleSize();
        Vec2 origin = Director::getInstance()->getVisibleOrigin();

        filename = "ui/06.png";


        Rect tileRect = this->calculateRect();

        _displaySprite = Sprite::create(filename, tileRect);

        // ??????
        if (_displaySprite) {
            _displaySprite->getTexture()->setAliasTexParameters();
            _displaySprite->setScale(6.0f); // ??? scaleAmount ???????????

            // ????��??
            _displaySprite->setPosition(origin.x + visibleSize.width - 150, origin.y + visibleSize.height - 50);

            // ??????????
            parentNode->addChild(_displaySprite, 2);
        }
    }
    void refresh() {
        if (_displaySprite != nullptr) {
            // 1. ??????? Rect (??? coin_count ???????)
            Rect newRect = this->calculateRect();

            // 2. ???????????????????????????????????
            _displaySprite->setTextureRect(newRect);

            // log("Coin icon refreshed!"); 
        }
    }
    void update() {
    }
};
class water : public resource
{
private:
    cocos2d::Sprite* _displaySprite = nullptr;
    Rect calculateRect() {
        float start_x = 0;
        float start_y = 19;
        float tileWidth = 43;
        float tileHeight = 10;

        // ?????????????
        if (water_count > 0 && water_count <= water_limit / 4) {
            start_x = 137;
        }
        else if (water_count == 0) {
            start_x = 182;
        }
        else if (water_count > water_limit / 4 && water_count <= water_limit / 2) {
            start_x = 92;
        }
        else if (water_count > water_limit / 2 && water_count <= water_limit * 0.75) {
            start_x = 47;
        }
        else if (water_count > water_limit * 0.75 && water_count <= water_limit) {
            start_x = 2;
        }

        return Rect(start_x, start_y, tileWidth, tileHeight);
    }
public:
    water() : resource(5000) {};
    ~water() {};

    void modify() override { // ?????��???? override ???????????????????
        // ??????
    }

    void print(Node* parentNode) override {

        // ?????? 2???????????????????
        auto visibleSize = Director::getInstance()->getVisibleSize();
        Vec2 origin = Director::getInstance()->getVisibleOrigin();

        filename = "ui/06.png";


        Rect tileRect = this->calculateRect();

        _displaySprite = Sprite::create(filename, tileRect);

        // ??????
        if (_displaySprite) {
            _displaySprite->getTexture()->setAliasTexParameters();
            _displaySprite->setScale(6.0f); // ??? scaleAmount ???????????

            // ????��??
            _displaySprite->setPosition(origin.x + visibleSize.width - 150, origin.y + visibleSize.height - 120);

            // ??????????
            parentNode->addChild(_displaySprite, 2);
        }
    }
    void refresh() {
        if (_displaySprite != nullptr) {
            // 1. ??????? Rect (??? coin_count ???????)
            Rect newRect = this->calculateRect();

            // 2. ???????????????????????????????????
            _displaySprite->setTextureRect(newRect);

            // log("Coin icon refreshed!"); 
        }
    }

    void update() {
    }
};
class Gem : public resource
{
private:
    cocos2d::Sprite* _displaySprite = nullptr;
    Rect calculateRect() {
        float start_x = 0;
        float start_y = 34;
        float tileWidth = 43;
        float tileHeight = 10;

        // ?????????????
        if (gem_count > 0 && gem_count <= gem_limit / 4) {
            start_x = 137;
        }
        else if (gem_count == 0) {
            start_x = 182;
        }
        else if (gem_count > gem_limit / 4 && gem_count <= gem_limit / 2) {
            start_x = 92;
        }
        else if (gem_count > gem_limit / 2 && gem_count <= gem_limit * 0.75) {
            start_x = 47;
        }
        else if (gem_count > gem_limit * 0.75 && gem_count <= gem_limit) {
            start_x = 2;
        }

        return Rect(start_x, start_y, tileWidth, tileHeight);
    }
public:
    Gem() : resource(5000) {};
    ~Gem() {};

    void modify() override {
    }

    
    void print(Node* parentNode) override {

        // ?????? 2???????????????????
        auto visibleSize = Director::getInstance()->getVisibleSize();
        Vec2 origin = Director::getInstance()->getVisibleOrigin();

        filename = "ui/06.png";


        Rect tileRect = this->calculateRect();

        _displaySprite = Sprite::create(filename, tileRect);

        // ??????
        if (_displaySprite) {
            _displaySprite->getTexture()->setAliasTexParameters();
            _displaySprite->setScale(6.0f); // ??? scaleAmount ???????????

            // ????��??
            _displaySprite->setPosition(origin.x + visibleSize.width - 150, origin.y + visibleSize.height - 182);

            // ??????????
            parentNode->addChild(_displaySprite, 2);
        }
    }
    void refresh() {
        if (_displaySprite != nullptr) {
            // 1. ??????? Rect (??? coin_count ???????)
            Rect newRect = this->calculateRect();

            // 2. ???????????????????????????????????
            _displaySprite->setTextureRect(newRect);

            // log("Coin icon refreshed!"); 
        }
    }

    void update() {
    }
};

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
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    _tiledMap = TMXTiledMap::create("Grass.tmx");
    if (!_tiledMap) {
        log("Error: Failed to load map!");
        return false;
    }
    this->addChild(_tiledMap, 1); // Z-Order ��Ϊ 1 (֮ǰ�޸�������ͻ�ĵ�)

    // ============================================================
    // �������޸� 1�������ʼ���ţ��õ�ͼ������ʾ����Ļ��
    // ============================================================
    Size mapSize = _tiledMap->getContentSize();

    // ���� X ��� Y ��ֱ���Ҫ�����ű���
    float scaleX = visibleSize.width / mapSize.width;
    float scaleY = visibleSize.height / mapSize.height;

    // ȡ���ߵĽ�Сֵ����֤��ͼ������ȫ������Ļ�����ᱻ�е�
    float minScale = std::min(scaleX, scaleY);

    // ���ó�ʼ����
    _tiledMap->setScale(minScale);

    // ��������һ�α߽����������������Զ������ź�ĵ�ͼ�����С���ʾ
    this->checkAndClampMapPosition();

    // ������������ӽ���֮ǰ�������¼����˿�����
    this->recalculateArmyLimit();

    // ============================================================
    // �������޸� 2�������ͼ�ġ��������ġ� (Local Coordinate)
    // ============================================================
    // ע�⣺��Ҫ����Ļ���ģ�Ҫ�õ�ͼ���ģ�
    Vec2 mapCenterLocal = Vec2(mapSize.width / 2, mapSize.height / 2);
    auto objectGroup = _tiledMap->getObjectGroup("Objects");
    //����ͼ�ж�������
    if (objectGroup) {
        ValueVector objects = objectGroup->getObjects();

        for (const auto& v : objects) {
            ValueMap dict = v.asValueMap();

            if (dict.find("fileName") != dict.end())
            {
                std::string path = dict["fileName"].asString();

                auto sprite = Sprite::create(path);

                if (sprite) {
                    _tiledMap->addChild(sprite, 1);

                    this->addObstacle(sprite);
                    sprite->setAnchorPoint(Vec2::ZERO);
                    sprite->getTexture()->setAliasTexParameters();

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
                else {
                    log("Error: Can't load image: %s", path.c_str());
                }
            }
        }
    }

    auto backLabel = Label::createWithTTF("Back", "fonts/Marker Felt.ttf", 36);
    backLabel->setTextColor(Color4B::YELLOW);

    auto backItem = MenuItemFont::create("Back", CC_CALLBACK_1(GameScene::menuBackCallback, this));

    backItem->setColor(Color3B::YELLOW);

    float x = origin.x + backItem->getContentSize().width / 2 + 20;
    float y = origin.y + backItem->getContentSize().height / 2 + 20;
    backItem->setPosition(Vec2(x, y));
    auto menu = Menu::create(backItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 100);


    bool hasBaseBuilding = false;
    bool hasBarracksBuilding = false;
    for (auto& building : g_allPurchasedBuildings) {
        if (building) {
            if (building->getType() == BuildingType::BASE) {
                hasBaseBuilding = true;
            }
            else if (building->getType() == BuildingType::BARRACKS) {
                hasBarracksBuilding = true;
            }
        }
    }

    // ���û�л��ؽ���������Ĭ�ϵ�
    if (!hasBaseBuilding) {
        // ʹ�� getNearestFreePosition Ѱ�����ĸ�������Ŀյ� (��ֹ���������п���)
        // ����� mapCenterLocal �ǵ�ͼ�ڲ����꣬����ֱ�Ӵ��� getNearestFreePosition (����֮ǰ�޹���������������� Local)
        // ע�⣺�������ǻ�û���� Building �����޷����� getNearestFreePosition ���С��
        // ���������ȼ򵥵ش� targetPos���� setbuilding �ڲ������ֶ�΢��

        // �򵥴����ֱ�ӳ��Է������ģ��Ժ� setbuilding �ᴦ�� ZOrder
        this->setbuilding("House.png", Rect::ZERO, "My House", 500, BuildingType::BASE, mapCenterLocal);
        CCLOG("Created default base at Map Center");
    }

    // ����Ĭ�ϱ�Ӫ (���ڵ�ͼ������΢ƫ��һ���λ��)
    if (!hasBarracksBuilding) {
        Vec2 barracksPos = mapCenterLocal + Vec2(200, 0); // ����ƫ�� 200 ����
        this->setbuilding("junying.png", Rect::ZERO, "My junying", 500, BuildingType::BARRACKS, barracksPos);
        CCLOG("Created default barracks near Map Center");
    }



    // ��������ѹ���Ľ���
    this->addAllPurchasedBuildings();

    myCoin = new goldcoin();
    myCoin->print(this);

    std::string txt = "Coin " + std::to_string(coin_count) + "/" + std::to_string(coin_limit);
    _coinTextLabel = this->showText(txt, origin.x + visibleSize.width - 370, origin.y + visibleSize.height - 50, Color4B::WHITE);

    mywater = new water();
    mywater->print(this);
    txt = "Water " + std::to_string(water_count) + "/" + std::to_string(water_limit);

    _waterTextLabel = this->showText(txt, origin.x + visibleSize.width - 370, origin.y + visibleSize.height - 120, Color4B::WHITE);

    mygem = new Gem();
    mygem->print(this);
    txt = "Gem " + std::to_string(gem_count) + "/" + std::to_string(gem_limit);
    _gemTextLabel = this->showText(txt, origin.x + visibleSize.width - 370, origin.y + visibleSize.height - 182, Color4B::WHITE);


    this->addShopButton();
    this->addSaveButton(); // �����������ñ�����Ϸ��ť���ֺ���

    isMapDragging = false;

    // 2. ��Ӵ������� (�����ƶ���ͼ)
    // ע�⣺����Ҫ�������������Ϊ�ϵ͵����ȼ��������������ɴ���
    // ����Ϊ����Ҫ�㽨�����������ﲻ���ɣ�����ͨ���߼��ж�
    auto touchListener = EventListenerTouchOneByOne::create();
    touchListener->setSwallowTouches(true); // ��Ϊtrue������ onTouchBegan ���ж�
    touchListener->onTouchBegan = CC_CALLBACK_2(GameScene::onTouchBegan, this);
    touchListener->onTouchMoved = CC_CALLBACK_2(GameScene::onTouchMoved, this);
    touchListener->onTouchEnded = CC_CALLBACK_2(GameScene::onTouchEnded, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, _tiledMap);

    // 3. ��������� (���ڹ�������)
    auto mouseListener = EventListenerMouse::create();
    mouseListener->onMouseScroll = CC_CALLBACK_1(GameScene::onMouseScroll, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);
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
    log("Back button clicked!");
    // 1. ???? HelloWorld ????
    // ???????????????????
    auto scene = HelloWorld::createScene();

    // 2. ?��????? (????? 0.5?? ??????��)
    // ???????????? TransitionSlideInL::create(...) ????????��??
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
}

void GameScene::addShopButton() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // ????????????????????
    auto shopNormal = Sprite::create("ui/shop_normal.png"); // ??????????????
    auto shopSelected = Sprite::create("ui/shop_selected.png"); // ?????????????

    // ????????????????????????????
    if (!shopNormal) {
        // ?????????????
        shopNormal = Sprite::create();
        shopNormal->setTextureRect(Rect(0, 0, 150, 80));
        shopNormal->setColor(Color3B(255, 200, 0)); // ???

        auto shopLabel = Label::createWithTTF("MARKET", "fonts/Marker Felt.ttf", 30);
        shopLabel->setPosition(Vec2(75, 40));
        shopLabel->setColor(Color3B::WHITE);
        shopNormal->addChild(shopLabel);
    }

    if (!shopSelected) {
        shopSelected = Sprite::create();
        shopSelected->setTextureRect(Rect(0, 0, 85, 85));
        shopSelected->setColor(Color3B(255, 150, 0)); // ???????
    }

    auto shopItem = MenuItemSprite::create(shopNormal, shopSelected,
        [](Ref* pSender) {
            // ???pushScene??????replaceScene
            auto scene = ShopScene::createScene();
            Director::getInstance()->pushScene(TransitionFade::create(0.5f, scene));
        }
    );


    // ??????��?????????
    float x = origin.x + 150; // ???��??
    float y = origin.y + visibleSize.height - 100; // ????��??

    shopItem->setPosition(Vec2(x, y));

    // ???????????
    auto shopMenu = Menu::create(shopItem, NULL);
    shopMenu->setPosition(Vec2::ZERO);
    this->addChild(shopMenu, 200); // ????????

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
                if (building->getType() == BuildingType::BARRACKS ) {
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
    Scene::onEnter();
    AudioEngine::stopAll();
    this->updateResourceDisplay();

    // ???????????????????????????????
    for (auto& building : g_allPurchasedBuildings) {
        if (building && !building->getParent()) {
            _tiledMap->addChild(building, 10);
            Size mapSize = _tiledMap->getContentSize();
            auto visibleSize = Director::getInstance()->getVisibleSize();
            if (building->getPositionX() == 0 && building->getPositionY() == 0) {

                Vec2 center = Vec2(mapSize.width / 2, mapSize.height / 2);
                Vec2 pos = getNearestFreePosition(building, center);
                auto x = pos.x;
                auto y = pos.y;
                building->setPosition(x, y);
            }
            this->addObstacle(building);
            // ˢ��UI
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
                this->updateResourceDisplay();
                });

            _allBuildings.pushBack(building);
        }
    }
    // �����������볡�������¼����˿�����
    this->recalculateArmyLimit();
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
    auto saveLabel = Label::createWithTTF("SAVE GAME", "fonts/Marker Felt.ttf", 28);
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