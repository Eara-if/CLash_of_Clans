// GiantSoldier.cpp
#include "GiantSoldier.h"
#include "BattleScene.h"

// =========================================================
// 1. 子类初始化
// =========================================================
bool GiantSoldier::init(BattleScene* battleScene, SoldierType type)
{
    // 1. 调用基类的 init，完成通用初始化
    if (!Soldier::init(battleScene, type)) return false;

    // 2. 设置巨人士兵独有的属性和图片 (必须在调用 setupHealthBar 前完成)
    this->setupProperties(type);


    // 4. 初始化血条（使用基类的实现）
    this->setupHealthBar();

    return true;
}

// =========================================================
// 2. 属性设置 (特有逻辑)
// =========================================================
void GiantSoldier::setupProperties(SoldierType type)
{
    // 加载巨人的第一帧图片
    if (!this->initWithFile(StringUtils::format("%s%d.png", WALK_ANIM_BASE.c_str(), 1))) return;

    // 设置巨人的属性
    _maxHp = 100;
    _currentHp = _maxHp;
    _attackDamage = 3;
    _attackRange = 50.0f;
    _attackInterval = 1.0f;
    _moveSpeed = 80.0f;

    // 计算巨人的每格血量
    const int NOTCH_COUNT = 5;
    _damagePerNotch = _maxHp / NOTCH_COUNT;
    if (_damagePerNotch < 1) _damagePerNotch = 1;
}

// =========================================================
// 3. 动画逻辑 (特有逻辑)
// =========================================================
void GiantSoldier::playWalkAnim()
{
    if (this->getActionByTag(101)) return;

    auto animation = Animation::create();
    for (int i = 1; i <= 4; ++i) {
        std::string name = StringUtils::format("%s%d.png", WALK_ANIM_BASE.c_str(), i);
        animation->addSpriteFrameWithFile(name);
    }

    animation->setDelayPerUnit(0.15f);

    auto action = RepeatForever::create(Animate::create(animation));
    action->setTag(101);
    this->runAction(action);
}

void GiantSoldier::stopAnim()
{
    this->stopActionByTag(101);
}