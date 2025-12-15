// BattleScene.h
#ifndef __BATTLE_SCENE_H__
#define __BATTLE_SCENE_H__

#include "cocos2d.h"
#include "EnemyBuilding.h"
#include "Soldier.h" // 需要包含 Soldire 头文件

// 定义一个简单的结构体来管理底部的兵种UI
struct SoldierUIItem {
    cocos2d::Sprite* icon;        // 图标
    cocos2d::Label* countLabel;   // 数量文字
    SoldierType type;             // 兵种类型
    int count;                    // 剩余数量
    std::string nameKey;          // 对应 DataManager 的字符串key
};

class BattleScene : public cocos2d::Scene
{
public:
    void onEnter();
    static cocos2d::Scene* createScene();
    virtual bool init();
    virtual void update(float dt) override;
    CREATE_FUNC(BattleScene);


    // 供士兵 AI 使用的公共接口
    cocos2d::Vector<EnemyBuilding*>& getTowers() { return _towers; }
    EnemyBuilding* getBase() { return _base; }
    static cocos2d::Scene* createScene(const std::string& mapFileName); // 【修改】接受地图名参数

    // 碰撞检测接口 (用于放置检测和未来可能的寻路)
    bool isPositionBlocked(cocos2d::Vec2 worldPos);
    // 【新增】自定义初始化方法
    bool initWithMap(const std::string& mapFileName);

private:
    cocos2d::TMXTiledMap* _tileMap;
    std::string _mapFileName; // 【新增】用于存储要加载的地图文件名
    void setMapFileName(const std::string& fileName); // 【新增】设置地图文件名

    // 游戏对象列表
    cocos2d::Vector<EnemyBuilding*> _towers; // 所有防御塔
    EnemyBuilding* _base;                  // 大本营
    cocos2d::Vector<Soldier*> _soldiers;    // 所有士兵

    // 障碍物区域 (世界坐标系 Rect，用于检测放置位置和绘制红色区域)
    std::vector<cocos2d::Rect> _forbiddenRects; // 

    // UI & 放置逻辑
    std::vector<SoldierUIItem*> _soldierUIList;
    bool _isPlacingMode;
    cocos2d::DrawNode* _forbiddenAreaNode; // 用于绘制红色警告区
    cocos2d::Label* _msgLabel;
    // 【新增】当前选中的兵种类型
    SoldierType _currentSelectedType;
    SoldierUIItem* _currentSelectedItem; // 当前选中的UI项指针
    // 放置计数和计时器相关
    int _spawnCount;
    const int MAX_SPAWN = 10;
    cocos2d::Vec2 _currentTouchPos;
    bool _isTouchingMap;

    // 【新增】游戏状态
    bool _isGameOver;
    bool _isGamePaused;

    // TMX & 实体加载
    void loadEnemyMap();

    // UI & 交互回调
    void createUI();
    void BattleScene::onSoldierIconClicked(SoldierUIItem* item);
    void trySpawnSoldier(cocos2d::Vec2 worldPos);
    void showWarning(const std::string& msg);
    void spawnScheduler(float dt);

    // 【新增】游戏结束逻辑
    void checkGameEnd();
    void showVictoryPopup();
    void hideVictoryPopup();

    // 触摸事件
    bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event);
    void onTouchMoved(cocos2d::Touch* touch, cocos2d::Event* event);
    void onTouchEnded(cocos2d::Touch* touch, cocos2d::Event* event);

    // 【返回函数】 <--- C2039/C2065: menuBackToGameScene 的声明
    void menuBackToGameScene(cocos2d::Ref* pSender);
};

#endif // __BATTLE_SCENE_H__