#ifndef __ENEMY_BUILDING_H__
#define __ENEMY_BUILDING_H__

#include "cocos2d.h"

class EnemyBuilding : public cocos2d::Sprite
{
public:
    // 初始化函数
    // filename: 建筑本身的图片 (如 Base.png)
    // hpBarFilename: 血条的图片 (如 Heart2.png)
    // totalHp: 总血量 (比如 3格血，每格5血，总血量就是15)
    // damagePerNotch: 掉一格血需要的伤害值 (你提到的 5 或 8)
    static EnemyBuilding* create(const std::string& filename, const std::string& hpBarFilename, int totalHp, int damagePerNotch);

    virtual bool init(const std::string& filename, const std::string& hpBarFilename, int totalHp, int damagePerNotch);

    // 受伤函数：外部调用这个函数来扣血
    void takeDamage(int damage);

private:
    int _currentHp;      // 当前血量
    int _maxHp;          // 总血量
    int _damagePerNotch; // 掉一格血代表多少数值

    cocos2d::Sprite* _healthBar; // 血条精灵

    // 辅助函数：根据当前血量刷新血条显示
    void updateHealthBar();
};

#endif