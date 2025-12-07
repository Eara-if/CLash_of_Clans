// Soldier.cpp

#include "Soldier.h"
#include "GiantSoldier.h" // 用于 Soldier::create 工厂方法
#include "BattleScene.h"
#include "EnemyBuilding.h" // 【修复点 1】必须引用，因为 _target 是 EnemyBuilding*

// =========================================================
// 1. 工厂方法 (Factory Method)
// =========================================================
Soldier* Soldier::create(BattleScene* battleScene, SoldierType type)
{
    if (type == SoldierType::GIANT) {
        // 创建 GiantSoldier 实例
        // 【修改点 2】使用 new 关键字手动创建实例，然后调用 autorelease
        auto ret = new (std::nothrow) GiantSoldier();

        if (ret && ret->init(battleScene, type)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;

    }
    // TODO: 其他兵种在此处添加 else if (type == SoldierType::ARCHER) { ... }

    CCLOG("Error: Unknown Soldier Type!");
    return nullptr;
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
    // 1. 如果没有目标或目标已死亡，寻找新目标
    if (!_target || _target->getCurrentHp() <= 0) {
        _target = nullptr;
        findNewTarget();
    }

    // 2. 如果依然没有目标（说明赢了），待机
    if (!_target) {
        if (_state != State::IDLE) {
            _state = State::IDLE;
            stopAnim(); // 调用虚函数
        }
        return;
    }

    // 3. 计算与目标的距离
    float dist = this->getPosition().distance(_target->getPosition());

    if (dist <= _attackRange) {
        // 进入攻击范围
        if (_state != State::ATTACKING) {
            _state = State::ATTACKING;
            stopAnim(); // 调用虚函数
        }
        attackLogic(dt);
    }
    else {
        // 超出范围，追击
        if (_state != State::MOVING) {
            _state = State::MOVING;
            playWalkAnim(); // 调用虚函数
        }
        moveLogic(dt);
    }
}

void Soldier::findNewTarget()
{
    if (!_battleScene) return;

    // 优先级 1: 防御塔
    auto& towers = _battleScene->getTowers();
    EnemyBuilding* nearestTower = nullptr;
    float minDist = FLT_MAX;

    Vec2 myPos = this->getPosition();

    for (auto tower : towers) {
        if (tower && tower->getCurrentHp() > 0) {
            float dist = myPos.distance(tower->getPosition());
            if (dist < minDist) {
                minDist = dist;
                nearestTower = tower;
            }
        }
    }

    if (nearestTower) {
        _target = nearestTower;
        return;
    }

    // 优先级 2: 大本营
    auto base = _battleScene->getBase();
    if (base && base->getCurrentHp() > 0) {
        _target = base;
    }
}

void Soldier::moveLogic(float dt)
{
    if (!_target) return;

    Vec2 myPos = this->getPosition();
    Vec2 targetPos = _target->getPosition();

    // 计算方向向量
    Vec2 direction = (targetPos - myPos).getNormalized();

    // 移动
    Vec2 newPos = myPos + direction * _moveSpeed * dt;
    this->setPosition(newPos);

    // 简单的朝向调整
    if (direction.x > 0) this->setFlippedX(false);
    else if (direction.x < 0) this->setFlippedX(true);
}

void Soldier::attackLogic(float dt)
{
    _attackTimer += dt;
    if (_attackTimer >= _attackInterval) {
        _attackTimer = 0.0f;

        // 执行攻击
        if (_target) {
            // 攻击动画效果（保持 3.0f 的缩放基准）
            auto seq = Sequence::create(
                ScaleTo::create(0.1f, 2.8f),
                ScaleTo::create(0.1f, 3.0f),
                nullptr
            );
            this->runAction(seq);

            // 扣除目标血量
            _target->takeDamage(_attackDamage);

            // 如果目标死了，清空引用
            if (_target->getCurrentHp() <= 0) {
                _target = nullptr;
            }
        }
    }
}

void Soldier::takeDamage(int damage)
{
    _currentHp -= damage;
    if (_currentHp < 0) _currentHp = 0; // 避免负数

    updateHealthBar(); // 更新血条显示

    if (_currentHp <= 0) {
        this->removeFromParent();
        // 可以在这里添加死亡动画等
    }
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