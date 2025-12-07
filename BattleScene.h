// BattleScene.h
#ifndef __BATTLE_SCENE_H__
#define __BATTLE_SCENE_H__

#include "cocos2d.h"
#include "EnemyBuilding.h"
#include "Soldier.h" // 需要包含 Soldire 头文件

class BattleScene : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();
    virtual bool init();
    virtual void update(float dt) override;
    CREATE_FUNC(BattleScene);

    // 供士兵 AI 使用的公共接口
    cocos2d::Vector<EnemyBuilding*>& getTowers() { return _towers; }
    EnemyBuilding* getBase() { return _base; }

    // 碰撞检测接口 (用于放置检测和未来可能的寻路)
    bool isPositionBlocked(cocos2d::Vec2 worldPos);

private:
    cocos2d::TMXTiledMap* _tileMap;

    // 游戏对象列表
    cocos2d::Vector<EnemyBuilding*> _towers; // 所有防御塔
    EnemyBuilding* _base;                  // 大本营
    cocos2d::Vector<Soldier*> _soldiers;    // 所有士兵

    // 障碍物区域 (世界坐标系 Rect，用于检测放置位置和绘制红色区域)
    std::vector<cocos2d::Rect> _forbiddenRects; // <--- C2065: _forbiddenRects 的声明

    // UI & 放置逻辑
    cocos2d::Sprite* _soldierIcon;
    bool _isPlacingMode;
    cocos2d::DrawNode* _forbiddenAreaNode; // 用于绘制红色警告区
    cocos2d::Label* _msgLabel;

    // 放置计数和计时器相关
    int _spawnCount;
    const int MAX_SPAWN = 10;
    cocos2d::Vec2 _currentTouchPos;
    bool _isTouchingMap;

    // TMX & 实体加载
    void loadEnemyMap();

    // UI & 交互回调
    void createUI();
    void onSoldierIconClicked();
    void trySpawnSoldier(cocos2d::Vec2 worldPos);
    void showWarning(const std::string& msg);
    void spawnScheduler(float dt);

    // 触摸事件
    bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event);
    void onTouchMoved(cocos2d::Touch* touch, cocos2d::Event* event);
    void onTouchEnded(cocos2d::Touch* touch, cocos2d::Event* event);

    // 【返回函数】 <--- C2039/C2065: menuBackToGameScene 的声明
    void menuBackToGameScene(cocos2d::Ref* pSender);
};

#endif // __BATTLE_SCENE_H__