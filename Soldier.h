#ifndef SOLDIER_H_
#define SOLDIER_H_

#include "cocos2d.h"
#include <vector>
#include "EnemyBuilding.h" // ��Ҫ���� EnemyBuilding ͷ�ļ�


USING_NS_CC;
class BattleScene; // ǰ�����������⽻������

// �����������
enum SoldierType {
    ORIGINAL = 0,
    ARROW = 1,
    BOOM = 2,
    GIANT = 3,
	AIRFORCE = 4
};

class Soldier : public Sprite
{
public:
    // ͳһ�Ĵ����ӿڣ�����������
    static Soldier* create(BattleScene* battleScene, SoldierType type);

    // ��ʼ���߼������ڸ������ͨ����Դ����� Update
    virtual bool init(BattleScene* battleScene, SoldierType type);

    // 【新增】虚函数，判断是否是飞行单位
    virtual bool isFlying() const { return false; }

    // 【新增】获取单位类型
    SoldierType getSoldierType() const { return _soldierType; }

    // AI �߼� (���б���ͨ��)
    void update(float dt);
    void findNewTarget();
    void moveLogic(float dt);
    void attackLogic(float dt);

    // ͨ��ս���߼�
    void takeDamage(int damage);
    int getCurrentHp() const { return _currentHp; }
    EnemyBuilding* findNearestWall();

protected:
    SoldierType _soldierType; // 记得在 init 中赋值
    enum class State { IDLE, MOVING, ATTACKING };

    BattleScene* _battleScene;
    EnemyBuilding* _target;

    int _maxHp;
    int _currentHp;
    int _attackDamage;
    float _attackRange;
    float _attackInterval;
    float _attackTimer;
    float _moveSpeed;
    State _state;

    // Ѫ����Ա
    Sprite* _healthBar;
    int _damagePerNotch; // ��һ��Ѫ��Ҫ���˺�

    // ���麯����������ʵ���ض������ԺͶ��� 
    virtual void setupProperties(SoldierType type) = 0;
    virtual void playWalkAnim() = 0;
    virtual void stopAnim() = 0;

    // Ѫ���߼�������ʵ�֣���Ϊ�麯��������������д��
    virtual void setupHealthBar();
    virtual void updateHealthBar();
    // ���ؼ�������ȷ������������ virtual �ģ����������д
    virtual void attackTarget(EnemyBuilding* target);
};

#endif // SOLDIER_H_