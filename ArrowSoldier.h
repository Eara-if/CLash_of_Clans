#pragma once

#include "Soldier.h"

class ArrowSoldier : public Soldier
{
public:
    virtual bool init(BattleScene* battleScene, SoldierType type) override;

protected:
    virtual void setupProperties(SoldierType type) override;
    virtual void playWalkAnim() override;
    virtual void stopAnim() override;
    virtual void attackTarget(EnemyBuilding* target) override;

private:
    const std::string WALK_ANIM_BASE = "anim/arrow";
};

