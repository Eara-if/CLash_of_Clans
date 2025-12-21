// Soldier.cpp

#include "Soldier.h"
#include "GiantSoldier.h"    // ����ʿ��
#include "OriginalSoldier.h" // ԭʼʿ��
#include "ArrowSoldier.h"    // ������
#include "BoomSoldier.h"     // ը����
#include "AirforceSoldier.h"
#include "BattleScene.h"
#include "EnemyBuilding.h" 

// =========================================================
// 1. �������� (Factory Method)
// =========================================================
Soldier* Soldier::create(BattleScene* battleScene, SoldierType type)
{
    Soldier* ret = nullptr;

    switch (type) {
        case SoldierType::ORIGINAL:
        {
            // 1. ���� OriginalSoldier ʵ��
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
            // 2. ���� ArrowSoldier ʵ��
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
            // 3. ���� BoomSoldier ʵ��
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
            // 4. ���� GiantSoldier ʵ�� (���ģ��)
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
        case SoldierType::AIRFORCE:
        {
            auto air = new (std::nothrow) AirforceSoldier();
            if (air && air->init(battleScene, type)) {
                air->autorelease();
                ret = air;
            }
            else {
                CC_SAFE_DELETE(air);
            }
            break;
        }
        default:
        {
            cocos2d::log("Error: Unknown SoldierType %d", (int)type);
            // Ĭ������£�Ҳ���Է��� nullptr �򴴽�һ������ʵ��
            ret = nullptr;
            break;
        }
    }

    return ret;
}

// =========================================================
// 2. �����ʼ�� (ͨ��)
// =========================================================
bool Soldier::init(BattleScene* battleScene, SoldierType type)
{
    if (!Sprite::init()) return false;

    _battleScene = battleScene;
    _attackTimer = 0.0f;
    _state = State::IDLE;
    _target = nullptr;

    // 【新增】保存类型，用于后续 AI 判断
    _soldierType = type;

    this->setScale(3.0f);
    this->scheduleUpdate();
    return true;
}

// =========================================================
// 3. ���� AI �߼� (ͨ��������)
// =========================================================

void Soldier::update(float dt)
{
    // ���Ŀ����Ч��
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
    // �������޸� 1�������ж��߼�
    // ==========================================
    float dist = this->getPosition().distance(_target->getPosition());

    // ֻҪ���νӴ��ˣ����㡰��������
    bool isTouching = this->getBoundingBox().intersectsRect(_target->getBoundingBox());

    // ���� ����� �� �����Ӵ�������Ϊ�ɹ���
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

    // 初始最小分数设为最大值
    float minScore = FLT_MAX;

    Vec2 myPos = this->getPosition();

    // 遍历所有防御塔
    for (auto tower : towers) {
        if (tower && !tower->isDestroyed()) {

            float dist = myPos.distance(tower->getPosition());
            float score = dist; // 基础分数是距离（越小越好）

            EnemyType enemyType = tower->getType();

            // ============================================================
            // 【优先级逻辑核心】通过加减分来控制优先级
            // 分数越低，优先级越高
            // ============================================================

            if (_soldierType == SoldierType::AIRFORCE) {
                // --- 空军逻辑 ---
                if (enemyType == EnemyType::WALL) {
                    // 空军完全无视墙，给墙无限大的罚分
                    score += 1000000.0f;
                }
                else if (enemyType == EnemyType::CANNON) {
                    // 加农炮优先级最高，减去巨额分数，确保即使距离远也先打它
                    score -= 5000.0f;
                }
                else if (enemyType == EnemyType::TOWER) {
                    // 防御塔次之，适量减分
                    score -= 2000.0f;
                }
                // Base 保持原距离分数（优先级最低）
            }
            else if (_soldierType == SoldierType::GIANT) {
                // --- 巨人逻辑 ---
                if (enemyType == EnemyType::WALL) {
                    score += 10000.0f; // 讨厌墙
                }
                else if (enemyType == EnemyType::CANNON || enemyType == EnemyType::TOWER) {
                    // 防御塔和加农炮同等高优先级
                    score -= 5000.0f;
                }
                // Base 保持原距离分数
            }
            else {
                // --- 其他陆军 (Original, Arrow, Boom) ---
                if (enemyType == EnemyType::WALL) {
                    score += 10000.0f; // 除非挡路否则不打墙
                }
                // 其他建筑一视同仁，打最近的
            }

            if (score < minScore) {
                minScore = score;
                bestTarget = tower;
            }
        }
    }

    // 比较基地 (Base)
    auto base = _battleScene->getBase();
    if (base && !base->isDestroyed()) {
        float dist = myPos.distance(base->getPosition());
        float score = dist;

        // 调整基地的优先级分数
        if (_soldierType == SoldierType::AIRFORCE) {
            // 空军：Cannnon > Tower > Base
            // 所以 Base 不需要减分，自然排在后面
        }
        else if (_soldierType == SoldierType::GIANT) {
            // 巨人：Defenses > Base
            // Base 不减分
        }
        // 普通兵：看距离

        if (score < minScore) {
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
//    // ���㷽������
//    Vec2 direction = (targetPos - myPos).getNormalized();
//
//    // �ƶ�
//    Vec2 newPos = myPos + direction * _moveSpeed * dt;
//    this->setPosition(newPos);
//
//    // �򵥵ĳ������
//    if (direction.x > 0) this->setFlippedX(false);
//    else if (direction.x < 0) this->setFlippedX(true);
//}
//// Soldier.cpp
EnemyBuilding* Soldier::findNearestWall()
{
    if (!_battleScene) return nullptr;
    EnemyBuilding* nearest = nullptr;
    float minDst = 100.0f; // ����С��Χ��ֻ��������ǽ (֮ǰ��200����̫Զ)

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

    // 【核心】如果是飞行单位，直接移动，无视阻挡
    if (this->isFlying()) {
        // 简单的朝向调整
        if (direction.x > 0) this->setFlippedX(false);
        else if (direction.x < 0) this->setFlippedX(true);

        this->setPosition(nextPos);
        return;
    }

    // --- 以下是原有的陆军寻路逻辑 ---
    Vec2 nextWorldPos = this->getParent()->convertToWorldSpace(nextPos);

    if (_battleScene->isPositionBlocked(nextWorldPos))
    {

        // 记得保留原有的 "被迫打墙" 逻辑
        Rect myRect = this->getBoundingBox();
        Rect targetRect = _target->getBoundingBox();
        if (myRect.intersectsRect(targetRect)) { return; }

        bool isArcher = (_attackRange > 150.0f);
        float distToTarget = myPos.distance(targetPos);
        if (isArcher && distToTarget <= _attackRange) { return; }

        EnemyBuilding* wall = findNearestWall();
        if (wall && _target != wall) {
            _target = wall;
        }
        return;
    }

    this->setPosition(nextPos);
    if (direction.x > 0) this->setFlippedX(false);
    else if (direction.x < 0) this->setFlippedX(true);
}


void Soldier::attackLogic(float dt) {
    _attackTimer += dt;
    if (_attackTimer >= _attackInterval) {
        _attackTimer = 0.0f;
        if (_target) {
            // ����
            auto seq = Sequence::create(ScaleTo::create(0.1f, 2.8f), ScaleTo::create(0.1f, 3.0f), nullptr);
            this->runAction(seq);
            // ����
            this->attackTarget(_target);
            // ը�����Ա����
            if (this->getReferenceCount() == 0 || this->getCurrentHp() <= 0) 
                return;
            if (_target && _target->getCurrentHp() <= 0) _target = nullptr;
        }
    }
}

void Soldier::takeDamage(int damage)
{
    _currentHp -= damage;
    if (_currentHp < 0) _currentHp = 0; // ���⸺��

    updateHealthBar(); // ����Ѫ����ʾ
}

// =========================================================
// 4. ͨ��Ѫ��ʵ��
// =========================================================

void Soldier::setupHealthBar()
{
    // Ѫ��ͼƬ�ļ���Ϊ "ui/Heart.png"
    _healthBar = Sprite::create("ui/Heart.png");

    // ��ע�⡿_damagePerNotch ����������� setupProperties ��������Ϻ󣬲��ܵ��ô˺���

    if (_healthBar)
    {
        // 1. ��ȡ����Ѫ��ͼƬ�ĳߴ�
        float textureWidth = _healthBar->getContentSize().width;
        float textureHeight = _healthBar->getContentSize().height;

        // 2. ����ÿһ��Ŀ�� (5��)
        float frameWidth = textureWidth / 5.0f;

        // 3. ��ʼ״̬����ʾ��Ѫ���� 5 ������ 4��
        // ���� 4 ��Ӧ��ͼ��� 4.0f * frameWidth��
        _healthBar->setTextureRect(Rect(4.0f * frameWidth, 0, frameWidth, textureHeight)); // 4.0f ����ȷ����Ѫ����

        // 4. ����Ѫ��λ�� (���ʿ����������ĺ͸߶�)
        // ����ʿ��ͷ�� 
        _healthBar->setPosition(Vec2(this->getContentSize().width / 2, this->getContentSize().height));

        // 5. Ѫ������
        _healthBar->setScale(1.0f);

        this->addChild(_healthBar);
    }
}

void Soldier::updateHealthBar()
{
    if (!_healthBar) return;

    const int NOTCH_COUNT = 5;

    // 1. ����ʣ��Ѫ��������
    // ע�⣺��������ͨ��ʣ��Ѫ�������㡰��ʣ���ٸ񡱣������ǡ���ʧ�˶��ٸ񡱡�
    int remainingHp = _currentHp;

    // ���� _damagePerNotch ��ʹѪ������һ������Ҫ���˺�ֵ��
    // ��ô ʣ����� = (��ǰѪ�� / (��Ѫ�� / 5))������ֱ���� Notch ��������

    // ����ʣ��� Notch (��) ��
    // ���ø�׼ȷ�ļ��㷽ʽ�����������������������⣺
    // ��ǰѪ��ռ��Ѫ���İٷֱȣ������ܸ�����
    int currentNotches = roundf((float)remainingHp / (float)_maxHp * NOTCH_COUNT);

    // ��һ�ָ�������ԭ�д���ļ��㷽ʽ (ʹ�� _damagePerNotch):
    // ʣ����� = �ܸ��� - ��ʧ����
    // int lostHp = _maxHp - _currentHp;
    // int lostNotches = lostHp / _damagePerNotch;
    // int currentNotches = NOTCH_COUNT - lostNotches; 

    // �����ȷ�� _damagePerNotch ���� _maxHp / 5���ðٷֱȷ�����ͨ�á�
    // ��������ʹ�û��� Notch ���߼�����ȷ������ 0 �� 5 ֮�䣺
    int lostHp = _maxHp - _currentHp;
    int lostNotches = lostHp / _damagePerNotch;
    int actualNotches = NOTCH_COUNT - lostNotches;

    // 2. ����������Χ [0, 5]����ʵ����ͼ������ [0, 4]
    // 5 ��Ѫ����Ӧ 5 ��״̬ͼƬ������ 0 �� 4��
    // ���Ѫ��Ϊ 0��������Ϊ 0�����Ѫ��Ϊ����������Ϊ 4��
    int frameIndex = actualNotches - 1;

    // �߽��飺ȷ�������� 0 �� 4 ֮��
    if (frameIndex < 0) frameIndex = 0;
    if (frameIndex >= NOTCH_COUNT) frameIndex = NOTCH_COUNT - 1; // ȷ������������ 4

    // 3. ������ͼ��������
    float textureWidth = _healthBar->getTexture()->getContentSize().width;
    float textureHeight = _healthBar->getTexture()->getContentSize().height;
    float frameWidth = textureWidth / NOTCH_COUNT;

    // ���ؼ��޸ĵ㡿
    // ��������ͼƬ�Ǵ��������У�[��][1��][2��][3��][��] (���� 0 -> 4)
    // �� frameIndex �Ѿ���ȷ�����˵�ǰӦ����ʾ������ (0=��, 4=��)
    // ������ͼ�������� frameIndex * frameWidth��

    // ɾ��ԭ�е� (4 - currentFrameIndex) �߼���ֱ��ʹ�� frameIndex ����
    _healthBar->setTextureRect(Rect(frameIndex * frameWidth, 0, frameWidth, textureHeight));
}

void Soldier::attackTarget(EnemyBuilding* target)
{
    // ���� OriginalSoldier, GiantSoldier ��δ��д��ʿ����ִ�����
    if (target) {
        target->takeDamage(this->_attackDamage);
    }
}