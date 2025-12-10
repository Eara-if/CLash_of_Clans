#ifndef __ENEMY_BUILDING_H__
#define __ENEMY_BUILDING_H__

#include "cocos2d.h"
// 2. 【新增】前向声明：告诉编译器 Soldier 是一个类，具体细节等会儿再说
class Soldier;

class EnemyBuilding : public cocos2d::Sprite
{
public:
    // 初始化函数
    // filename: 建筑本身的图片 (如 Base.png)
    // hpBarFilename: 血条的图片 (如 Heart2.png)
    // totalHp: 总血量 (比如 3格血，每格5血，总血量就是15)
    // damagePerNotch: 掉一格血需要的伤害值 (你提到的 5 或 8)
    // 修改 create 函数声明，增加 attack 和 range
    static EnemyBuilding* create(const std::string& filename, const std::string& hpBarFilename, int totalHp, int damagePerNotch, int attack, float range);

    // 修改 init 函数声明
    virtual bool init(const std::string& filename, const std::string& hpBarFilename, int totalHp, int damagePerNotch, int attack, float range);

    // 【新增】核心攻击逻辑，由 BattleScene 的 update 调用
    void updateTowerLogic(float dt, const cocos2d::Vector<Soldier*>& soldiers);

    // 受伤函数：外部调用这个函数来扣血
    void takeDamage(int damage);

    // 1. 获取当前血量 (供士兵AI检查目标是否存活)
    int getCurrentHp() const { return _currentHp; }

    // 【新增】检查建筑是否被摧毁
    bool isDestroyed() const { return _isDestroyed; }

private:
    // 【新增】发射导弹
    void fireMissile(Soldier* target);

    int _currentHp;      // 当前血量
    int _maxHp;          // 总血量
    int _damagePerNotch; // 掉一格血代表多少数值
    // 【新增属性】
    int _attackPower;       // 攻击力
    float _attackRange;     // 攻击范围 (像素)
    float _attackCooldown;  // 攻击间隔 (秒)
    float _attackTimer;     // 计时器
    // 【新增】摧毁状态标志
    bool _isDestroyed;

    cocos2d::Sprite* _healthBar; // 血条精灵

    // 辅助函数：根据当前血量刷新血条显示
    void updateHealthBar();
    // 【新增】声明播放爆炸特效的函数
    void playExplosionEffect();
};

#endif