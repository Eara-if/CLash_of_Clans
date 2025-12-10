#ifndef __BOOM_SOLDIER_H__
#define __BOOM_SOLDIER_H__

#include "Soldier.h"

class BoomSoldier : public Soldier
{
public:
    virtual bool init(BattleScene* battleScene, SoldierType type) override;

protected:
    virtual void setupProperties(SoldierType type) override;
    virtual void playWalkAnim() override;
    virtual void stopAnim() override;
    // 【新增】重写攻击函数，实现自爆逻辑
    virtual void attackTarget(EnemyBuilding* target) override; 

private:
    const std::string WALK_ANIM_BASE = "anim/boom";
};

#endif // __BOOM_SOLDIER_H__