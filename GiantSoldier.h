#pragma once

#include "Soldier.h"

class GiantSoldier : public Soldier
{
public:

    // 重载 init，用于设置子类特有的初始值，然后调用基类 init
    virtual bool init(BattleScene* battleScene, SoldierType type) override;

protected:
    // 核心：实现基类的纯虚函数 
    virtual void setupProperties(SoldierType type) override;
    virtual void playWalkAnim() override;
    virtual void stopAnim() override;

private:
    // 巨人士兵特有的动画帧名
    const std::string WALK_ANIM_BASE = "anim/giant";
};
