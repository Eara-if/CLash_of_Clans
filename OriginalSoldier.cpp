#include "OriginalSoldier.h"
#include "BattleScene.h"

bool OriginalSoldier::init(BattleScene* battleScene, SoldierType type)
{
    if (!Soldier::init(battleScene, type)) return false;
    this->setupProperties(type);
    this->setupHealthBar();
    return true;
}

void OriginalSoldier::setupProperties(SoldierType type)
{
    // 加载第一帧
    if (!this->initWithFile(StringUtils::format("%s%d.png", WALK_ANIM_BASE.c_str(), 1))) return;

    // 设置属性
    _maxHp = 75;
    _currentHp = _maxHp;
    _attackDamage = 4;
    _attackRange = 50.0f; // 与巨人相同
    _attackInterval = 1.0f;
    _moveSpeed = 80.0f;

    // 计算每格血量 (5格)
    _damagePerNotch = _maxHp / 5;
    if (_damagePerNotch < 1) _damagePerNotch = 1;
}

void OriginalSoldier::playWalkAnim()
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

void OriginalSoldier::stopAnim()
{
    this->stopActionByTag(101);
}