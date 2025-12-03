#ifndef __BATTLE_SCENE_H__
#define __BATTLE_SCENE_H__

#include "cocos2d.h"

class BattleScene : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();
    virtual bool init();

    // 返回 GameScene 的回调函数
    void menuBackToGameScene(cocos2d::Ref* pSender);

    // 【新增】一个用于加载敌方地图和建筑的函数
    void loadEnemyMap();

    CREATE_FUNC(BattleScene);
private:
    static const int MAP_WIDTH = 16;
    static const int MAP_HEIGHT = 10;
};

#endif // __BATTLE_SCENE_H__