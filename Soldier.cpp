// Soldier.cpp

#include "Soldier.h"
#include "GiantSoldier.h"    // 巨人士兵
#include "OriginalSoldier.h" // 原始士兵
#include "ArrowSoldier.h"    // 弓箭手
#include "BoomSoldier.h"     // 炸弹人
#include "BattleScene.h"
#include "EnemyBuilding.h" // 【修复点 1】必须引用，因为 _target 是 EnemyBuilding*

// =========================================================
// 1. 工厂方法 (Factory Method)
// =========================================================
Soldier* Soldier::create(BattleScene* battleScene, SoldierType type)
{
    Soldier* ret = nullptr;

    switch (type) {
        case SoldierType::ORIGINAL:
        {
            // 1. 创建 OriginalSoldier 实例
            auto original = new (std::nothrow) OriginalSoldier();
            if (original && original->init(battleScene, type)) {
                original->autorelease();
                ret = original;
            }
            else {
                CC_SAFE_DELETE(original);
            }
            break;
        }
        case SoldierType::ARROW:
        {
            // 2. 创建 ArrowSoldier 实例
            auto arrow = new (std::nothrow) ArrowSoldier();
            if (arrow && arrow->init(battleScene, type)) {
                arrow->autorelease();
                ret = arrow;
            }
            else {
                CC_SAFE_DELETE(arrow);
            }
            break;
        }
        case SoldierType::BOOM:
        {
            // 3. 创建 BoomSoldier 实例
            auto boom = new (std::nothrow) BoomSoldier();
            if (boom && boom->init(battleScene, type)) {
                boom->autorelease();
                ret = boom;
            }
            else {
                CC_SAFE_DELETE(boom);
            }
            break;
        }
        case SoldierType::GIANT:
        {
            // 4. 创建 GiantSoldier 实例 (你的模板)
            auto giant = new (std::nothrow) GiantSoldier();
            if (giant && giant->init(battleScene, type)) {
                giant->autorelease();
                ret = giant;
            }
            else {
                CC_SAFE_DELETE(giant);
            }
            break;
        }
        default:
        {
            cocos2d::log("Error: Unknown SoldierType %d", (int)type);
            // 默认情况下，也可以返回 nullptr 或创建一个基类实例
            ret = nullptr;
            break;
        }
    }

    return ret;
}

// =========================================================
// 2. 基类初始化 (通用)
// =========================================================
bool Soldier::init(BattleScene* battleScene, SoldierType type)
{
    // 不加载图片，图片加载在子类的 setupProperties 中进行
    if (!Sprite::init()) return false;

    _battleScene = battleScene;
    _attackTimer = 0.0f;
    _state = State::IDLE;
    _target = nullptr;

    // 通用缩放设置（假设所有士兵都是 3.0f）
    this->setScale(3.0f);

    this->scheduleUpdate();

    return true;
}

// =========================================================
// 3. 核心 AI 逻辑 (通用且完整)
// =========================================================

void Soldier::update(float dt)
{
    // 检查目标有效性
    if (!_target || _target->getCurrentHp() <= 0 || _target->isDestroyed()) {
        _target = nullptr;
        findNewTarget();
    }

    if (!_target) {
        if (_state != State::IDLE) {
            _state = State::IDLE;
            stopAnim();
        }
        return;
    }

    // ==========================================
    // 【核心修复 1】攻击判定逻辑
    // ==========================================
    float dist = this->getPosition().distance(_target->getPosition());

    // 只要矩形接触了，就算“贴脸”了
    bool isTouching = this->getBoundingBox().intersectsRect(_target->getBoundingBox());

    // 满足 射程内 或 贴脸接触，都视为可攻击
    if (dist <= _attackRange || isTouching) {
        if (_state != State::ATTACKING) {
            _state = State::ATTACKING;
            stopAnim();
        }
        attackLogic(dt);
    }
    else {
        if (_state != State::MOVING) {
            _state = State::MOVING;
            playWalkAnim();
        }
        moveLogic(dt);
    }
}

// Soldier.cpp

void Soldier::findNewTarget()
{
    if (!_battleScene) return;

    auto& towers = _battleScene->getTowers();
    EnemyBuilding* bestTarget = nullptr;
    float minScore = FLT_MAX;

    Vec2 myPos = this->getPosition();

    // 判断是否是弓箭手 (用于之后的特殊逻辑，如果有的话)
    bool isArcher = (_attackRange > 150.0f);

    for (auto tower : towers) {
        if (tower && !tower->isDestroyed()) {

            float dist = myPos.distance(tower->getPosition());
            float score = dist;

            // ============================================================
            // 【核心修改】所有士兵都极度嫌弃墙！
            // ============================================================
            if (tower->getType() == EnemyType::WALL) {
                // 给墙增加巨额罚分 (相当于这个墙在 10000 像素以外)
                // 这样，只要场上还有活着的塔或基地，士兵绝对不会主动把墙当目标
                score += 10000.0f;
            }

            if (score < minScore) {
                minScore = score;
                bestTarget = tower;
            }
        }
    }

    // 比较基地 (基地优先级最高，不加罚分)
    auto base = _battleScene->getBase();
    if (base && !base->isDestroyed()) {
        float dist = myPos.distance(base->getPosition());
        // 如果算出来的墙的分数(距离+10000)比基地还小，那说明实在没东西打了
        if (dist < minScore) {
            bestTarget = base;
        }
    }

    _target = bestTarget;
}

//void Soldier::moveLogic(float dt)
//{
//    if (!_target) return;
//
//    Vec2 myPos = this->getPosition();
//    Vec2 targetPos = _target->getPosition();
//
//    // 计算方向向量
//    Vec2 direction = (targetPos - myPos).getNormalized();
//
//    // 移动
//    Vec2 newPos = myPos + direction * _moveSpeed * dt;
//    this->setPosition(newPos);
//
//    // 简单的朝向调整
//    if (direction.x > 0) this->setFlippedX(false);
//    else if (direction.x < 0) this->setFlippedX(true);
//}
//// Soldier.cpp
EnemyBuilding* Soldier::findNearestWall()
{
    if (!_battleScene) return nullptr;
    EnemyBuilding* nearest = nullptr;
    float minDst = 100.0f; // 【缩小范围】只找贴脸的墙 (之前是200可能太远)

    Vec2 myPos = this->getPosition();
    for (auto b : _battleScene->getTowers()) {
        if (b && !b->isDestroyed() && b->getType() == EnemyType::WALL) {
            float dst = myPos.distance(b->getPosition());
            if (dst < minDst) {
                minDst = dst;
                nearest = b;
            }
        }
    }
    return nearest;
}

void Soldier::moveLogic(float dt)
{
    if (!_target) return;

    Vec2 myPos = this->getPosition();
    Vec2 targetPos = _target->getPosition();

    Vec2 direction = (targetPos - myPos).getNormalized();
    Vec2 nextPos = myPos + direction * _moveSpeed * dt;
    Vec2 nextWorldPos = this->getParent()->convertToWorldSpace(nextPos);

    // 检测阻挡
    if (_battleScene->isPositionBlocked(nextWorldPos))
    {
        // === 撞到东西了 ===

        // 1. 如果撞到的是目标本身 -> 停下攻击
        Rect myRect = this->getBoundingBox();
        Rect targetRect = _target->getBoundingBox();
        if (myRect.intersectsRect(targetRect)) {
            return;
        }

        // 2. 弓箭手逻辑 (隔墙打)
        bool isArcher = (_attackRange > 150.0f);
        float distToTarget = myPos.distance(targetPos);
        if (isArcher && distToTarget <= _attackRange) {
            return;
        }

        // 3. 【近战兵被挡路】 -> 被迫打墙
        // 注意：这里我们是在 "被迫" 状态。
        // 因为 findNewTarget 已经忽略了墙，所以 _target 此刻肯定是个远处的塔。
        // 但路不通，所以我们要临时把目标切换成挡路的这堵墙。

        EnemyBuilding* wall = findNearestWall();

        // 只有当墙真的在身边时才切目标
        if (wall && _target != wall) {
            _target = wall;
            // 士兵会原地开始攻击这堵墙。
            // 当墙被打爆 (_isDestroyed=true) 后，update 会调用 findNewTarget。
            // 此时 findNewTarget 又会忽略剩下的墙，继续锁定远处的塔。
            // 结果：士兵打穿一个缺口就走了，不会清理旁边的墙。
        }
        return;
    }

    // 没挡路就正常走
    this->setPosition(nextPos);
    if (direction.x > 0) this->setFlippedX(false);
    else if (direction.x < 0) this->setFlippedX(true);
}
void Soldier::attackLogic(float dt) {
    _attackTimer += dt;
    if (_attackTimer >= _attackInterval) {
        _attackTimer = 0.0f;
        if (_target) {
            // 动画
            auto seq = Sequence::create(ScaleTo::create(0.1f, 2.8f), ScaleTo::create(0.1f, 3.0f), nullptr);
            this->runAction(seq);
            // 攻击
            this->attackTarget(_target);
            // 炸弹人自爆检测
            if (this->getReferenceCount() == 0 || this->getCurrentHp() <= 0) 
                return;
            if (_target && _target->getCurrentHp() <= 0) _target = nullptr;
        }
    }
}

void Soldier::takeDamage(int damage)
{
    _currentHp -= damage;
    if (_currentHp < 0) _currentHp = 0; // 避免负数

    updateHealthBar(); // 更新血条显示
}

// =========================================================
// 4. 通用血条实现
// =========================================================

void Soldier::setupHealthBar()
{
    // 血条图片文件名为 "ui/Heart.png"
    _healthBar = Sprite::create("ui/Heart.png");

    // 【注意】_damagePerNotch 必须在子类的 setupProperties 中设置完毕后，才能调用此函数

    if (_healthBar)
    {
        // 1. 获取整张血条图片的尺寸
        float textureWidth = _healthBar->getContentSize().width;
        float textureHeight = _healthBar->getContentSize().height;

        // 2. 计算每一格的宽度 (5格)
        float frameWidth = textureWidth / 5.0f;

        // 3. 初始状态：显示满血（第 5 格，索引 4）
        // 索引 4 对应切图起点 4.0f * frameWidth。
        _healthBar->setTextureRect(Rect(4.0f * frameWidth, 0, frameWidth, textureHeight)); // 4.0f 是正确的满血索引

        // 4. 设置血条位置 (相对士兵自身的中心和高度)
        // 放在士兵头顶 
        _healthBar->setPosition(Vec2(this->getContentSize().width / 2, this->getContentSize().height));

        // 5. 血条缩放
        _healthBar->setScale(1.0f);

        this->addChild(_healthBar);
    }
}

void Soldier::updateHealthBar()
{
    if (!_healthBar) return;

    const int NOTCH_COUNT = 5;

    // 1. 计算剩余血量格子数
    // 注意：这里我们通过剩余血量来计算“还剩多少格”，而不是“损失了多少格”。
    int remainingHp = _currentHp;

    // 假设 _damagePerNotch 是使血条减少一格所需要的伤害值，
    // 那么 剩余格数 = (当前血量 / (总血量 / 5))，或者直接用 Notch 数量计算

    // 计算剩余的 Notch (格) 数
    // 采用更准确的计算方式，避免整数除法带来的问题：
    // 当前血量占总血量的百分比，乘以总格数。
    int currentNotches = roundf((float)remainingHp / (float)_maxHp * NOTCH_COUNT);

    // 另一种更贴近你原有代码的计算方式 (使用 _damagePerNotch):
    // 剩余格数 = 总格数 - 损失格数
    // int lostHp = _maxHp - _currentHp;
    // int lostNotches = lostHp / _damagePerNotch;
    // int currentNotches = NOTCH_COUNT - lostNotches; 

    // 如果你确定 _damagePerNotch 总是 _maxHp / 5，用百分比方法更通用。
    // 这里我们使用基于 Notch 的逻辑，但确保它在 0 到 5 之间：
    int lostHp = _maxHp - _currentHp;
    int lostNotches = lostHp / _damagePerNotch;
    int actualNotches = NOTCH_COUNT - lostNotches;

    // 2. 限制索引范围 [0, 5]，但实际切图索引是 [0, 4]
    // 5 格血条对应 5 个状态图片，索引 0 到 4。
    // 如果血量为 0，则索引为 0。如果血量为满，则索引为 4。
    int frameIndex = actualNotches - 1;

    // 边界检查：确保索引在 0 到 4 之间
    if (frameIndex < 0) frameIndex = 0;
    if (frameIndex >= NOTCH_COUNT) frameIndex = NOTCH_COUNT - 1; // 确保索引不超过 4

    // 3. 计算切图区域并设置
    float textureWidth = _healthBar->getTexture()->getContentSize().width;
    float textureHeight = _healthBar->getTexture()->getContentSize().height;
    float frameWidth = textureWidth / NOTCH_COUNT;

    // 【关键修改点】
    // 由于您的图片是从左到右排列：[空][1格][2格][3格][满] (索引 0 -> 4)
    // 且 frameIndex 已经正确计算了当前应该显示的索引 (0=空, 4=满)
    // 所以切图的起点就是 frameIndex * frameWidth。

    // 删掉原有的 (4 - currentFrameIndex) 逻辑，直接使用 frameIndex 即可
    _healthBar->setTextureRect(Rect(frameIndex * frameWidth, 0, frameWidth, textureHeight));
}

void Soldier::attackTarget(EnemyBuilding* target)
{
    // 对于 OriginalSoldier, GiantSoldier 等未重写的士兵，执行这里！
    if (target) {
        target->takeDamage(this->_attackDamage);
    }
}