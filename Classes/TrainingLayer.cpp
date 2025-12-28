#include "TrainingLayer.h"
#include "DataManager.h" 
#include "BattleScene.h"

USING_NS_CC;
//将设置文字的标签的功能封装成一个函数
Label* TrainingLayer::showText(std::string content, float x, float y, cocos2d::Color4B color)
{
    auto label = Label::createWithTTF(content, "fonts/Marker Felt.ttf", 24);
    if (label) {
        label->setPosition(Vec2(x, y));
        label->setTextColor(color);
        this->addChild(label, 100);
    }
    return label;
}
//训练士兵界面的初始化函数
bool TrainingLayer::init()
{
    if (!Layer::init()) //同样调用一下父类函数
        return false;
    // 同样设置一个黑色封盖层，添加到层
    auto shield = LayerColor::create(Color4B(0, 0, 0, 180));
    this->addChild(shield);

    auto listener = EventListenerTouchOneByOne::create();
    // 吞噬触摸事件，避免在此界面误点击到下面的建筑
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [](Touch*, Event*) {   //接收这次点击
        return true; 
        };
    //设置优先级，本层先拿到点击事件
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    //获取中心位置
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 center = Vec2(visibleSize.width / 2, visibleSize.height / 2);

    //创建一个弹窗背景图
    auto bg = Sprite::create("popup_bg.png");
    bg->setPosition(center);
    bg->setScale(8.0f);
    this->addChild(bg);

    //获取当前打到第几关了
    int currentProgress = DataManager::getInstance()->getMaxLevelUnlocked();
    //创建士兵结构体 第三关才可以解锁天空兵种
    struct UnitData { 
        std::string name; 
        std::string img; 
        bool locked; 
    };
    std::vector<UnitData> units = {
        { "Soldier", "soldiers/man.png",   false },
        { "Arrow",   "soldiers/arrow.png", false },
        { "Boom",    "soldiers/boom.png",  false },
        { "Giant",   "soldiers/giant.png", false },
        { "Airforce","soldiers/Owl.png",   (currentProgress < 3) } 
    };
    //获取一些信息，实现兵种图片的排版
    int total = (int)units.size();
    int cols = 3;
    float hSpacing = 220.0f;
    float vSpacing = 180.0f;

    for (int i = 0; i < total; ++i) {
        int row = i / cols;
        int col = i % cols;
        int unitsInThisRow = (row == total / cols) ? (total % cols) : cols;
        if (unitsInThisRow == 0) 
            unitsInThisRow = cols;

        float rowWidth = (unitsInThisRow - 1) * hSpacing;
        float startX = center.x - rowWidth / 2.0f;
        float startY = center.y + 110.0f;

        float x = startX + col * hSpacing;
        float y = startY - row * vSpacing;
        //根据上述计算出的数据创建士兵卡片
        this->createTroopCard(units[i].name, units[i].img, i, Vec2(x, y), units[i].locked);
    }
    //创建一个关闭界面的按钮
    float baseY = center.y - 180;
    auto closeLabel = Label::createWithTTF("Close", "fonts/Marker Felt.ttf", 28);
    auto closeBtn = MenuItemLabel::create(closeLabel, [=](Ref*) { this->closeLayer(); });
    closeBtn->setPosition(center.x - 220, baseY);
    //添加到menu上面
    auto menu = Menu::create(closeBtn, nullptr);

    
    auto createFightBtn = [&](std::string text, Color3B color, float yOffset, std::string mapName, int levelRequired) {
        //判断是否通过了前一关，不通过这关锁住
        bool isLevelLocked = (currentProgress < levelRequired); 

        auto lbl = Label::createWithTTF(text, "fonts/Marker Felt.ttf", 30);
        //锁住了设置成灰色
        lbl->setColor(isLevelLocked ? Color3B::GRAY : color);
        lbl->enableOutline(Color4B::BLACK, 2);

        auto btn = MenuItemLabel::create(lbl, [=](Ref*) {
            //如果点击锁住的关卡，设置一个流动的警告，并打断
            if (isLevelLocked) {
                auto warning = this->showText("Pass previous level first!", center.x, center.y - 50, Color4B::ORANGE);
                warning->runAction(Sequence::create(MoveBy::create(1.0f, Vec2(0, 30)), FadeOut::create(0.5f), RemoveSelf::create(), nullptr));
                return;
            }
            //如果玩家还没有训练，提示先训练士兵，然后打断
            if (DataManager::getInstance()->getTotalPopulation() <= 0) {
                auto warning = this->showText("Train troops first!", center.x, center.y, Color4B::RED);
                warning->runAction(Sequence::create(MoveBy::create(1.0f, Vec2(0, 50)), RemoveSelf::create(), nullptr));
                return;
            }

            // 现在可以进入闯关地图中了
            auto scene = BattleScene::createScene(levelRequired, "");
            // 用replace代替push,减少内存开销
            Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
            });
        //设置位置
        btn->setPosition(center.x + 80, baseY + yOffset);
        menu->addChild(btn);
        };

    //设置四个关卡
    createFightBtn("FIGHT 1ST PASS", Color3B::YELLOW, 0, "Enemy_map1.tmx", 1);
    createFightBtn("FIGHT 2ND PASS", Color3B::WHITE, -50, "Enemy_map2.tmx", 2);
    createFightBtn("FIGHT 3RD PASS", Color3B::ORANGE, -100, "Enemy_map3.tmx", 3);
    createFightBtn("FIGHT 4TH PASS", Color3B(250, 182, 169), -150, "Enemy_map4.tmx", 4);
    //坐标归零
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 10);

    return true;
}
//创建士兵卡片
void TrainingLayer::createTroopCard(std::string name, std::string image, int index, Vec2 pos, bool isLocked)
{
    float x = pos.x;
    float y = pos.y;

    //再次确认当前已经解锁的关卡
    int currentProgress = DataManager::getInstance()->getMaxLevelUnlocked();
    bool actuallyLocked = isLocked;
    //确认锁住天空兵种
    if (name == "Airforce") {
        actuallyLocked = (currentProgress < 3);
    }

    //接收传入的图片创建卡片
    auto sprite = Sprite::create(image);
    if (sprite) {
        sprite->setPosition(x, y + 30);
        sprite->setScale(2.5f);
        if (isLocked) //如果被限制了，图片变成灰灰的
            sprite->setColor(Color3B(80, 80, 80)); 
        //绑定到界面上
        this->addChild(sprite);
    }
    //显示兵种的名字
    this->showText(name, x, y - 25, isLocked ? Color4B(120, 120, 120, 255) : Color4B::WHITE);

    //显示当前的数量或是锁住的文字
    std::string countStr = isLocked ? "Locked" : std::to_string(DataManager::getInstance()->getTroopCount(name));
    auto countLabel = Label::createWithTTF(countStr, "fonts/Marker Felt.ttf", 24);
   //放在士兵图片下面
    countLabel->setPosition(x, y - 60);
    countLabel->setTextColor(isLocked ? Color4B::GRAY : Color4B::GREEN);
    countLabel->setName(name);
    this->addChild(countLabel);
    if (!isLocked) //没锁就将这个兵种放入数组中进行管理
        countLabels.push_back(countLabel);

    // 新创建一个菜单，用来绑定加减按钮
    auto menu = Menu::create();
    //如果没锁，设置一个减少数量按钮
    if (!isLocked) {
        auto minus = MenuItemLabel::create(Label::createWithTTF("-", "fonts/Marker Felt.ttf", 45), [=](Ref*) {
            if (DataManager::getInstance()->dismissTroop(name)) 
                this->refreshLabels(); //刷新标签数量
            });
        minus->setColor(Color3B::RED);
        minus->setPosition(x - 55, y - 60);
        menu->addChild(minus);
    }
    //设置增加数量标签
    auto plus = MenuItemLabel::create(Label::createWithTTF("+", "fonts/Marker Felt.ttf", 45), [=](Ref*) {
        if (isLocked) {
            //锁住的话给出流动提示
            auto warning = this->showText("Unlocks at Map 3!", x, y + 60, Color4B::ORANGE);
            warning->runAction(Sequence::create(MoveBy::create(0.5, Vec2(0, 20)), FadeOut::create(0.3f), RemoveSelf::create(), NULL));
        }
        else {
            if (DataManager::getInstance()->trainTroop(name)) 
                this->refreshLabels(); //如果还没到上限，刷新标签
            else {  //到上限了，给出数量已经满了的提示
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
    //遍历之前存储的士兵数量数组
    for (auto label : countLabels) {
        //识别身份
        std::string name = label->getName(); 
        //从DataMAnager中获取name的数量
        int count = DataManager::getInstance()->getTroopCount(name);
        //装换成字符串
        label->setString(std::to_string(count));
    }
}
//关闭这个训练士兵界面
void TrainingLayer::closeLayer()
{
    this->removeFromParent();
}