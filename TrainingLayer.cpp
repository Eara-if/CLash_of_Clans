#include "TrainingLayer.h"
#include "DataManager.h" // �������ݹ�����
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
    // ���������Ѵ����õ� label ���س�ȥ
    return label;
}
// TrainingLayer.cpp

bool TrainingLayer::init()
{
    if (!Layer::init()) return false;

    // 1. 背景和屏蔽层 (保持你原有的逻辑)
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

    // 2. 获取当前关卡进度
    int currentProgress = DataManager::getInstance()->getMaxLevelUnlocked();

    // 3. 兵种数据定义
    struct UnitData { std::string name; std::string img; bool locked; };
    std::vector<UnitData> units = {
        { "Soldier", "soldiers/man.png",   false },
        { "Arrow",   "soldiers/arrow.png", false },
        { "Boom",    "soldiers/boom.png",  false },
        { "Giant",   "soldiers/giant.png", false },
        { "Airforce","soldiers/Owl.png",   (currentProgress < 3) } // 进度小于3则锁定
    };

    // 4. 瀑布流布局计算
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

        // 调用修改后的函数
        this->createTroopCard(units[i].name, units[i].img, i, Vec2(x, y), units[i].locked);
    }

    // 5. 底部按钮 (Fight 按钮逻辑)
    float baseY = center.y - 180;
    auto closeLabel = Label::createWithTTF("Close", "fonts/Marker Felt.ttf", 28);
    auto closeBtn = MenuItemLabel::create(closeLabel, [=](Ref*) { this->closeLayer(); });
    closeBtn->setPosition(center.x - 220, baseY);

    auto menu = Menu::create(closeBtn, nullptr);

    // --- 修改后的 Fight 按钮创建 Lambda ---
    // 参数 levelRequired: 进入该关卡需要的最低解锁进度
    auto createFightBtn = [&](std::string text, Color3B color, float yOffset, std::string mapName, int levelRequired) {

        bool isLevelLocked = (currentProgress < levelRequired); // 进度不足则锁定

        auto lbl = Label::createWithTTF(text, "fonts/Marker Felt.ttf", 30);

        // 如果锁定了，文字变灰色
        lbl->setColor(isLevelLocked ? Color3B::GRAY : color);
        lbl->enableOutline(Color4B::BLACK, 2);

        auto btn = MenuItemLabel::create(lbl, [=](Ref*) {
            // 1. 检查是否锁定
            if (isLevelLocked) {
                auto warning = this->showText("Pass previous level first!", center.x, center.y - 50, Color4B::ORANGE);
                warning->runAction(Sequence::create(MoveBy::create(1.0f, Vec2(0, 30)), FadeOut::create(0.5f), RemoveSelf::create(), nullptr));
                return;
            }

            // 2. 检查人口 (原逻辑)
            if (DataManager::getInstance()->getTotalPopulation() <= 0) {
                auto warning = this->showText("Train troops first!", center.x, center.y, Color4B::RED);
                warning->runAction(Sequence::create(MoveBy::create(1.0f, Vec2(0, 50)), RemoveSelf::create(), nullptr));
                return;
            }

            // 3. 进入战斗 (原逻辑)
            this->removeFromParent();
            auto scene = BattleScene::createScene(mapName);
            Director::getInstance()->pushScene(TransitionFade::create(0.5f, scene));
            });

        btn->setPosition(center.x + 80, baseY + yOffset);
        menu->addChild(btn);
        };

    // --- 设置各关卡的解锁需求 ---
    // 第一关：默认解锁 (进度 >= 1)
    createFightBtn("FIGHT 1ST PASS", Color3B::YELLOW, 0, "Enemy_map1.tmx", 1);

    // 第二关：需要打过第一关 (进度 >= 2)
    createFightBtn("FIGHT 2ND PASS", Color3B::WHITE, -50, "Enemy_map2.tmx", 2);

    // 第三关：需要打过第二关 (进度 >= 3)
    createFightBtn("FIGHT 3RD PASS", Color3B::ORANGE, -100, "Enemy_map3.tmx", 3);

    createFightBtn("FIGHT 4TH PASS", Color3B(250,182,169), -150, "Enemy_map4.tmx", 1);

    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 10);

    return true;
}

void TrainingLayer::createTroopCard(std::string name, std::string image, int index, Vec2 pos, bool isLocked)
{
    float x = pos.x;
    float y = pos.y;

    // 1. 兵种图片
    auto sprite = Sprite::create(image);
    if (sprite) {
        sprite->setPosition(x, y + 30);
        sprite->setScale(2.5f);
        if (isLocked) sprite->setColor(Color3B(80, 80, 80)); // 变灰
        this->addChild(sprite);
    }

    // 2. 名字
    this->showText(name, x, y - 25, isLocked ? Color4B(120, 120, 120, 255) : Color4B::WHITE);

    // 3. 数量显示
    std::string countStr = isLocked ? "Locked" : std::to_string(DataManager::getInstance()->getTroopCount(name));
    auto countLabel = Label::createWithTTF(countStr, "fonts/Marker Felt.ttf", 24);
    countLabel->setPosition(x, y - 60);
    countLabel->setTextColor(isLocked ? Color4B::GRAY : Color4B::GREEN);
    countLabel->setName(name);
    this->addChild(countLabel);
    if (!isLocked) _countLabels.push_back(countLabel);

    // 4. 加减按钮
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
            // 锁定时的提示
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
        std::string name = label->getName(); // ��ȡ������
        int count = DataManager::getInstance()->getTroopCount(name);
        label->setString(std::to_string(count));
    }
}

void TrainingLayer::closeLayer()
{
    this->removeFromParent();
}