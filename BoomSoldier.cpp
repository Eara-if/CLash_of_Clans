#include "BoomSoldier.h"
#include "BattleScene.h"

bool BoomSoldier::init(BattleScene* battleScene, SoldierType type)
{
    if (!Soldier::init(battleScene, type)) return false;
    this->setupProperties(type);
    this->setupHealthBar();
    return true;
}

void BoomSoldier::setupProperties(SoldierType type)
{
    if (!this->initWithFile(StringUtils::format("%s%d.png", WALK_ANIM_BASE.c_str(), 1))) return;

    _maxHp = 150;
    _currentHp = _maxHp;
    _attackDamage = 20;    // 自爆伤害
    _attackRange = 30.0f;  // 必须非常靠近建筑物才触发攻击（自爆）
    _attackInterval = 1.0f;
    _moveSpeed = 80.0f;

    _damagePerNotch = _maxHp / 5;
    if (_damagePerNotch < 1) _damagePerNotch = 1;
}

void BoomSoldier::playWalkAnim()
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

void BoomSoldier::stopAnim()
{
    this->stopActionByTag(101);
}

void BoomSoldier::attackTarget(EnemyBuilding* target)
{
    // 1. 造成伤害 (自爆)
    if (target) {
        target->takeDamage(this->_attackDamage);
    }

    // 2. 停止自身所有计时器 (非必须，但推荐)
    this->unscheduleAllCallbacks();
    this->stopAllActions();

    // 3. 核心自毁：标记死亡
    this->_currentHp = 0;

    // 4. 血条更新 (更新后血条会显示空)
    this->updateHealthBar();

    // 不调用 removeFromParent()，等待 BattleScene 清理（您已在 BattleScene 中正确实现）
}