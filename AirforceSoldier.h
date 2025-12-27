#ifndef AIRFORCE_SOLDIER_H_
#define AIRFORCE_SOLDIER_H_

#include "Soldier.h"

class AirforceSoldier : public Soldier
{
public:
    virtual bool init(BattleScene* battleScene, SoldierType type) override;

    // 重写属性设置
    virtual void setupProperties(SoldierType type) override;
    // 重写动画
    virtual void playWalkAnim() override;
    virtual void stopAnim() override;

    // 【核心】重写是否飞行，返回 true
    virtual bool isFlying() const override { return true; }

    // 可选：重写攻击特效（如果需要发射东西，目前暂用基类近战逻辑）
    // virtual void attackTarget(EnemyBuilding* target) override; 
};

#endif