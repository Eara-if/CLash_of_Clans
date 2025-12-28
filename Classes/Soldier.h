/**
 * @file       Soldier.h
 * @brief      士兵抽象基类（Soldier）的头文件定义
 * @details    该文件声明了所有士兵子类的通用接口、枚举类型、核心成员变量与通用业务逻辑，
 *             作为抽象基类（包含纯虚函数），提供了士兵的通用AI逻辑（寻路、攻击、移动）、
 *             战斗逻辑（受击、血条管理）与生命周期管理，所有具体士兵子类需继承该类并实现纯虚函数
 * @version    1.0
 * @note       该类依赖 Cocos2d-x 引擎的 Sprite 组件及 EnemyBuilding、BattleScene 相关头文件；
 *             类内包含纯虚函数，不可直接实例化，需通过子类实现并调用静态 create 方法创建实例；
 *             飞行单位判断、攻击逻辑等接口支持子类重写，实现差异化特性
 */
#ifndef SOLDIER_H_
#define SOLDIER_H_

#include "cocos2d.h"
#include <vector>
#include "EnemyBuilding.h" // 引入敌方建筑头文件，支持士兵攻击目标的定义

USING_NS_CC;
class BattleScene; // 前向声明战斗场景类，避免循环包含

/**
 * @enum       SoldierType
 * @brief      士兵类型枚举
 * @details    定义了所有支持的士兵类型标识，用于工厂方法创建对应士兵实例，
 *             每个枚举值对应一个具体士兵子类，便于统一管理与扩展
 */
enum SoldierType {
    ORIGINAL = 0,  ///< 原始士兵（基础步兵）
    ARROW = 1,     ///< 弓箭士兵（远程单位）
    BOOM = 2,      ///< 自爆士兵（近战自爆单位）
    GIANT = 3,     ///< 巨人士兵（肉盾近战单位）
    AIRFORCE = 4   ///< 空军士兵（飞行单位）
};

/**
 * @class      Soldier
 * @brief      士兵抽象基类，提供所有士兵的通用核心逻辑
 * @details    继承自 Cocos2d-x 的 Sprite 类，封装了士兵的通用AI（寻靶、移动、攻击）、
 *             战斗（受击、血条）、状态管理等逻辑，通过纯虚函数约束子类实现专属特性（属性、动画），
 *             支持子类重写虚函数实现差异化功能（如飞行判断、攻击特效）
 * @extends    cocos2d::Sprite
 * @note       该类为抽象基类，不可直接实例化；通过静态 create 方法实现工厂模式，创建具体士兵实例
 */
class Soldier : public Sprite
{
public:
    /**
     * @brief      静态工厂方法，用于创建具体士兵实例
     * @details    根据传入的士兵类型，创建对应子类的实例并完成初始化，
     *             是外部创建士兵对象的唯一入口，实现了士兵类型的统一管理
     * @param      battleScene  战斗场景指针，关联士兵所在的战斗场景上下文
     * @param      type         士兵类型枚举（SoldierType），指定要创建的士兵类型
     * @return     Soldier*     创建成功返回士兵实例指针；创建失败返回 nullptr
     */
    static Soldier* create(BattleScene* battleScene, SoldierType type);

    /**
     * @brief      士兵初始化核心函数（虚函数）
     * @details    完成士兵的通用初始化流程，包括场景关联、类型赋值、状态初始化等，
     *             子类可重写该函数扩展初始化逻辑，但需优先调用父类实现
     * @param      battleScene  战斗场景指针，关联士兵所在的战斗场景上下文
     * @param      type         士兵类型枚举（SoldierType），当前士兵的具体类型
     * @return     bool         初始化成功返回 true；初始化失败返回 false
     */
    virtual bool init(BattleScene* battleScene, SoldierType type);

    /**
     * @brief      虚函数：判断当前士兵是否为飞行单位
     * @details    默认返回 false（地面单位），飞行士兵子类可重写该函数返回 true，
     *             战斗场景可根据该返回值实现差异化逻辑（如无视地面围墙、被对空建筑锁定）
     * @return     bool         默认返回 false（地面单位）；飞行单位返回 true
     * @const      该函数为只读函数，不修改类的任何成员变量
     */
    virtual bool isFlying() const { return false; }

    /**
     * @brief      获取当前士兵的类型
     * @details    返回士兵初始化时赋值的类型标识，用于外部判断士兵类型，实现差异化处理
     * @return     SoldierType  当前士兵的类型枚举值
     * @const      该函数为只读函数，不修改类的任何成员变量
     */
    SoldierType getSoldierType() const { return _soldierType; }

    /**
     * @brief      士兵AI主更新函数
     * @details    每帧调用，统一驱动士兵的状态更新、移动逻辑、攻击逻辑，
     *             是士兵AI的入口函数，由战斗场景统一调度
     * @param      dt  帧间隔时间（秒），用于时间相关的逻辑计算（如移动距离、攻击计时）
     */
    void update(float dt);

    /**
     * @brief      寻找新的攻击目标
     * @details    当当前目标失效（被摧毁、超出范围）时，调用该函数寻找最近的有效敌方建筑作为新目标，
     *             找到目标后更新士兵状态为攻击或移动状态
     */
    void findNewTarget();

    /**
     * @brief      士兵移动逻辑
     * @details    每帧调用（仅当士兵处于移动状态时有效），根据目标位置与移动速度计算移动距离，
     *             更新士兵的位置，当到达攻击范围后切换为攻击状态
     * @param      dt  帧间隔时间（秒），用于计算本次帧的移动距离
     */
    void moveLogic(float dt);

    /**
     * @brief      士兵攻击逻辑
     * @details    每帧调用（仅当士兵处于攻击状态时有效），通过攻击计时器判断是否达到攻击间隔，
     *             达到间隔后调用 attackTarget 函数触发攻击，并重置攻击计时器
     * @param      dt  帧间隔时间（秒），用于更新攻击计时器
     */
    void attackLogic(float dt);

    /**
     * @brief      士兵受击逻辑
     * @details    处理士兵受到伤害的流程，扣除对应生命值，更新血条显示，
     *             当生命值低于或等于0时，触发士兵销毁逻辑（由子类或战斗场景实现）
     * @param      damage  本次受到的伤害值
     */
    void takeDamage(int damage);

    /**
     * @brief      获取士兵当前生命值
     * @details    返回士兵当前剩余的生命值，用于外部判断士兵是否存活、是否需要触发特殊逻辑
     * @return     int  当前生命值
     * @const      该函数为只读函数，不修改类的任何成员变量
     */
    int getCurrentHp() const { return _currentHp; }

    /**
     * @brief      寻找最近的围墙建筑
     * @details    遍历战斗场景中的围墙建筑，计算并返回距离当前士兵最近的围墙指针，
     *             用于士兵优先攻击围墙的AI逻辑
     * @return     EnemyBuilding*  最近的围墙建筑指针；无围墙时返回 nullptr
     */
    EnemyBuilding* findNearestWall();

protected:
    SoldierType _soldierType; ///< 士兵类型标识，在 init 函数中赋值，用于区分士兵类型

    /**
     * @enum       State
     * @brief      士兵状态枚举（类内枚举）
     * @details    定义了士兵的三种核心状态，用于状态机管理，控制士兵的行为逻辑切换
     */
    enum class State {
        IDLE,      ///< 闲置状态：无目标，等待新目标出现
        MOVING,    ///< 移动状态：有目标但未进入攻击范围，向目标移动
        ATTACKING  ///< 攻击状态：已进入目标攻击范围，持续攻击目标
    };

    BattleScene* _battleScene;    ///< 战斗场景指针，关联当前士兵所在的战斗场景，用于获取场景数据
    EnemyBuilding* _target;       ///< 当前攻击目标指针，指向被锁定的敌方建筑，为空时士兵处于闲置状态

    // 士兵核心战斗属性
    int _maxHp;                   ///< 最大生命值，决定士兵的抗伤能力
    int _currentHp;               ///< 当前生命值，初始值等于最大生命值，受击后扣除
    int _attackDamage;            ///< 攻击伤害值，每次攻击对目标造成的伤害量
    float _attackRange;           ///< 攻击范围（像素），进入该范围后士兵切换为攻击状态
    float _attackInterval;        ///< 攻击间隔（秒），两次攻击之间的最小时间间隔
    float _attackTimer;           ///< 攻击计时器，用于判断是否达到攻击间隔
    float _moveSpeed;             ///< 移动速度（像素/秒），决定士兵的移动快慢
    State _state;                 ///< 士兵当前状态，控制士兵的行为逻辑

    // 血条相关成员
    Sprite* _healthBar;           ///< 血条精灵指针，用于显示士兵当前生命值状态
    int _damagePerNotch;          ///< 血条每格对应的伤害值，用于血条分段显示

    /**
     * @brief      纯虚函数：配置士兵专属核心属性
     * @details    子类必须实现该函数，用于初始化自身的专属属性（如生命值、攻击力、移动速度），
     *             是士兵差异化特性的核心实现接口
     * @param      type  士兵类型枚举（SoldierType），可用于子类差异化配置
     */
    virtual void setupProperties(SoldierType type) = 0;

    /**
     * @brief      纯虚函数：播放士兵行走/专属移动动画
     * @details    子类必须实现该函数，用于加载并播放自身的移动动画（如行走、飞行），
     *             实现士兵的视觉差异化
     */
    virtual void playWalkAnim() = 0;

    /**
     * @brief      纯虚函数：停止士兵行走/专属移动动画
     * @details    子类必须实现该函数，用于停止自身的移动动画，避免无效动画残留，
     *             与 playWalkAnim 函数一一对应
     */
    virtual void stopAnim() = 0;

    /**
     * @brief      虚函数：初始化士兵血条
     * @details    实现士兵血条的通用初始化逻辑，包括血条精灵创建、位置设置、初始显示等，
     *             子类可重写该函数扩展血条样式（如不同颜色、不同位置）
     */
    virtual void setupHealthBar();

    /**
     * @brief      虚函数：更新士兵血条显示
     * @details    根据士兵当前生命值与最大生命值，更新血条的显示长度，
     *             直观展示士兵的存活状态，子类可重写该函数实现血条的差异化更新逻辑
     */
    virtual void updateHealthBar();

    /**
     * @brief      虚函数：士兵攻击目标的核心逻辑
     * @details    实现士兵攻击的通用逻辑（触发目标受击），子类可重写该函数扩展攻击特效（如射箭、自爆），
     *             实现士兵的攻击差异化
     * @param      target  被攻击的敌方建筑指针，为空时不执行攻击逻辑
     */
    virtual void attackTarget(EnemyBuilding* target);
};

#endif // SOLDIER_H_