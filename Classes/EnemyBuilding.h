#ifndef ENEMY_BUILDING_H_
#define ENEMY_BUILDING_H_

#include"SharedData.h"
#include "cocos2d.h"
class Soldier;
class EnemyBuilding : public cocos2d::Sprite
{
public:
    void setHealthBarScale(float s);
    static EnemyBuilding* create(const std::string& filename, const std::string& hpBarFilename, int totalHp, int damagePerNotch, int attack, float range);
    virtual bool init(const std::string& filename, const std::string& hpBarFilename, int totalHp, int damagePerNotch, int attack, float range);
    void updateTowerLogic(float dt, const cocos2d::Vector<Soldier*>& soldiers);
    void takeDamage(int damage);

    //获取当前血量值
    int getCurrentHp() const 
    { 
        return _currentHp; 
    }
    //获取建筑是否被摧毁的信息
    bool isDestroyed() const
    { 
        return _isDestroyed; 
    }
    //设置当前建筑类型
    void setType(EnemyType type) 
    { 
        _type = type; 
    }
    //读取当前建筑类型
    EnemyType getType() const 
    { 
        return _type; 
    }
    //添加设置类型的方法
    void setEnemyType(EnemyType type)
    { 
        _type = type; 
    }

private:
    void fireMissile(Soldier* target);
    //当前血量
    int _currentHp;      
    //最大血量
    int _maxHp;        
    //每个血条刻度的血量值
    int _damagePerNotch; 
    //攻击力
    int _attackPower;     
    //攻击范围
    float _attackRange;  
    //攻击间隔
    float _attackCooldown;  
    //计时器，等待下一次攻击的时间
    float _attackTimer;     
    //摧毁标志
    bool _isDestroyed;
    //创建一个血条对象
    cocos2d::Sprite* _healthBar; 

    void updateHealthBar();
    void playExplosionEffect();
    //初始化建筑类别
    EnemyType _type = EnemyType::TOWER; 
};

#endif