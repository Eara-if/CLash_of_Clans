// GiantSoldier.cpp
#include "GiantSoldier.h"
#include "BattleScene.h"

// =========================================================
// 1. �����ʼ��
// =========================================================
bool GiantSoldier::init(BattleScene* battleScene, SoldierType type)
{
    // 1. ���û���� init�����ͨ�ó�ʼ��
    if (!Soldier::init(battleScene, type)) return false;

    // 2. ���þ���ʿ�����е����Ժ�ͼƬ (�����ڵ��� setupHealthBar ǰ���)
    this->setupProperties(type);


    // 4. ��ʼ��Ѫ����ʹ�û����ʵ�֣�
    this->setupHealthBar();

    return true;
}

// =========================================================
// 2. �������� (�����߼�)
// =========================================================
void GiantSoldier::setupProperties(SoldierType type)
{
    // ���ؾ��˵ĵ�һ֡ͼƬ
    if (!this->initWithFile(StringUtils::format("%s%d.png", WALK_ANIM_BASE.c_str(), 1))) return;

    // ���þ��˵�����
    _maxHp = 100;
    _currentHp = _maxHp;
    _attackDamage = 3;
    _attackRange = 50.0f;
    _attackInterval = 1.0f;
    _moveSpeed = 80.0f;

    // ������˵�ÿ��Ѫ��
    const int NOTCH_COUNT = 5;
    _damagePerNotch = _maxHp / NOTCH_COUNT;
    if (_damagePerNotch < 1) _damagePerNotch = 1;
}

// =========================================================
// 3. �����߼� (�����߼�)
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