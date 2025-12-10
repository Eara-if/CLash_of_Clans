#include "ArrowSoldier.h"
#include "BattleScene.h"
#include "EnemyBuilding.h" // 【新增】确保能够使用 EnemyBuilding 类型

bool ArrowSoldier::init(BattleScene* battleScene, SoldierType type)
{
    if (!Soldier::init(battleScene, type)) return false;
    this->setupProperties(type);
    this->setupHealthBar();
    return true;
}

void ArrowSoldier::setupProperties(SoldierType type)
{
    if (!this->initWithFile(StringUtils::format("%s%d.png", WALK_ANIM_BASE.c_str(), 1))) return;

    _maxHp = 60;
    _currentHp = _maxHp;
    _attackDamage = 8;
    _attackRange = 200.0f; // 远程攻击范围
    _attackInterval = 1.0f;
    _moveSpeed = 80.0f;

    _damagePerNotch = _maxHp / 5;
    if (_damagePerNotch < 1) _damagePerNotch = 1;
}

void ArrowSoldier::playWalkAnim()
{
    if (this->getActionByTag(101)) return;

    auto animation = Animation::create();
    for (int i = 1; i <= 4; ++i) { // 假设有2帧动画
        std::string name = StringUtils::format("%s%d.png", WALK_ANIM_BASE.c_str(), i);
        animation->addSpriteFrameWithFile(name);
    }
    animation->setDelayPerUnit(0.15f);

    auto action = RepeatForever::create(Animate::create(animation));
    action->setTag(101);
    this->runAction(action);
}

void ArrowSoldier::stopAnim()
{
    this->stopActionByTag(101);
}

// 【新增】重写 attackTarget 实现远程攻击和动画
void ArrowSoldier::attackTarget(EnemyBuilding* target)
{
    // 1. 安全检查
    if (!target || target->isDestroyed()) {
        return;
    }

    // 2. 创建弓箭 Sprite
    // 确保你的图片路径正确，这里使用 "weapon/Arrow.png"
    auto arrow = Sprite::create("weapon/Arrow.png");
    if (!arrow) {
        // 如果图片加载失败，作为备用方案，直接应用伤害
        target->takeDamage(this->_attackDamage);
        return;
    }

    // 3. 设置初始位置和父节点
    Vec2 startPos = this->getPosition(); // 弓箭兵在 TileMap 上的位置
    arrow->setPosition(startPos);

    // 【关键】将弓箭添加到弓箭兵的父节点（即 TileMap）上
    // Z-Order 设置为比弓箭兵高一点，确保弓箭在地图上方
    if (this->getParent()) {
        this->getParent()->addChild(arrow, this->getLocalZOrder() + 1);
    }
    else {
        // 极少情况，但作为后备
        this->addChild(arrow);
        arrow->setPosition(this->getContentSize() / 2);
    }

    // 4. 计算目标位置、速度和飞行时间
    Vec2 targetPos = target->getPosition();

    float distance = startPos.distance(targetPos);
    float speed = 800.0f; // 弓箭的飞行速度 (像素/秒)，可调整
    float duration = distance / speed;

    // 5. 计算旋转角度：让弓箭头朝向目标 (可选，但推荐)
    Vec2 direction = targetPos - startPos;
    float angle = CC_RADIANS_TO_DEGREES(atan2(direction.y, direction.x));
    arrow->setRotation(-angle); // Cocos 的旋转通常与数学角度相反
    arrow->setScale(3.0f); // 根据箭图调整缩放

    // 6. 创建动作序列：移动 -> 造成伤害 -> 移除弓箭
    auto seq = Sequence::create(
        MoveTo::create(duration, targetPos),
        CallFunc::create([target, this, arrow]() {
            // Lambda 回调：当弓箭到达目标位置时执行

            // 再次检查目标是否在飞行途中死亡（例如被其他士兵击杀）
            if (target && target->getCurrentHp() > 0) {
                // 造成伤害
                target->takeDamage(this->_attackDamage);
            }
            // 移除弓箭 Sprite
            arrow->removeFromParent();
            }),
        nullptr
    );

    arrow->runAction(seq);
}