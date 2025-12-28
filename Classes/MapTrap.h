#ifndef MAP_TRAP_H_
#define MAP_TRAP_H_

#include "cocos2d.h"
#include "Soldier.h"

class MapTrap : public cocos2d::Node // 继承 Node 不是 Sprite，因为平时它是隐形的
{
public:
    // create 函数：传入触发区域和伤害
    static MapTrap* create(const cocos2d::Rect& area, int damage);
    //初始化函数
    bool init(const cocos2d::Rect& area, int damage);

    // 检测触发：传入所有士兵，判断有没有人踩中
    bool checkTrigger(const cocos2d::Vector<Soldier*>& soldiers);

private:
    cocos2d::Rect trapArea; // 陷阱生效的矩形区域
    int damage;             // 伤害值
    bool isExploded;        // 是否已经爆炸过

    void explode(const cocos2d::Vector<Soldier*>& soldiers); // 爆炸逻辑
    void playExplosionEffect(); // 播放特效
};

#endif