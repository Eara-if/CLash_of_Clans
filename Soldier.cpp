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
    // 假设血条图片文件名为 "ui/Heart1.png"
    _healthBar = Sprite::create("ui/Heart1.png");

    // 【注意】_damagePerNotch 必须在子类的 setupProperties 中设置完毕后，才能调用此函数

    if (_healthBar)
    {
        // 1. 获取整张血条图片的尺寸
        float textureWidth = _healthBar->getContentSize().width;
        float textureHeight = _healthBar->getContentSize().height;

        // 2. 计算每一格的宽度
        float frameWidth = textureWidth / 4.0f;

        // 3. 初始状态：显示满血（第 4 格，索引 3）
        _healthBar->setTextureRect(Rect(3.0f * frameWidth, 0, frameWidth, textureHeight));

        // 4. 设置血条位置 (相对士兵自身的中心和高度)
        // 放在士兵头顶 (士兵本体缩放了 3.0f，因此 +40 的偏移量相对合适)
        _healthBar->setPosition(Vec2(this->getContentSize().width / 2, this->getContentSize().height + 40));

        // 5. 血条缩放
        _healthBar->setScale(1.5f / 3.0f); // 血条缩放 1.5f，但由于父节点缩放了 3.0f，所以这里要抵消

        this->addChild(_healthBar);
    }
}

void Soldier::updateHealthBar()
{
    if (!_healthBar) return;

    // 1. 计算损失的血量
    int lostHp = _maxHp - _currentHp;

    // 2. 计算损失了多少个 Notch (格)
    int lostNotches = lostHp / _damagePerNotch;

    // 3. 限制索引在 0 到 3 之间
    const int NOTCH_COUNT = 4;
    if (lostNotches >= NOTCH_COUNT) lostNotches = NOTCH_COUNT - 1;

    // 4. 计算当前应该显示的血条索引 (假设 [空血] [1/4] [2/4] [满血]，索引 0 到 3)
    int currentFrameIndex = (NOTCH_COUNT - 1) - lostNotches;

    // 5. 计算切图区域并设置
    float textureWidth = _healthBar->getTexture()->getContentSize().width;
    float textureHeight = _healthBar->getTexture()->getContentSize().height;
    float frameWidth = textureWidth / NOTCH_COUNT;

    _healthBar->setTextureRect(Rect(currentFrameIndex * frameWidth, 0, frameWidth, textureHeight));
}