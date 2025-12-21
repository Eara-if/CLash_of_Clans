#ifndef __ENEMY_BUILDING_H__
#define __ENEMY_BUILDING_H__

#include "cocos2d.h"
// 2. ��������ǰ�����������߱����� Soldier ��һ���࣬����ϸ�ڵȻ����˵
class Soldier;
enum class EnemyType {
    BASE,   // ��Ӫ
    TOWER,  // ������ (�ṥ��)
    WALL,   // դ��/ǽ (���ṥ�������赲�ƶ�)
    CANNON  // 【新增】加农炮
};
class EnemyBuilding : public cocos2d::Sprite
{
public:
    // ��ʼ������
    // filename: ���������ͼƬ (�� Base.png)
    // hpBarFilename: Ѫ����ͼƬ (�� Heart2.png)
    // totalHp: ��Ѫ�� (���� 3��Ѫ��ÿ��5Ѫ����Ѫ������15)
    // damagePerNotch: ��һ��Ѫ��Ҫ���˺�ֵ (���ᵽ�� 5 �� 8)
    // �޸� create �������������� attack �� range
    static EnemyBuilding* create(const std::string& filename, const std::string& hpBarFilename, int totalHp, int damagePerNotch, int attack, float range);

    // �޸� init ��������
    virtual bool init(const std::string& filename, const std::string& hpBarFilename, int totalHp, int damagePerNotch, int attack, float range);

    // �����������Ĺ����߼����� BattleScene �� update ����
    void updateTowerLogic(float dt, const cocos2d::Vector<Soldier*>& soldiers);

    // ���˺������ⲿ���������������Ѫ
    void takeDamage(int damage);

    // 1. ��ȡ��ǰѪ�� (��ʿ��AI���Ŀ���Ƿ���)
    int getCurrentHp() const { return _currentHp; }

    // ����������齨���Ƿ񱻴ݻ�
    bool isDestroyed() const { return _isDestroyed; }

    void setType(EnemyType type) { _type = type; }
    EnemyType getType() const { return _type; }

private:
    // �����������䵼��
    void fireMissile(Soldier* target);

    int _currentHp;      // ��ǰѪ��
    int _maxHp;          // ��Ѫ��
    int _damagePerNotch; // ��һ��Ѫ���������ֵ
    // ���������ԡ�
    int _attackPower;       // ������
    float _attackRange;     // ������Χ (����)
    float _attackCooldown;  // ������� (��)
    float _attackTimer;     // ��ʱ��
    // ���������ݻ�״̬��־
    bool _isDestroyed;

    cocos2d::Sprite* _healthBar; // Ѫ������

    // �������������ݵ�ǰѪ��ˢ��Ѫ����ʾ
    void updateHealthBar();
    // ���������������ű�ը��Ч�ĺ���
    void playExplosionEffect();
    EnemyType _type = EnemyType::TOWER; // Ĭ��Ϊ��
};

#endif