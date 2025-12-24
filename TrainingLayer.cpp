#include "TrainingLayer.h"
#include "DataManager.h" // ?????????????
#include "BattleScene.h"

USING_NS_CC;
Label* TrainingLayer::showText(std::string content, float x, float y, cocos2d::Color4B color)
{
    auto label = Label::createWithTTF(content, "fonts/Marker Felt.ttf", 24);
    if (label) {
        label->setPosition(Vec2(x, y));
        label->setTextColor(color);
        this->addChild(label, 100);
    }
    // ???????????????? label ??????
    return label;
}
// TrainingLayer.cpp

bool TrainingLayer::init()
{
    if (!Layer::init()) return false;

    // 1. ���������β� (������ԭ�е��߼�)
    auto shield = LayerColor::create(Color4B(0, 0, 0, 180));
    this->addChild(shield);
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [](Touch*, Event*) { return true; };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 center = Vec2(visibleSize.width / 2, visibleSize.height / 2);

    auto bg = Sprite::create("popup_bg.png");
    bg->setPosition(center);
    bg->setScale(8.0f);
    this->addChild(bg);

    // 2. ��ȡ��ǰ�ؿ�����
    int currentProgress = DataManager::getInstance()->getMaxLevelUnlocked();

    // 3. �������ݶ���
    struct UnitData { std::string name; std::string img; bool locked; };
    std::vector<UnitData> units = {
        { "Soldier", "soldiers/man.png",   false },
        { "Arrow",   "soldiers/arrow.png", false },
        { "Boom",    "soldiers/boom.png",  false },
        { "Giant",   "soldiers/giant.png", false },
        { "Airforce","soldiers/Owl.png",   (currentProgress < 3) } // ����С��3������
    };

    // 4. �ٲ������ּ���
    int total = (int)units.size();
    int cols = 3;
    float hSpacing = 220.0f;
    float vSpacing = 180.0f;

    for (int i = 0; i < total; ++i) {
        int row = i / cols;
        int col = i % cols;
        int unitsInThisRow = (row == total / cols) ? (total % cols) : cols;
        if (unitsInThisRow == 0) unitsInThisRow = cols;

        float rowWidth = (unitsInThisRow - 1) * hSpacing;
        float startX = center.x - rowWidth / 2.0f;
        float startY = center.y + 110.0f;

        float x = startX + col * hSpacing;
        float y = startY - row * vSpacing;

        // �����޸ĺ�ĺ���
        this->createTroopCard(units[i].name, units[i].img, i, Vec2(x, y), units[i].locked);
    }

    // 5. �ײ���ť (Fight ��ť�߼�)
    float baseY = center.y - 180;
    auto closeLabel = Label::createWithTTF("Close", "fonts/Marker Felt.ttf", 28);
    auto closeBtn = MenuItemLabel::create(closeLabel, [=](Ref*) { this->closeLayer(); });
    closeBtn->setPosition(center.x - 220, baseY);

    auto menu = Menu::create(closeBtn, nullptr);

    // --- �޸ĺ�� Fight ��ť���� Lambda ---
    // ���� levelRequired: ����ùؿ���Ҫ����ͽ�������
    auto createFightBtn = [&](std::string text, Color3B color, float yOffset, std::string mapName, int levelRequired) {

        bool isLevelLocked = (currentProgress < levelRequired); // ���Ȳ���������

        auto lbl = Label::createWithTTF(text, "fonts/Marker Felt.ttf", 30);

        // ��������ˣ����ֱ��ɫ
        lbl->setColor(isLevelLocked ? Color3B::GRAY : color);
        lbl->enableOutline(Color4B::BLACK, 2);

        auto btn = MenuItemLabel::create(lbl, [=](Ref*) {
            // 1. ����Ƿ�����
            if (isLevelLocked) {
                auto warning = this->showText("Pass previous level first!", center.x, center.y - 50, Color4B::ORANGE);
                warning->runAction(Sequence::create(MoveBy::create(1.0f, Vec2(0, 30)), FadeOut::create(0.5f), RemoveSelf::create(), nullptr));
                return;
            }

            // 2. ����˿� (ԭ�߼�)
            if (DataManager::getInstance()->getTotalPopulation() <= 0) {
                auto warning = this->showText("Train troops first!", center.x, center.y, Color4B::RED);
                warning->runAction(Sequence::create(MoveBy::create(1.0f, Vec2(0, 50)), RemoveSelf::create(), nullptr));
                return;
            }

            // 【重要修复 1】：不要 removeFromParent()！
             // 我们可以直接 replaceScene，这样更彻底且不容易内存冲突

            // 【重要修复 2】：确保参数传递正确。如果是闯关模式，传 levelIndex 即可
            auto scene = BattleScene::createScene(levelRequired, "");

            // 【重要修复 3】：使用 replaceScene 替代 pushScene 
            // pushScene 会让两个场景同时占内存，手机端很容易崩
            Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
            });

        btn->setPosition(center.x + 80, baseY + yOffset);
        menu->addChild(btn);
        };

    // --- ���ø��ؿ��Ľ������� ---
    // ��һ�أ�Ĭ�Ͻ��� (���� >= 1)
    createFightBtn("FIGHT 1ST PASS", Color3B::YELLOW, 0, "Enemy_map1.tmx", 1);

    // �ڶ��أ���Ҫ�����һ�� (���� >= 2)
    createFightBtn("FIGHT 2ND PASS", Color3B::WHITE, -50, "Enemy_map2.tmx", 2);

    // �����أ���Ҫ����ڶ��� (���� >= 3)
    createFightBtn("FIGHT 3RD PASS", Color3B::ORANGE, -100, "Enemy_map3.tmx", 3);

    createFightBtn("FIGHT 4TH PASS", Color3B(250, 182, 169), -150, "Enemy_map4.tmx", 4);

    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 10);

    return true;
}

void TrainingLayer::createTroopCard(std::string name, std::string image, int index, Vec2 pos, bool isLocked)
{
    float x = pos.x;
    float y = pos.y;

    // ����������ȡ��ǰ�ؿ�����
    int currentProgress = DataManager::getInstance()->getMaxLevelUnlocked();

    // ���޸ġ���̬��������״̬������������ʹ�ô���Ĳ���
    // �ر��ǿվ���Ӧ�ø��ݵ�ǰ���ȶ�̬�ж�
    bool actuallyLocked = isLocked;

    // ����վ������ݹؿ����������ж�
    if (name == "Airforce") {
        actuallyLocked = (currentProgress < 3);
    }

    // 1. ����ͼƬ
    auto sprite = Sprite::create(image);
    if (sprite) {
        sprite->setPosition(x, y + 30);
        sprite->setScale(2.5f);
        if (isLocked) sprite->setColor(Color3B(80, 80, 80)); // ���
        this->addChild(sprite);
    }

    // 2. ����
    this->showText(name, x, y - 25, isLocked ? Color4B(120, 120, 120, 255) : Color4B::WHITE);

    // 3. ������ʾ
    std::string countStr = isLocked ? "Locked" : std::to_string(DataManager::getInstance()->getTroopCount(name));
    auto countLabel = Label::createWithTTF(countStr, "fonts/Marker Felt.ttf", 24);
    countLabel->setPosition(x, y - 60);
    countLabel->setTextColor(isLocked ? Color4B::GRAY : Color4B::GREEN);
    countLabel->setName(name);
    this->addChild(countLabel);
    if (!isLocked) _countLabels.push_back(countLabel);

    // 4. �Ӽ���ť
    auto menu = Menu::create();

    if (!isLocked) {
        auto minus = MenuItemLabel::create(Label::createWithTTF("-", "fonts/Marker Felt.ttf", 45), [=](Ref*) {
            if (DataManager::getInstance()->dismissTroop(name)) this->refreshLabels();
            });
        minus->setColor(Color3B::RED);
        minus->setPosition(x - 55, y - 60);
        menu->addChild(minus);
    }

    auto plus = MenuItemLabel::create(Label::createWithTTF("+", "fonts/Marker Felt.ttf", 45), [=](Ref*) {
        if (isLocked) {
            // ����ʱ����ʾ
            auto warning = this->showText("Unlocks at Map 3!", x, y + 60, Color4B::ORANGE);
            warning->runAction(Sequence::create(MoveBy::create(0.5, Vec2(0, 20)), FadeOut::create(0.3f), RemoveSelf::create(), NULL));
        }
        else {
            if (DataManager::getInstance()->trainTroop(name)) this->refreshLabels();
            else {
                auto warning = this->showText("Full!", x, y + 60, Color4B::RED);
                warning->runAction(Sequence::create(MoveBy::create(0.5, Vec2(0, 20)), RemoveSelf::create(), NULL));
            }
        }
        });
    plus->setColor(isLocked ? Color3B(100, 100, 100) : Color3B::YELLOW);
    plus->setPosition(x + 55, y - 60);
    menu->addChild(plus);

    menu->setPosition(Vec2::ZERO);
    this->addChild(menu);
}

void TrainingLayer::refreshLabels()
{
    for (auto label : _countLabels) {
        std::string name = label->getName(); // ?????????
        int count = DataManager::getInstance()->getTroopCount(name);
        label->setString(std::to_string(count));
    }
}

void TrainingLayer::closeLayer()
{
    this->removeFromParent();
}