#ifndef __SOLDIER_H__
#define __SOLDIER_H__

#include "cocos2d.h"
#include <vector>
#include "EnemyBuilding.h" // 需要包含 EnemyBuilding 头文件

USING_NS_CC;
class BattleScene; // 前向声明，避免交叉引用

// 定义兵种类型
enum class SoldierType
{
    NONE,
    GIANT,      // 巨人士兵
    ARCHER,     // 弓箭手
    // TODO: 添加其他兵种
};

class Soldier : public Sprite
{
public:
    // 统一的创建接口（工厂方法）
    static Soldier* create(BattleScene* battleScene, SoldierType type);

    // 初始化逻辑：现在负责加载通用资源和启动 Update
    virtual bool init(BattleScene* battleScene, SoldierType type);

    // AI 逻辑 (所有兵种通用)
    void update(float dt);
    void findNewTarget();
    void moveLogic(float dt);
    void attackLogic(float dt);

    // 通用战斗逻辑
    void takeDamage(int damage);
    int getCurrentHp() const { return _currentHp; }

protected:
    // AI 状态枚举
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

    // 血条成员
    Sprite* _healthBar;
    int _damagePerNotch; // 掉一格血需要的伤害

    // 纯虚函数：让子类实现特定的属性和动画 
    virtual void setupProperties(SoldierType type) = 0;
    virtual void playWalkAnim() = 0;
    virtual void stopAnim() = 0;

    // 血条逻辑（基类实现，但为虚函数，允许子类重写）
    virtual void setupHealthBar();
    virtual void updateHealthBar();
};

#endif // __SOLDIER_H__