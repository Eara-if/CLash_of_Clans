#include "TrainingLayer.h"
#include "DataManager.h" // 引入数据管理器
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
    // 【新增】把创建好的 label 返回出去
    return label;
}
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
    bg->setScale(8.0f);

    // 标题
    auto title = Label::createWithTTF("Training Camp", "fonts/Marker Felt.ttf", 32);
    title->setPosition(center.x, center.y + 200);
    this->addChild(title);

    // 3. 创建兵种卡片 (兵种名, 图片, 造价, 第几个, 起始位置)
    // 假设你有 soldier.png 和 archer.png
    createTroopCard("Soldier", "soldiers/man.png", 0, center);
    createTroopCard("Arrow", "soldiers/arrow.png", 1, center);
    createTroopCard("Boom", "soldiers/boom.png", 2, center);
    createTroopCard("Giant", "soldiers/giant.png", 3, center);


    // ============================================================
    // 【新增/修改】 底部按钮区域
    // ============================================================

    // 1. 关闭按钮 (Close) - 稍微往左挪一点，或者放左下角
    auto closeLabel = Label::createWithTTF("Close", "fonts/Marker Felt.ttf", 28);
    auto closeBtn = MenuItemLabel::create(closeLabel, [=](Ref*) {
        this->closeLayer();
        });
    // 放在底部偏左
    closeBtn->setPosition(center.x - 100, center.y - 180);

    // 2. 【核心新增】战斗按钮 (Ready to Fight!) - 放底部偏右，显眼一点
    auto fightLabel = Label::createWithTTF("FIGHT FIRST PASS!", "fonts/Marker Felt.ttf", 36);
    fightLabel->setColor(Color3B::YELLOW); // 金色文字
    fightLabel->enableOutline(Color4B::BLACK, 2); // 加个黑边更霸气

    auto fightBtn = MenuItemLabel::create(fightLabel, [=](Ref*) {

        // A. 检查一下有没有兵 (可选优化)
        if (DataManager::getInstance()->getTotalPopulation() <= 0) {
            auto warning = this->showText("Train troops first!", center.x, center.y, Color4B::RED);
            warning->runAction(Sequence::create(MoveBy::create(1.0f, Vec2(0, 50)), RemoveSelf::create(), nullptr));
            return;
        }

        // B. 关闭当前弹窗 (防止返回时弹窗还在)
        this->removeFromParent();

        // C. 跳转战斗场景
        // 使用 pushScene 可以在战斗结束后 popScene 回到 GameScene
        auto scene = BattleScene::createScene();
        Director::getInstance()->pushScene(TransitionFade::create(0.5f, scene));
        });

    // 放在底部偏右
    fightBtn->setPosition(center.x + 100, center.y - 180);


    // 3. 把两个按钮加到 Menu
    auto menu = Menu::create(closeBtn, fightBtn, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu);

    return true;

}

// TrainingLayer.cpp

void TrainingLayer::createTroopCard(std::string name, std::string image, int index, Vec2 centerPos)
{
    // 计算卡片位置
    float x = centerPos.x - 300 + (index * 200);
    float y = centerPos.y;

    // ============================================================
    // 1. 兵种图片 (现在只是展示，不再是按钮)
    // ============================================================
    auto sprite = Sprite::create(image);
    if (sprite) {
        sprite->setPosition(x, y + 20); // 稍微往上放一点
        this->addChild(sprite);
    }
    sprite->setScale(3.0f);
    // (如果图片太大，可以在这里 setScale)

    // ============================================================
    // 2. 兵种名字
    // ============================================================
    this->showText(name, x, y - 50, Color4B::WHITE);

    // ============================================================
    // 3. 数量显示 (放在加减按钮中间)
    // ============================================================
    int currentCount = DataManager::getInstance()->getTroopCount(name);
    auto countLabel = this->showText(std::to_string(currentCount), x, y - 90, Color4B::GREEN);
    countLabel->setName(name); // 方便后续刷新
    _countLabels.push_back(countLabel);

    // ============================================================
    // 4. 创建 [ - ] 和 [ + ] 按钮
    // ============================================================

    // --- 减号按钮 ---
    auto minusLabel = Label::createWithTTF("-", "fonts/Marker Felt.ttf", 40);
    minusLabel->setColor(Color3B::RED);
    auto minusBtn = MenuItemLabel::create(minusLabel, [=](Ref*) {

        // 调用减少逻辑
        bool success = DataManager::getInstance()->dismissTroop(name);
        if (success) {
            this->refreshLabels(); // 刷新数字
        }
        });
    minusBtn->setPosition(x - 60, y - 90); // 放在数字左边

    // --- 加号按钮 ---
    auto plusLabel = Label::createWithTTF("+", "fonts/Marker Felt.ttf", 40);
    plusLabel->setColor(Color3B::YELLOW);
    auto plusBtn = MenuItemLabel::create(plusLabel, [=](Ref*) {

        // 调用增加逻辑 (cost 传 0 即可，反正不扣钱)
        bool success = DataManager::getInstance()->trainTroop(name);
        if (success) {
            this->refreshLabels(); // 刷新数字
        }
        else {
            // 满了提示
            auto warning = this->showText("Full!", x, y + 80, Color4B::RED);
            warning->runAction(Sequence::create(MoveBy::create(0.5, Vec2(0, 20)), RemoveSelf::create(), NULL));
        }
        });
    plusBtn->setPosition(x + 60, y - 90); // 放在数字右边

    // ============================================================
    // 5. 添加到菜单
    // ============================================================
    auto menu = Menu::create(minusBtn, plusBtn, NULL);
    menu->setPosition(Vec2::ZERO); // 按钮位置已经设的是绝对坐标
    this->addChild(menu);
}

void TrainingLayer::refreshLabels()
{
    for (auto label : _countLabels) {
        std::string name = label->getName(); // 获取兵种名
        int count = DataManager::getInstance()->getTroopCount(name);
        label->setString(std::to_string(count));
    }
}

void TrainingLayer::closeLayer()
{
    this->removeFromParent();
}