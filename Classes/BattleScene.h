/**
 * @file       BattleScene.h
 * @brief      战斗场景核心类头文件
 * @details    该文件声明了战斗场景（BattleScene）的核心接口与成员，继承自Cocos2d-x的Scene类，
 *             封装了PVE/PVP双模式战斗逻辑、关卡加载、士兵AI调度、建筑管理、UI交互、触摸事件等核心功能，
 *             是整个战斗流程的主控类，负责协调士兵、敌方建筑、地图等模块的交互
 * @author     （可补充作者信息）
 * @date       （可补充创建日期）
 * @version    1.0
 * @note       该类依赖MapTrap、EnemyBuilding、Soldier等模块；支持扩展新关卡与战斗模式，
 *             私有方法按功能分组，便于维护与扩展；原代码中冗余的类名限定已移除，修复注释乱码问题
 */
#ifndef BATTLE_SCENE_H_
#define BATTLE_SCENE_H_

#include "MapTrap.h"
#include "cocos2d.h"
#include "EnemyBuilding.h"
#include "Soldier.h" // 引入士兵基类头文件

 /**
  * @struct     SoldierUIItem
  * @brief      士兵UI项纯数据结构体
  * @details    定义了士兵选择UI的核心组件信息，用于构建士兵快捷选择界面，关联士兵类型与数量显示，
  *             不包含业务逻辑，仅用于UI组件的统一管理与数据传递
  */
struct SoldierUIItem {
    cocos2d::Sprite* icon;        ///< 士兵图标精灵（显示士兵外观）
    cocos2d::Label* countLabel;   ///< 士兵数量标签（显示剩余可召唤数量）
    SoldierType type;             ///< 士兵类型枚举（关联对应士兵子类）
    int count;                    ///< 士兵剩余可召唤数量
    std::string nameKey;          ///< 数据管理密钥（对应DataManager中的字符串键值）
};

/**
 * @class      BattleScene
 * @brief      战斗场景主控类
 * @details    继承自Cocos2d-x的Scene类，负责战斗场景的整体初始化、帧更新、模块协调与生命周期管理，
 *             支持PVE关卡与PVP玩家对战两种模式，封装了关卡加载、士兵召唤、游戏状态判断、UI交互等全流程逻辑
 * @extends    cocos2d::Scene
 */
class BattleScene : public cocos2d::Scene
{
public:
    /**
     * @enum       BattleMode
     * @brief      战斗模式枚举
     * @details    定义了战斗场景的两种核心运行模式，用于区分不同的战斗逻辑与关卡加载方式，
     *             外部可通过该枚举指定战斗模式，内部根据模式执行差异化流程
     */
    enum class BattleMode {
        CAMPAIGN, ///< PVE模式（闯关玩法，加载内置关卡数据）
        PVP       ///< PVP模式（玩家对战，加载敌方玩家JSON配置数据）
    };

    // ==========================================
    // 静态创建方法（外部创建场景的唯一入口）
    // ==========================================

    /**
     * @brief      静态创建方法（带参数，指定关卡/模式）
     * @details    根据关卡索引与PVP JSON数据，创建对应模式的战斗场景，支持PVE与PVP模式切换
     * @param      levelIndex    PVE关卡索引（仅CAMPAIGN模式有效）
     * @param      pvpJsonData   PVP模式敌方配置JSON字符串（默认空字符串，对应CAMPAIGN模式）
     * @return     cocos2d::Scene*  创建成功返回场景指针；创建失败返回nullptr
     */
    static cocos2d::Scene* createScene(int levelIndex, std::string pvpJsonData = "");

    /**
     * @brief      Cocos2d-x宏定义，自动生成创建实例的相关代码
     * @details    封装了对象创建、初始化与自动内存管理的逻辑，简化场景实例的创建流程
     */
    CREATE_FUNC(BattleScene);

    // ==========================================
    // 生命周期方法（继承自Scene，控制场景生命周期）
    // ==========================================
    /**
     * @brief      场景进入回调函数
     * @details    当场景切换至当前战斗场景时调用，用于执行场景进入后的初始化操作（如UI刷新、状态重置）
     */
    void onEnter();

    /**
     * @brief      场景初始化函数
     * @details    完成战斗场景的核心初始化流程，包括地图加载、组件创建、事件注册等，
     *             是场景内部初始化的入口函数
     * @return     bool  初始化成功返回true；初始化失败返回false
     */
    virtual bool init();

    /**
     * @brief      场景帧更新函数（重写父类方法）
     * @details    每帧调用，负责驱动士兵AI、敌方建筑、游戏状态等模块的更新，是战斗逻辑的核心调度入口
     * @param      dt  帧间隔时间（秒），用于时间相关逻辑计算（如冷却计时、移动距离）
     * @override   cocos2d::Node::update
     */
    virtual void update(float dt) override;

    // ==========================================
    // 公共业务接口（供外部模块调用）
    // ==========================================
    /**
     * @brief      获取敌方防御塔列表（供士兵AI调用）
     * @details    返回场景中所有敌方防御塔的引用，用于士兵寻靶逻辑，获取有效攻击目标
     * @return     cocos2d::Vector<EnemyBuilding*>&  敌方防御塔向量引用
     */
    cocos2d::Vector<EnemyBuilding*>& getTowers() { return _towers; }

    /**
     * @brief      获取敌方大本营（供士兵AI调用）
     * @details    返回场景中敌方大本营的指针，是士兵的核心攻击目标，摧毁后判定战斗胜利
     * @return     EnemyBuilding*  敌方大本营指针；无有效大本营时返回nullptr
     */
    EnemyBuilding* getBase() { return _base; }

    /**
     * @brief      初始化战斗场景（指定关卡/模式）
     * @details    根据传入的关卡索引与PVP JSON数据，初始化战斗模式并加载对应关卡资源，
     *             是战斗场景业务初始化的核心接口
     * @param      levelIndex    PVE关卡索引
     * @param      pvpJsonData   PVP模式敌方配置JSON字符串
     */
    void setupBattle(int levelIndex, std::string pvpJsonData);

    /**
     * @brief      判定指定世界坐标是否被阻挡（供士兵寻路调用）
     * @details    校验传入的世界坐标是否在禁止移动区域内，用于士兵陆军的寻路阻挡判断，避免穿透地形/建筑
     * @param      worldPos  世界坐标点
     * @return     bool  被阻挡返回true；可通行返回false
     */
    bool isPositionBlocked(cocos2d::Vec2 worldPos);

    // ==========================================
    // 私有成员变量（按功能分组，关联成员集中摆放）
    // ==========================================
private:
    // 地图相关成员
    cocos2d::TMXTiledMap* _tileMap;                ///< TMX地图节点（承载战斗场景地图资源）
    std::string _mapFileName;                      ///< 地图文件名（存储当前加载的地图名称，用于后续重加载）
    std::vector<cocos2d::Rect> _forbiddenRects;    ///< 禁止移动区域矩形列表（用于陆军寻路阻挡判断）
    cocos2d::Vector<MapTrap*> _traps;              ///< 地图陷阱列表（承载场景中的陷阱组件）

    // 战斗核心成员
    BattleMode _currentMode;                       ///< 当前战斗模式（PVE/PVP）
    cocos2d::Vector<EnemyBuilding*> _towers;       ///< 敌方防御塔列表（含加农炮、弓箭塔等防御建筑）
    EnemyBuilding* _base;                          ///< 敌方大本营指针（核心攻击目标）
    cocos2d::Vector<Soldier*> _soldiers;           ///< 己方士兵列表（存储所有已召唤的士兵实例）

    // UI相关成员
    std::vector<SoldierUIItem*> _soldierUIList;    ///< 士兵UI项列表（构建士兵选择界面）
    bool _isPlacingMode;                           ///< 士兵放置模式标记（true=可放置士兵，false=不可放置）
    cocos2d::DrawNode* _forbiddenAreaNode;         ///< 禁止区域绘制节点（用于渲染红色禁止移动区域）
    cocos2d::Label* _msgLabel;                     ///< 提示信息标签（显示警告、提示等文本信息）
    SoldierType _currentSelectedType;              ///< 当前选中的士兵类型（用于召唤对应士兵）
    SoldierUIItem* _currentSelectedItem;           ///< 当前选中的士兵UI项（关联UI组件与士兵数据）

    // 士兵召唤相关成员
    int _spawnCount;                               ///< 已召唤士兵计数（用于限制最大召唤数量）
    const int MAX_SPAWN = 10;                      ///< 最大士兵召唤数量（固定阈值，限制战场士兵数量）
    cocos2d::Vec2 _currentTouchPos;                ///< 当前触摸坐标（用于士兵放置定位）
    bool _isTouchingMap;                           ///< 地图触摸标记（true=正在触摸地图，false=未触摸）

    // 游戏状态相关成员
    bool _isGameOver;                              ///< 游戏结束标记（true=游戏已结束，false=游戏进行中）
    bool _isGamePaused;                            ///< 游戏暂停标记（true=游戏已暂停，false=游戏正常运行）

    // ==========================================
    // 私有业务方法（按功能分组，关联方法集中摆放）
    // ==========================================

    // 关卡加载方法
    /**
     * @brief      加载PVE关卡
     * @details    根据关卡索引，加载内置的PVE关卡数据（地图、敌方建筑布局等）
     * @param      levelIndex  PVE关卡索引（对应内置关卡配置）
     */
    void loadLevelCampaign(int levelIndex);

    /**
     * @brief      加载PVP关卡
     * @details    根据传入的JSON字符串，加载敌方玩家的建筑布局与配置数据，构建PVP对战场景
     * @param      json  PVP敌方配置JSON字符串
     */
    void loadLevelPVP(const std::string& json);

    // UI创建与回调方法
    /**
     * @brief      创建战斗场景UI
     * @details    构建士兵选择UI、提示信息UI、返回菜单UI等所有战斗场景所需UI组件，注册UI事件回调
     */
    void createUI();

    /**
     * @brief      士兵图标点击回调
     * @details    当点击士兵选择UI图标时触发，切换当前选中的士兵类型，进入士兵放置模式
     * @param      item  被点击的士兵UI项指针
     */
    void onSoldierIconClicked(SoldierUIItem* item);

    // 士兵召唤方法
    /**
     * @brief      尝试召唤士兵
     * @details    根据当前选中的士兵类型与触摸位置，校验召唤条件（数量充足、位置可放置），满足条件则召唤士兵
     * @param      worldPos  士兵召唤的世界坐标
     */
    void trySpawnSoldier(cocos2d::Vec2 worldPos);

    /**
     * @brief      士兵召唤调度器
     * @details    定时调度士兵召唤逻辑，用于批量召唤士兵或延迟召唤士兵，控制召唤节奏
     * @param      dt  调度间隔时间（秒）
     */
    void spawnScheduler(float dt);

    // 提示信息方法
    /**
     * @brief      显示警告信息
     * @details    在场景中显示指定的警告文本，一段时间后自动隐藏，用于提示玩家非法操作等信息
     * @param      msg  警告文本内容
     */
    void showWarning(const std::string& msg);

    // 游戏状态判断方法
    /**
     * @brief      检查游戏是否结束
     * @details    每帧校验游戏胜利/失败条件（大本营是否被摧毁、所有敌方建筑是否被清除），触发对应结束逻辑
     */
    void checkGameEnd();

    /**
     * @brief      显示胜利弹窗
     * @details    当战斗胜利时，显示胜利提示弹窗，包含奖励信息与返回菜单按钮
     */
    void showVictoryPopup();

    /**
     * @brief      显示失败弹窗
     * @details    当战斗失败时，显示失败提示弹窗，包含重新挑战与返回菜单按钮
     */
    void showDefeatPopup();

    /**
     * @brief      隐藏胜利弹窗
     * @details    隐藏胜利提示弹窗，用于弹窗关闭或场景切换前的清理操作
     */
    void hideVictoryPopup();

    // 触摸事件方法
    /**
     * @brief      触摸开始回调
     * @details    当玩家触摸屏幕时触发，记录触摸坐标，标记触摸状态，进入士兵放置预览模式
     * @param      touch  触摸对象指针
     * @param      event  事件对象指针
     * @return     bool  消费触摸事件返回true；不消费返回false
     */
    bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event);

    /**
     * @brief      触摸移动回调
     * @details    当玩家在屏幕上移动触摸点时触发，更新当前触摸坐标，用于士兵放置位置预览
     * @param      touch  触摸对象指针
     * @param      event  事件对象指针
     */
    void onTouchMoved(cocos2d::Touch* touch, cocos2d::Event* event);

    /**
     * @brief      触摸结束回调
     * @details    当玩家结束触摸屏幕时触发，校验触摸位置，尝试召唤士兵，重置触摸状态
     * @param      touch  触摸对象指针
     * @param      event  事件对象指针
     */
    void onTouchEnded(cocos2d::Touch* touch, cocos2d::Event* event);

    // 菜单回调方法
    /**
     * @brief      返回游戏主场景回调
     * @details    当点击返回菜单按钮时触发，切换回游戏主场景，释放当前战斗场景资源
     * @param      pSender  按钮触发对象指针
     */
    void menuBackToGameScene(cocos2d::Ref* pSender);
};

#endif // BATTLE_SCENE_H_