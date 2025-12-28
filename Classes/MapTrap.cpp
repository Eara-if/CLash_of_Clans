#include "MapTrap.h"

USING_NS_CC;

MapTrap* MapTrap::create(const Rect& area, int damage)
{
    MapTrap* ret = new (std::nothrow) MapTrap();//创建空间
    if (ret && ret->init(area, damage)) {
        ret->autorelease();       //自动管理
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool MapTrap::init(const Rect& area, int damage1)
{
    if (!Node::init())    //先调用父类
        return false;

    trapArea = area;     //初始化炸弹区域
    damage = damage1;    //初始化伤害值
    isExploded = false;  //初始设置为没炸过

    return true;
}

bool MapTrap::checkTrigger(const Vector<Soldier*>& soldiers)
{
    if (isExploded) 
        return false;

    // 遍历所有活着的士兵
    for (auto soldier : soldiers) {
        if (soldier && soldier->getCurrentHp() > 0) {

            // 判断士兵的位置点，是否在陷阱矩形内
            // getPosition() 是士兵的世界坐标或者地图相对坐标，取决于你的层级结构
            // 假设 Soldier 和 MapTrap 都添加在 _tileMap 上，坐标系一致
            if (soldier->getSoldierType() == SoldierType::AIRFORCE) {
                continue;      //天空兵种不受炸弹影响
            }
            if (trapArea.containsPoint(soldier->getPosition())) {

                this->explode(soldiers); // 触发爆炸
                return true;
            }
        }
    }
    return false;
}

void MapTrap::explode(const Vector<Soldier*>& soldiers)
{
    isExploded = true;

    // 播放爆炸特效
    this->playExplosionEffect();

    // 造成范围伤害 (所有区域内的士兵都掉血)
    for (auto soldier : soldiers) {
        if (soldier && soldier->getCurrentHp() > 0) {
            if (soldier->getSoldierType() == SoldierType::AIRFORCE) {
                continue;          //天空兵种不受炸弹影响
            }
            if (trapArea.containsPoint(soldier->getPosition())) {
                soldier->takeDamage(damage);     //调用士兵类中造成伤害的函数
            }
        }
    }
}

void MapTrap::playExplosionEffect()
{
    // 创建爆炸 Sprite
    auto explosion = Sprite::create();
    // 放在陷阱中心
    explosion->setPosition(trapArea.origin + trapArea.size / 2);
    explosion->setScale(3.0f); 
    // 特效加在 TileMap 上
    Node* mapNode = this->getParent();
    if (mapNode) { 
        mapNode->addChild(explosion, 999); // 爆炸特效在最上层
    }
    // 加载动画帧，这里与EnemyBUilding逻辑相似
    Vector<SpriteFrame*> frames;
    char str[100] = { 0 };
    for (int i = 1; i <= 9; i++) {     //遍历读取九张爆炸图片
        sprintf(str, "soldiers/Explosion%d.png", i);
        auto sp = Sprite::create(str);
        if (sp) 
            frames.pushBack(sp->getSpriteFrame());   //把读取的图片放进来，准备形成动画
    }

    if (!frames.empty()) {
        auto anim = Animation::createWithSpriteFrames(frames, 0.1f);  //准备爆炸，定义爆炸速度0.1

        auto leaveCraterFunc = CallFunc::create([mapNode, this]() {  //Lambda表达式，将当前炸弹和地图传进来
            // 确保地图还在
            if (!mapNode) 
                return;

            // 创建弹坑图片，让玩家知道出现炸弹了
            auto crater = Sprite::create("soldiers/Bomb.png");

            if (crater) {
                // 设置和陷阱位置一致
                crater->setPosition(trapArea.origin + trapArea.size / 2);

                crater->setOpacity(200); // 稍微半透明
                crater->setScale(5.0f); 
                crater->getTexture()->setAliasTexParameters();
                mapNode->addChild(crater, 100);  //添加到地图中作为子节点

            }
        });

        // 整个爆炸过程 动画 -> 生成弹坑 -> 移除爆炸特效
        explosion->runAction(Sequence::create(
            Animate::create(anim),
            leaveCraterFunc,       // 插入回调，把炸弹显示出来
            RemoveSelf::create(),  // 移除爆炸光效本身
            nullptr
        ));
    }
}
