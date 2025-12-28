/**
 * @file       AirforceSoldier.h
 * @brief      空军士兵（AirforceSoldier）类的头文件定义
 * @details    该文件声明了空军士兵的核心接口与成员函数，空军士兵继承自基础士兵类（Soldier），
 *             核心特性是具备飞行能力（通过重写`isFlying`函数标识），同时重写了父类的初始化、
 *             属性配置及动画相关接口，可独立于地面单位实现地形穿越（如无视围墙等障碍物）
 * @version    1.0
 * @note       该类依赖 Soldier 基类和 BattleScene 相关头文件，使用前需确保依赖文件已正确引入；
 *             飞行能力的具体业务逻辑（如碰撞检测豁免）需在战斗场景中配合`isFlying`返回值实现
 */
#ifndef AIRFORCE_SOLDIER_H_
#define AIRFORCE_SOLDIER_H_

#include "Soldier.h"  // 引入基础士兵基类头文件，提供通用士兵接口

 /**
  * @class      AirforceSoldier
  * @brief      空军士兵类，实现具备飞行能力的士兵核心逻辑
  * @details    继承自 Soldier 基类，重写了父类的初始化、属性配置、动画播放/停止等虚函数，
  *             核心差异化特性是通过`isFlying`函数返回`true`，标识该士兵为飞行单位，
  *             可用于区分地面单位与空中单位，实现不同的战斗规则（如无视地面障碍物、被对空建筑优先攻击等）
  * @extends    Soldier
  * @note       该类支持进一步继承扩展（保护成员可被子类访问），若无需继承可将保护成员改为私有成员
  */
class AirforceSoldier : public Soldier
{
public:
    /**
     * @brief      空军士兵的初始化函数（重写父类虚函数）
     * @details    先调用父类 Soldier 的初始化函数，初始化成功后，依次完成空军士兵的属性配置
     *             与动画相关初始化，是创建空军士兵实例的唯一入口函数
     * @param      battleScene  战斗场景指针，用于关联空军士兵所在的战斗场景上下文，提供环境数据
     * @param      type         士兵类型枚举（SoldierType），指定空军士兵的具体类型（暂未差异化配置属性）
     * @return     bool         初始化成功返回 true；若父类初始化失败或自身初始化异常，返回 false
     * @override   Soldier::init
     */
    virtual bool init(BattleScene* battleScene, SoldierType type) override;

    /**
     * @brief      配置空军士兵的核心属性（重写父类虚函数）
     * @details    初始化空军士兵的精灵纹理，设置专属的最大生命值、攻击力、攻击范围、
     *             飞行移动速度等固有属性，同时初始化血条相关参数（若有）
     * @param      type         士兵类型枚举（SoldierType），当前暂未根据类型差异化配置，统一使用空军默认属性
     * @override   Soldier::setupProperties
     */
    virtual void setupProperties(SoldierType type) override;

    /**
     * @brief      播放空军士兵的飞行动画（重写父类虚函数，原父类为行走动画）
     * @details    创建并运行循环飞行动画，通过动作标签避免重复创建相同动画实例，
     *             动画帧来源于空军专属的纹理资源，与地面士兵行走动画区分
     * @override   Soldier::playWalkAnim
     * @note       函数名保留`playWalkAnim`是为了兼容父类接口，实际功能为播放飞行动画
     */
    virtual void playWalkAnim() override;

    /**
     * @brief      停止空军士兵的飞行动画（重写父类虚函数，原父类为停止行走动画）
     * @details    根据飞行动画的唯一标识标签，精准停止对应的循环飞行动画，
     *             避免无效动画残留占用系统资源
     * @override   Soldier::stopAnim
     * @note       函数名保留`stopAnim`是为了兼容父类接口，实际功能为停止飞行动画
     */
    virtual void stopAnim() override;

    /**
     * @brief      核心标识函数：判断当前士兵是否为飞行单位（重写父类虚函数）
     * @details    该函数是空军士兵的核心差异化接口，返回`true`标识自身为飞行单位，
     *             战斗场景可根据该返回值实现差异化逻辑（如：无视地面围墙碰撞、被对空防御建筑优先锁定等）
     * @return     bool         固定返回 true，表示该士兵为飞行单位
     * @override   Soldier::isFlying
     * @const      该函数为只读函数，不修改类的任何成员变量
     */
    virtual bool isFlying() const override { return true; }

    /**
     * @brief      （可选）空军士兵攻击目标的特效逻辑（暂未启用，注释保留）
     * @details    若需要为空军士兵实现自定义攻击特效（如发射导弹、投掷炸弹等），
     *             可取消该注释并实现具体逻辑；当前暂使用父类的近战攻击逻辑，无需额外修改
     * @param      target       敌方建筑指针（EnemyBuilding*），指向被攻击的目标建筑
     * @override   Soldier::attackTarget
     * @note       该函数为可选扩展接口，当前处于注释状态，不影响现有功能
     */
     // virtual void attackTarget(EnemyBuilding* target) override; 
};

#endif // AIRFORCE_SOLDIER_H_