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

// =========================================================
// 【新增】攻击逻辑：自爆！
// =========================================================
// =========================================================
// 【修改】攻击逻辑：自爆！
// =========================================================
void BoomSoldier::attackTarget(EnemyBuilding* target)
{
    // 确保目标存在
    if (!target) return;

    // 1. 停止所有后续动作和计时器
    // 关键：终止基类 Soldier 启动的重复攻击计时器
    this->unscheduleAllCallbacks();
    this->stopAllActions();

    // 2. 释放伤害 (值为 20)
    // 伤害值已经在 setupProperties 中设置 (_attackDamage = 20)
    target->takeDamage(this->_attackDamage);

    // 3. 播放爆炸动画/特效 (可选，如果爆炸动画很短，可以加，如果长，需另行处理)
    // ... (你的爆炸特效代码) ...

    // 4. 立即将自己血量归零并消失
    this->_currentHp = 0;

    // 从 BattleScene 的 _soldiers 容器中移除，并从场景中移除
    this->removeFromParent();

    // 【重要】自爆完成
}