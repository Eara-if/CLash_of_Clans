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
    _attackDamage = 20;    // �Ա��˺�
    _attackRange = 30.0f;  // ����ǳ�����������Ŵ����������Ա���
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
    // 1. ����˺� (�Ա�)
    if (target) {
        target->takeDamage(this->_attackDamage);
    }

    // ==================== �����������ű�ը���� ====================
    // �߼����������ը���ƣ���������� Parent (��ͼ) ��
    if (this->getParent())
    {
        auto explosion = Sprite::create();
        explosion->setPosition(this->getPosition()); // λ����Ϊը������ǰλ��
        explosion->setScale(3.0f); // ��С���Ը����������

        // ȷ����ʾ�����ϲ� (��ը������ǰ�Ĳ㼶����)
        this->getParent()->addChild(explosion, this->getLocalZOrder() + 10);

        // ��������֡ (1~9)
        Vector<SpriteFrame*> animFrames;
        char str[100] = { 0 };
        for (int i = 1; i <= 9; ++i) {
            // ����ͼƬ·���� soldiers/Explosion1.png
            sprintf(str, "soldiers/Explosion%d.png", i);
            auto sprite = Sprite::create(str);
            if (sprite) {
                animFrames.pushBack(sprite->getSpriteFrame());
            }
        }

        // ���������ж���
        auto animation = Animation::createWithSpriteFrames(animFrames, 0.1f);
        auto seq = Sequence::create(
            Animate::create(animation),
            RemoveSelf::create(), // �������Զ��Ƴ���Ч
            nullptr
        );
        explosion->runAction(seq);
    }
    // ============================================================

    // 2. ֹͣ�������м�ʱ��
    this->unscheduleAllCallbacks();
    this->stopAllActions();

    // 3. �����Ի٣��������
    // BattleScene ������һ֡��⵽ hp <= 0 ���Ƴ���ʿ��
    this->_currentHp = 0;

    // 4. Ѫ������
    this->updateHealthBar();
}