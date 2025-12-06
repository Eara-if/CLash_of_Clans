#include "TrainingLayer.h"
#include "DataManager.h" // 引入数据管理器

USING_NS_CC;

bool TrainingLayer::init()
{
    if (!Layer::init()) return false;

    // 1. 半透明背景 (点击空白处关闭)
    auto shield = LayerColor::create(Color4B(0, 0, 0, 180));
    this->addChild(shield);

    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [=](Touch* t, Event* e) {
        // 简单处理：点任意地方都拦截，只有点关闭按钮才关
        return true;
        };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 center = Vec2(visibleSize.width / 2, visibleSize.height / 2);

    // 2. 弹窗底图
    auto bg = Sprite::create("popup_bg.png"); // 假设你有一张大点的背景图
    // 如果没有，用大一点的纯色块代替测试
    // bg->setTextureRect(Rect(0,0, 500, 300)); 
    bg->setPosition(center);
    this->addChild(bg);

    // 标题
    auto title = Label::createWithTTF("Training Camp", "fonts/Marker Felt.ttf", 32);
    title->setPosition(center.x, center.y + 120);
    this->addChild(title);

    // 3. 创建兵种卡片 (兵种名, 图片, 造价, 第几个, 起始位置)
    // 假设你有 soldier.png 和 archer.png
    createTroopCard("Soldier", "soldier.png", 100, 0, center);
    createTroopCard("Archer", "archer.png", 150, 1, center);

    // 4. 关闭按钮
    auto closeLabel = Label::createWithTTF("Close", "fonts/Marker Felt.ttf", 28);
    auto closeBtn = MenuItemLabel::create(closeLabel, [=](Ref*) {
        this->closeLayer();
        });
    closeBtn->setPosition(center.x, center.y - 120);

    auto menu = Menu::create(closeBtn, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu);

    return true;
}

void TrainingLayer::createTroopCard(std::string name, std::string image, int cost, int index, Vec2 centerPos)
{
    // 计算位置：横向排列
    float x = centerPos.x - 100 + (index * 200);
    float y = centerPos.y;

    // 1. 兵种图标 (作为按钮)
    // 如果没有图片，先用 MenuItemFont 代替测试
    auto btn = MenuItemImage::create(image, image, [=](Ref*) {

        // 【核心】点击训练：调用 DataManager
        bool success = DataManager::getInstance()->trainTroop(name, cost);

        if (success) {
            this->refreshLabels(); // 刷新界面数字
        }
        else {
            // 失败提示动画
            auto warning = Label::createWithSystemFont("Full/No Water!", "Arial", 24);
            warning->setPosition(x, y + 80);
            warning->setColor(Color3B::RED);
            this->addChild(warning);
            warning->runAction(Sequence::create(MoveBy::create(0.5, Vec2(0, 20)), RemoveSelf::create(), NULL));
        }
        });
    // 如果图片太大，缩放一下
    // btn->setScale(0.5f);

    auto menu = Menu::create(btn, NULL);
    menu->setPosition(x, y);
    this->addChild(menu);

    // 2. 名字和造价
    auto nameLabel = Label::createWithSystemFont(name, "Arial", 20);
    nameLabel->setPosition(x, y - 50);
    this->addChild(nameLabel);

    auto costLabel = Label::createWithSystemFont("Water: " + std::to_string(cost), "Arial", 18);
    costLabel->setPosition(x, y - 70);
    costLabel->setColor(Color3B::BLUE);
    this->addChild(costLabel);

    // 3. 当前拥有的数量 (需要动态刷新)
    int currentCount = DataManager::getInstance()->getTroopCount(name);
    auto countLabel = Label::createWithSystemFont("Owned: " + std::to_string(currentCount), "Arial", 22);
    countLabel->setPosition(x, y + 60);
    countLabel->setName(name); // 给 Label 起个名字，方便后面找
    this->addChild(countLabel);

    // 存起来方便刷新
    _countLabels.push_back(countLabel);
}

void TrainingLayer::refreshLabels()
{
    for (auto label : _countLabels) {
        std::string name = label->getName(); // 获取兵种名
        int count = DataManager::getInstance()->getTroopCount(name);
        label->setString("Owned: " + std::to_string(count));
    }
}

void TrainingLayer::closeLayer()
{
    this->removeFromParent();
}