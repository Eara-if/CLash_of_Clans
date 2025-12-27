#ifndef ORIGINAL_SOLDIER_H_
#define ORIGINAL_SOLDIER_H_

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

#endif // ORIGINAL_SOLDIER_H_