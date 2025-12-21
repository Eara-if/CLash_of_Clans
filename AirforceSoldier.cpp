#include "AirforceSoldier.h"
#include "BattleScene.h"

bool AirforceSoldier::init(BattleScene* battleScene, SoldierType type)
{
    if (!Soldier::init(battleScene, type)) return false;

    this->setupProperties(type);
    this->setupHealthBar();

    // 【新增】添加地面的阴影
    // 使用 DrawNode 画一个半透明黑色椭圆，或者用 Sprite
    auto shadow = DrawNode::create();
    // 画一个扁的椭圆模拟阴影
    shadow->drawSolidCircle(Vec2::ZERO, 15.0f, 0.0f, 20, 0.5f, 1.0f, Color4F(0, 0, 0, 0.4f));
    shadow->setScaleY(0.5f); // 压扁

    // 阴影位置：在猫头鹰脚下很远的地方（模拟飞在天上）
    // 注意：ZOrder设为-1，让它显示在身体下面
    shadow->setPosition(Vec2(this->getContentSize().width / 2, -30.0f));
    this->addChild(shadow, -1);

    return true;
}

void AirforceSoldier::setupProperties(SoldierType type)
{
    // 加载第一帧
    // 路径：Resources/anim/Owl1.png
    if (!this->initWithFile("anim/Owl1.png")) return;

    // 设定数值
    _maxHp = 50;
    _currentHp = _maxHp;
    _attackDamage = 6;
    _attackRange = 50.0f; // 近战范围
    _attackInterval = 1.0f;
    _moveSpeed = 80.0f;

    _damagePerNotch = _maxHp / 5;
    if (_damagePerNotch < 1) _damagePerNotch = 1;
}

void AirforceSoldier::playWalkAnim()
{
    if (this->getActionByTag(101)) return;

    auto animation = Animation::create();
    for (int i = 1; i <= 4; ++i) {
        // 这里的路径需要根据你实际 Resources 文件夹结构调整
        // 假设放在 Resources/anim/ 下
        std::string name = StringUtils::format("anim/Owl%d.png", i);
        animation->addSpriteFrameWithFile(name);
    }
    animation->setDelayPerUnit(0.1f); // 翅膀扇快点

    auto action = RepeatForever::create(Animate::create(animation));
    action->setTag(101);
    this->runAction(action);

    // 飞行单位可以加一个上下浮动的 Action 增加真实感
    auto floatAction = RepeatForever::create(Sequence::create(
        MoveBy::create(0.5f, Vec2(0, 5)),
        MoveBy::create(0.5f, Vec2(0, -5)),
        nullptr
    ));
    floatAction->setTag(102);
    this->runAction(floatAction);
}

void AirforceSoldier::stopAnim()
{
    this->stopActionByTag(101);
    this->stopActionByTag(102);
}