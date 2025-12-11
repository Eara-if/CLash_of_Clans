#include "GameScene.h"
#include "HelloWorldScene.h"
#include "BuildingInfoLayer.h"
#include "BattleScene.h"
#include "Building.h" 
#include "ShopScene.h"
#include"SaveGame.h"
USING_NS_CC;
extern int coin_count = 5000;
extern int water_count = 5000;
extern int gem_count = 500;

extern int coin_limit = 5000;
extern int water_limit = 5000;
extern int gem_limit = 5000;

cocos2d::Vector<Building*> g_allPurchasedBuildings;// ������ȫ���ѹ���������

int army_limit = 10;
class resource
{
public:
    std::string filename = "";
    int count = 0;

    // ���ؼ����� 1��Ϊ��ʵ�ֶ�̬������Ĳ��������������һ��
    // ���ǿ��Ը���һ��Ĭ�ϲ����������ڻ���Ҳ���� Node*
    virtual void print(Node* parentNode) {}; // �麯������������д
    virtual void modify() {};
    resource(int data) :count(data) {};
    virtual ~resource() {}; // �����顿������������������麯��
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

        // ������֮ǰ���߼�
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

    void modify() override { // �����顿���� override �ؼ��֣��������������
        // �޸��߼�
    }

    // �������������������������д�ˣ���Ϊ����Ҳ�� Node*
    void print(Node* parentNode) override {

        // ������ 2���ں����ڲ���ȡ��Ļ�ߴ�
        auto visibleSize = Director::getInstance()->getVisibleSize();
        Vec2 origin = Director::getInstance()->getVisibleOrigin();

        filename = "06.png";


        Rect tileRect = this->calculateRect();

        _displaySprite = Sprite::create(filename, tileRect);

        // ��ȫ���
        if (_displaySprite) {
            _displaySprite->getTexture()->setAliasTexParameters();
            _displaySprite->setScale(6.0f); // ȷ�� scaleAmount �ڻ����ʼ����

            // ����λ��
            _displaySprite->setPosition(origin.x + visibleSize.width - 150, origin.y + visibleSize.height - 50);

            // ���ӵ����ڵ�
            parentNode->addChild(_displaySprite, 0);
        }
    }
    void refresh() {
        if (_displaySprite != nullptr) {
            // 1. ���¼��� Rect (��Ϊ coin_count �Ѿ�����)
            Rect newRect = this->calculateRect();

            // 2. �����ġ�ֻ�޸ľ������ʾ���򣬲������´�����
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

        // ������֮ǰ���߼�
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

    void modify() override { // �����顿���� override �ؼ��֣��������������
        // �޸��߼�
    }

    void print(Node* parentNode) override {

        // ������ 2���ں����ڲ���ȡ��Ļ�ߴ�
        auto visibleSize = Director::getInstance()->getVisibleSize();
        Vec2 origin = Director::getInstance()->getVisibleOrigin();

        filename = "06.png";


        Rect tileRect = this->calculateRect();

        _displaySprite = Sprite::create(filename, tileRect);

        // ��ȫ���
        if (_displaySprite) {
            _displaySprite->getTexture()->setAliasTexParameters();
            _displaySprite->setScale(6.0f); // ȷ�� scaleAmount �ڻ����ʼ����

            // ����λ��
            _displaySprite->setPosition(origin.x + visibleSize.width - 150, origin.y + visibleSize.height - 120);

            // ���ӵ����ڵ�
            parentNode->addChild(_displaySprite, 0);
        }
    }
    void refresh() {
        if (_displaySprite != nullptr) {
            // 1. ���¼��� Rect (��Ϊ coin_count �Ѿ�����)
            Rect newRect = this->calculateRect();

            // 2. �����ġ�ֻ�޸ľ������ʾ���򣬲������´�����
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

        // ������֮ǰ���߼�
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

        // ������ 2���ں����ڲ���ȡ��Ļ�ߴ�
        auto visibleSize = Director::getInstance()->getVisibleSize();
        Vec2 origin = Director::getInstance()->getVisibleOrigin();

        filename = "06.png";


        Rect tileRect = this->calculateRect();

        _displaySprite = Sprite::create(filename, tileRect);

        // ��ȫ���
        if (_displaySprite) {
            _displaySprite->getTexture()->setAliasTexParameters();
            _displaySprite->setScale(6.0f); // ȷ�� scaleAmount �ڻ����ʼ����

            // ����λ��
            _displaySprite->setPosition(origin.x + visibleSize.width - 150, origin.y + visibleSize.height - 182);

            // ���ӵ����ڵ�
            parentNode->addChild(_displaySprite, 0);
        }
    }
    void refresh() {
        if (_displaySprite != nullptr) {
            // 1. ���¼��� Rect (��Ϊ coin_count �Ѿ�����)
            Rect newRect = this->calculateRect();

            // 2. �����ġ�ֻ�޸ľ������ʾ���򣬲������´�����
            _displaySprite->setTextureRect(newRect);

            // log("Coin icon refreshed!"); 
        }
    }

    void update() {
    }
};

Scene* GameScene::createScene(Building* purchasedBuilding) {
    auto scene = GameScene::create(); // ���� GameScene ʵ��

    // ���ؼ��޸ġ����ٽ��������ӵ��³��������Ǳ��浽ȫ������
    if (scene && purchasedBuilding) {
        // ȷ����������ȷ����
        purchasedBuilding->retain();
        g_allPurchasedBuildings.pushBack(purchasedBuilding);
        purchasedBuilding->release(); // GameScene���ڳ�����
    }
    return scene;
}

void GameScene::addPurchasedBuilding(Building* building) {
    if (!building) return;

    // ---------------------------------------------------------
    // 1. ȫ���������� (������ԭ�е��߼�)
    // ---------------------------------------------------------
    if (g_allPurchasedBuildings.contains(building)) {
        if (building->getParent()) {
            building->removeFromParent();
        }
    }
    else {
        g_allPurchasedBuildings.pushBack(building);
    }

    // ���뵱ǰ�����Ĺ����б�
    if (!_allBuildings.contains(building)) {
        _allBuildings.pushBack(building);
    }

    // ---------------------------------------------------------
    // 2. �������޸ġ�����Ѱ����ŵ�
    // ---------------------------------------------------------

    // A. �趨����Ŀ��㣺��Ļ����
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 screenCenter = Vec2(visibleSize.width / 2, visibleSize.height / 2);

    // B. ����Ļ����ת��Ϊ��ͼ�ڲ����� (�ǳ���Ҫ������λ�û�ƫ)
    // �������� _tiledMap ��Ա����
    Vec2 targetMapPos = _tiledMap->convertToNodeSpace(screenCenter);

    // C. ��������Ŀ���λ�� (��ֹ�ص�)
    Vec2 finalPos = this->getNearestFreePosition(building, targetMapPos);

    // D. ����Ƿ��ͼȫ����
    if (finalPos.equals(Vec2::ZERO)) {
        log("Error: Map is full! Cannot place purchased building.");
        // ������ԼӸ�������ʾ��ҡ�û�ط����ˡ�
        return;
    }

    // ---------------------------------------------------------
    // 3. �������޸ġ����ӵ���ͼ����������
    // ---------------------------------------------------------

    // ����λ�� (ʹ��������Ŀ�λ)
    building->setPosition(finalPos);

    // ���ؼ������ӵ� _tiledMap ������ this
    // �������ܺ���ľ������ȷ�� Y ���ڵ�����
    _tiledMap->addChild(building);

    // ���ò㼶 (YԽ��ZԽС��ʵ���ڵ�)
    building->setLocalZOrder(10000 - (int)finalPos.y);

    // ���ؼ���ע��Ϊ�ϰ��
    // �����´�����������קʱ����֪������������
    this->addObstacle(building);

    building->setOnUpgradeCallback([=]() {

        if (building->getType() == BuildingType::BASE) {


        }

        this->updateResourceDisplay();
        });
}

//
//   
//    auto startBattleItem = MenuItemFont::create(
//        "Ready To Fight",
//        CC_CALLBACK_1(GameScene::menuGotoBattleCallback, this)
//    );
//
//    startBattleItem->setFontNameObj("fonts/Marker Felt.ttf");
//    startBattleItem->setFontSize(48);
//
//    startBattleItem->setColor(Color3B::WHITE);
//
//    startBattleItem->setPosition(
//        Vec2(origin.x + visibleSize.width / 2, origin.y + 200)
//    );
//
//    auto battleMenu = Menu::create(startBattleItem, NULL);
//    battleMenu->setPosition(Vec2::ZERO);
//
//    // Z-Order:
//    this->addChild(battleMenu, 10); 
//
//}

void GameScene::menuGotoBattleCallback(Ref* pSender)
{
    log("Starting Battle...");
    auto scene = BattleScene::createScene();

    Director::getInstance()->pushScene(TransitionFade::create(0.5f, scene));
}
bool GameScene::init()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    _tiledMap = TMXTiledMap::create("Grass.tmx");
    if (_tiledMap) {
        _tiledMap->setAnchorPoint(Vec2::ZERO);
        _tiledMap->setPosition(Vec2::ZERO);

        _tiledMap->setScale(1.0f);

        this->addChild(_tiledMap, -99);
    }
    else {
        log("Error: Failed to load map! Check filename and Resources folder.");
    }
    auto objectGroup = _tiledMap->getObjectGroup("Objects");
    //检测地图中对象层的树
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

                    sprite->setPosition(x, y);

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

    // 4. �����˵�����
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

    // 如果没有基地建筑，创建默认的
    if (!hasBaseBuilding) {
        Vec2 housePos = Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2);
        this->setbuilding("House.png", Rect::ZERO, "My House", 500, BuildingType::BASE, housePos);
        CCLOG("=== GameScene: Created default base building ===");
    }

    // 如果没有兵营建筑，创建默认的
    if (!hasBarracksBuilding) {
        Vec2 junyingPos = Vec2(origin.x + visibleSize.width / 2 + 200, origin.y + visibleSize.height / 2);
        this->setbuilding("junying.png", Rect::ZERO, "My junying", 200, BuildingType::BARRACKS, junyingPos);
        CCLOG("=== GameScene: Created default barracks building ===");
    }



    // 添加所有已购买的建筑
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
    this->addSaveButton(); // 【新增】调用保存游戏按钮布局函数
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
        // 2. ����Ǳ�Ӫ -> ���˿����� (�����и�ȫ�ֱ��� army_limit)
        else if (type1 == BuildingType::BARRACKS) {
            army_limit += 10;
            log("Army limit increased to %d", army_limit);
        }

        this->updateResourceDisplay();
        });

    // 4. ���ӵ�����
    this->addObstacle(building);
    _tiledMap->addChild(building, 1);
    _allBuildings.pushBack(building);

    // 【关键修改】添加到全局容器，这样保存时才能找到
    if (!g_allPurchasedBuildings.contains(building)) {
        building->retain();
        g_allPurchasedBuildings.pushBack(building);
        building->release();

        CCLOG("=== GameScene: Default building added to global container: %s (type: %d) ===",
            name.c_str(), (int)type1);
    }
}


void GameScene::menuBackCallback(Ref* pSender)
{
    log("Back button clicked!");
    // 1. ���� HelloWorld ����
    // Ҳ���ǻص�������Ǹ�����
    auto scene = HelloWorld::createScene();

    // 2. �л����� (��һ�� 0.5�� �ķ�ҳ��Ч)
    // ��������Ż��� TransitionSlideInL::create(...) ���Բ�ͬ��Ч��
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
}

void GameScene::addShopButton() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // ʹ��ͼƬ��ť���������ְ�ť
    auto shopNormal = Sprite::create("ui/shop_normal.png"); // ����Ҫ�������ͼƬ
    auto shopSelected = Sprite::create("ui/shop_selected.png"); // ����ʹ��ͬһ��ͼƬ

    // ���ͼƬ�����ڣ�����һ���򵥵İ�ť���
    if (!shopNormal) {
        // �����򵥵�Բ�ΰ�ť
        shopNormal = Sprite::create();
        shopNormal->setTextureRect(Rect(0, 0, 150, 80));
        shopNormal->setColor(Color3B(255, 200, 0)); // ��ɫ

        auto shopLabel = Label::createWithTTF("MARKET", "fonts/Marker Felt.ttf", 30);
        shopLabel->setPosition(Vec2(75, 40));
        shopLabel->setColor(Color3B::WHITE);
        shopNormal->addChild(shopLabel);
    }

    if (!shopSelected) {
        shopSelected = Sprite::create();
        shopSelected->setTextureRect(Rect(0, 0, 85, 85));
        shopSelected->setColor(Color3B(255, 150, 0)); // ����Ľ�ɫ
    }

    auto shopItem = MenuItemSprite::create(shopNormal, shopSelected,
        [](Ref* pSender) {
            // ʹ��pushScene������replaceScene
            auto scene = ShopScene::createScene();
            Director::getInstance()->pushScene(TransitionFade::create(0.5f, scene));
        }
    );


    // ���ð�ťλ�ã����Ͻǣ�
    float x = origin.x + 150; // ���λ��
    float y = origin.y + visibleSize.height - 100; // ����λ��

    shopItem->setPosition(Vec2(x, y));

    // �����˵�����
    auto shopMenu = Menu::create(shopItem, NULL);
    shopMenu->setPosition(Vec2::ZERO);
    this->addChild(shopMenu, 200); // ʹ�ø��ߵĲ㼶

}

// ���������������ѹ������ĺ���
void GameScene::addAllPurchasedBuildings() {
    auto visibleSize = Director::getInstance()->getVisibleSize();

    for (auto& building : g_allPurchasedBuildings) {
        if (building && !building->getParent()) {
            _tiledMap->addChild(building, 10);

            // ȷ��������һ��Ĭ��λ�ã������ǰλ����0,0��
            if (building->getPositionX() == 0 && building->getPositionY() == 0) {
                Vec2 center = Vec2(visibleSize.width / 2, visibleSize.height / 2);
                Vec2 pos = getNearestFreePosition(building, center);
                auto x = pos.x;
                auto y = pos.y;
                building->setPosition(x, y);
            }

            // �������������ص�
            building->setOnUpgradeCallback([=]() {
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
                });

            // ���ӵ���������
            _allBuildings.pushBack(building);
        }
    }
}

// ��������дonEnter������ȷ�����̳Ƿ���ʱ��������ȷ����
void GameScene::onEnter() {
    Scene::onEnter();

    // ���̳Ƿ���ʱ��������Դ��ʾ
    this->updateResourceDisplay();

    // ���̳Ƿ���ʱ������Ƿ����½�����Ҫ����
    for (auto& building : g_allPurchasedBuildings) {
        if (building && !building->getParent()) {
            _tiledMap->addChild(building, 10);

            auto visibleSize = Director::getInstance()->getVisibleSize();
            if (building->getPositionX() == 0 && building->getPositionY() == 0) {
                Vec2 center = Vec2(visibleSize.width / 2, visibleSize.height / 2);
                Vec2 pos = getNearestFreePosition(building, center);
                auto x = pos.x;
                auto y = pos.y;
                building->setPosition(x, y);
            }
            this->addObstacle(building);
            // 刷新UI
            building->setOnUpgradeCallback([=]() {
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
                });

            _allBuildings.pushBack(building);
        }
    }
}

// ��������Դ��ʾ���º���
void GameScene::updateResourceDisplay() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // ���½����ʾ
    if (_coinTextLabel) {
        std::string txt = "Coin " + std::to_string(coin_count) + "/" + std::to_string(coin_limit);
        _coinTextLabel->setString(txt);
    }
    if (myCoin) {
        myCoin->refresh();
    }

    // ����ˮ��ʾ
    if (_waterTextLabel) {
        std::string txt = "Water " + std::to_string(water_count) + "/" + std::to_string(water_limit);
        _waterTextLabel->setString(txt);
    }
    if (mywater) {
        mywater->refresh();
    }

    // ���±�ʯ��ʾ
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

// GameScene.cpp

Rect GameScene::getWorldBoundingBox(Node* node)
{
    // ����ڵ�ոձ�������û�и��ڵ㣬ֱ�ӷ��� Rect::ZERO ��ֹ����
    if (!node->getParent()) return Rect::ZERO;

    Rect rect = node->getBoundingBox();
    Vec2 worldOrigin = node->getParent()->convertToWorldSpace(rect.origin);
    Rect worldRect = Rect(worldOrigin.x, worldOrigin.y, rect.size.width, rect.size.height);
    return worldRect;
}

// GameScene.cpp

// �����԰桿��ײ���
bool GameScene::checkCollision(Rect targetRect, Node* ignoreNode)
{// ���ϸ�ģʽ������С��������϶
        // ֻҪ�Ӵ� 1 ���ض���ײ

    for (auto obstacle : _obstacles)
    {
        if (obstacle == ignoreNode) continue;

        // ����ȷ���ϰ�����Ч
        if (!obstacle->getParent()) continue;

        // ��ȡ�ϰ���ľ��԰�Χ��
        Rect obstacleRect = obstacle->getBoundingBox();

        // �������޸ġ�ֱ���ж��ཻ�������κΡ��ݴ���
        if (targetRect.intersectsRect(obstacleRect))
        {
            return true; // ײ��
        }
    }
    return false; // ��ȫ
}
Vec2 GameScene::getNearestFreePosition(Building* building, Vec2 targetMapPos)
{
    // ע�⣺���ﴫ����� targetMapPos (��ͼ����)������Ҫ����ת����������ȥ�����
    // ��Ȼ checkCollision �ĳ����������꣬����ҲҪ��ϡ�

    // 1. ��ȡ�����ߴ�
    Size size = building->getContentSize();
    float w = size.width * building->getScaleX();
    float h = size.height * building->getScaleY();

    // 2. ��������
    int step = (int)(w / 2);
    if (step < 10) step = 10;
    int maxRadius = 30;

    for (int r = 1; r <= maxRadius; r++) {
        for (int x = -r; x <= r; x++) {
            for (int y = -r; y <= r; y++) {
                if (std::abs(x) != r && std::abs(y) != r) continue;

                // ���㡾��ͼ����ϵ���µ��º�ѡ��
                Vec2 candidateMapPos = targetMapPos + Vec2(x * step, y * step);

                // ====================================================
                // �����ġ��������õġ����������
                // ====================================================

                // A. �ѵ�ͼ���� -> ת����������
                Vec2 candidateWorldPos = _tiledMap->convertToWorldSpace(candidateMapPos);

                // B. �����������
                Rect testWorldRect = Rect(
                    candidateWorldPos.x - w * 0.5f, // ����ê�� 0.5
                    candidateWorldPos.y - h * 0.5f,
                    w, h
                );

                // C. ���
                if (!checkCollision(testWorldRect, nullptr)) {
                    // �ҵ��ˣ�ֱ�ӷ��ء���ͼ���꡿(��Ϊ setPosition ��Ҫ��ͼ����)
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

    // 创建保存按钮
    auto saveLabel = Label::createWithTTF("SAVE GAME", "fonts/Marker Felt.ttf", 28);
    saveLabel->setColor(Color3B::GREEN);
    saveLabel->enableOutline(Color4B::BLACK, 2);

    auto saveItem = MenuItemLabel::create(saveLabel,
        CC_CALLBACK_1(GameScene::menuSaveGameCallback, this));

    // 设置按钮位置（右下角）
    float x = origin.x + visibleSize.width - 150;
    float y = origin.y + 100;

    saveItem->setPosition(Vec2(x, y));

    // 【新增】调试按钮
    auto debugLabel = Label::createWithTTF("DEBUG SAVE", "fonts/Marker Felt.ttf", 20);
    debugLabel->setColor(Color3B::YELLOW);

    auto debugItem = MenuItemLabel::create(debugLabel,
        [](Ref* pSender) {
            CCLOG("=== Debug Save System ===");
            SaveGame::getInstance()->debugWritablePath();

            // 尝试保存和加载测试数据
            auto saveGame = SaveGame::getInstance();
            bool saveResult = saveGame->saveGameState("test_save.json");
            CCLOG("Test save result: %s", saveResult ? "SUCCESS" : "FAILED");

            bool loadResult = saveGame->loadGameState("test_save.json");
            CCLOG("Test load result: %s", loadResult ? "SUCCESS" : "FAILED");
        });

    // 创建菜单容器
    auto saveMenu = Menu::create(saveItem, NULL);
    saveMenu->setPosition(Vec2::ZERO);
    this->addChild(saveMenu, 200);
}

void GameScene::menuSaveGameCallback(Ref* pSender) {
    log("Saving game...");

    // 调用SaveGame单例保存游戏
    SaveGame::getInstance()->saveGameState();
}