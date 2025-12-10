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

    // ==================== 【新增】播放爆炸动画 ====================
    // 逻辑与防御塔爆炸类似，但必须加在 Parent (地图) 上
    if (this->getParent())
    {
        auto explosion = Sprite::create();
        explosion->setPosition(this->getPosition()); // 位置设为炸弹兵当前位置
        explosion->setScale(3.0f); // 大小可以根据需求调整

        // 确保显示在最上层 (比炸弹兵当前的层级更高)
        this->getParent()->addChild(explosion, this->getLocalZOrder() + 10);

        // 创建动画帧 (1~9)
        Vector<SpriteFrame*> animFrames;
        char str[100] = { 0 };
        for (int i = 1; i <= 9; ++i) {
            // 假设图片路径是 soldiers/Explosion1.png
            sprintf(str, "soldiers/Explosion%d.png", i);
            auto sprite = Sprite::create(str);
            if (sprite) {
                animFrames.pushBack(sprite->getSpriteFrame());
            }
        }

        // 创建并运行动画
        auto animation = Animation::createWithSpriteFrames(animFrames, 0.1f);
        auto seq = Sequence::create(
            Animate::create(animation),
            RemoveSelf::create(), // 播放完自动移除特效
            nullptr
        );
        explosion->runAction(seq);
    }
    // ============================================================

    // 2. 停止自身所有计时器
    this->unscheduleAllCallbacks();
    this->stopAllActions();

    // 3. 核心自毁：标记死亡
    // BattleScene 会在下一帧检测到 hp <= 0 并移除此士兵
    this->_currentHp = 0;

    // 4. 血条更新
    this->updateHealthBar();
}