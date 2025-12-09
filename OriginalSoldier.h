#ifndef __ORIGINAL_SOLDIER_H__
#define __ORIGINAL_SOLDIER_H__

#include "Soldier.h"

class OriginalSoldier : public Soldier
{
public:
    virtual bool init(BattleScene* battleScene, SoldierType type) override;

protected:
    virtual void setupProperties(SoldierType type) override;
    virtual void playWalkAnim() override;
    virtual void stopAnim() override;

private:
    // ¼ÙÉèÍ¼Æ¬Â·¾¶Îª anim/man1.png, anim/man2.png
    const std::string WALK_ANIM_BASE = "anim/man";
};

#endif // __ORIGINAL_SOLDIER_H__