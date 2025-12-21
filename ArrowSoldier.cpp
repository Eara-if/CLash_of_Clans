#include "ArrowSoldier.h"
#include "BattleScene.h"
#include "EnemyBuilding.h" // ��������ȷ���ܹ�ʹ�� EnemyBuilding ����

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
    _attackRange = 200.0f; // Զ�̹�����Χ
    _attackInterval = 1.0f;
    _moveSpeed = 80.0f;

    _damagePerNotch = _maxHp / 5;
    if (_damagePerNotch < 1) _damagePerNotch = 1;
}

void ArrowSoldier::playWalkAnim()
{
    if (this->getActionByTag(101)) return;

    auto animation = Animation::create();
    for (int i = 1; i <= 4; ++i) { // ������2֡����
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

// ����������д attackTarget ʵ��Զ�̹����Ͷ���
void ArrowSoldier::attackTarget(EnemyBuilding* target)
{
    // 1. ��ȫ���
    if (!target || target->isDestroyed()) {
        return;
    }

    // 2. �������� Sprite
    // ȷ�����ͼƬ·����ȷ������ʹ�� "weapon/Arrow.png"
    auto arrow = Sprite::create("weapon/Arrow.png");
    if (!arrow) {
        // ���ͼƬ����ʧ�ܣ���Ϊ���÷�����ֱ��Ӧ���˺�
        target->takeDamage(this->_attackDamage);
        return;
    }

    // 3. ���ó�ʼλ�ú͸��ڵ�
    Vec2 startPos = this->getPosition(); // �������� TileMap �ϵ�λ��
    arrow->setPosition(startPos);

    // ���ؼ�����������ӵ��������ĸ��ڵ㣨�� TileMap����
    // Z-Order ����Ϊ�ȹ�������һ�㣬ȷ�������ڵ�ͼ�Ϸ�
    if (this->getParent()) {
        this->getParent()->addChild(arrow, this->getLocalZOrder() + 1);
    }
    else {
        // �������������Ϊ��
        this->addChild(arrow);
        arrow->setPosition(this->getContentSize() / 2);
    }

    // 4. ����Ŀ��λ�á��ٶȺͷ���ʱ��
    Vec2 targetPos = target->getPosition();

    float distance = startPos.distance(targetPos);
    float speed = 800.0f; // �����ķ����ٶ� (����/��)���ɵ���
    float duration = distance / speed;

    // 5. ������ת�Ƕȣ��ù���ͷ����Ŀ�� (��ѡ�����Ƽ�)
    Vec2 direction = targetPos - startPos;
    float angle = CC_RADIANS_TO_DEGREES(atan2(direction.y, direction.x));
    arrow->setRotation(-angle); // Cocos ����תͨ������ѧ�Ƕ��෴
    arrow->setScale(3.0f); // ���ݼ�ͼ��������

    // 6. �����������У��ƶ� -> ����˺� -> �Ƴ�����
    auto seq = Sequence::create(
        MoveTo::create(duration, targetPos),
        CallFunc::create([target, this, arrow]() {
            // Lambda �ص�������������Ŀ��λ��ʱִ��

            // �ٴμ��Ŀ���Ƿ��ڷ���;�����������类����ʿ����ɱ��
            if (target && target->getCurrentHp() > 0) {
                // ����˺�
                target->takeDamage(this->_attackDamage);
            }
            // �Ƴ����� Sprite
            arrow->removeFromParent();
            }),
        nullptr
    );

    arrow->runAction(seq);
}