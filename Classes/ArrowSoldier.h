/**
 * @file       ArrowSoldier.h
 * @brief      弓箭士兵（ArrowSoldier）类的头文件定义
 * @details    该文件声明了弓箭士兵的核心接口与成员变量，弓箭士兵继承自基础士兵类（Soldier），
 *             核心特性是实现远程射箭攻击逻辑，同时重写了父类的初始化、属性配置、动画播放/停止
 *             等接口，具备地面行走能力和专属远程攻击特效（需在 cpp 文件中实现具体射箭逻辑）
 * @version    1.0
 * @note       该类依赖 Soldier 基类、BattleScene 及 EnemyBuilding 相关头文件，使用前需确保依赖文件已引入；
 *             远程攻击的箭支特效、弹道逻辑需在对应 cpp 文件中实现，本头文件仅声明接口
 */
#ifndef ARROW_SOLDIER_H_
#define ARROW_SOLDIER_H_

#include "Soldier.h"  // 引入基础士兵基类头文件，提供通用士兵核心接口

 /**
  * @class      ArrowSoldier
  * @brief      弓箭士兵类，实现远程射箭攻击的核心逻辑
  * @details    继承自 Soldier 基类，重写了父类的初始化、属性配置、行走动画播放/停止及攻击目标等虚函数，
  *             作为远程地面单位，具备比近战单位更远的攻击范围，核心差异化在于`attackTarget`函数中
  *             的远程射箭逻辑，行走动画依赖私有常量指定的纹理路径
  * @extends    Soldier
  * @note       该类保护成员可被子类继承扩展，私有成员为行走动画基础路径，不可外部访问
  */
class ArrowSoldier : public Soldier
{
public:
    /**
     * @brief      弓箭士兵的初始化函数（重写父类虚函数）
     * @details    调用父类 Soldier 的初始化函数后，依次完成弓箭士兵的属性配置、血条初始化
     *             等流程，是创建弓箭士兵实例的唯一入口函数
     * @param      battleScene  战斗场景指针，用于关联弓箭士兵所在的战斗场景上下文
     * @param      type         士兵类型枚举（SoldierType），指定弓箭士兵的具体类型（暂未差异化使用）
     * @return     bool         初始化成功返回 true；父类初始化失败或自身初始化异常返回 false
     * @override   Soldier::init
     */
    virtual bool init(BattleScene* battleScene, SoldierType type) override;

protected:
    /**
     * @brief      配置弓箭士兵的核心属性（重写父类虚函数）
     * @details    初始化弓箭士兵的精灵纹理，设置专属的最大生命值、远程攻击力、攻击范围、
     *             移动速度等固有属性，同时计算血条每格对应的伤害值，确保血条正常显示
     * @param      type         士兵类型枚举（SoldierType），当前暂未根据类型差异化配置属性
     * @override   Soldier::setupProperties
     */
    virtual void setupProperties(SoldierType type) override;

    /**
     * @brief      播放弓箭士兵的行走动画（重写父类虚函数）
     * @details    根据私有常量指定的行走动画基础路径，加载并运行循环行走动画，
     *             通过动作标签避免重复创建相同动画实例，适配弓箭士兵的地面移动特性
     * @override   Soldier::playWalkAnim
     */
    virtual void playWalkAnim() override;

    /**
     * @brief      停止弓箭士兵的行走动画（重写父类虚函数）
     * @details    根据行走动画的唯一标识标签，精准停止对应的循环行走动画，
     *             避免无效动画残留占用系统资源
     * @override   Soldier::stopAnim
     */
    virtual void stopAnim() override;

    /**
     * @brief      弓箭士兵攻击目标的核心逻辑（重写父类虚函数）
     * @details    实现远程射箭攻击的完整流程：创建箭支特效、计算弹道轨迹、
     *             对敌方建筑造成远程伤害，是弓箭士兵与近战士兵的核心差异化接口
     * @param      target       敌方建筑指针（EnemyBuilding*），指向被攻击的目标建筑，若为 nullptr 则不执行攻击逻辑
     * @override   Soldier::attackTarget
     */
    virtual void attackTarget(EnemyBuilding* target) override;

private:
    /**
     * @brief      行走动画的基础路径常量
     * @details    弓箭士兵行走动画帧的文件路径前缀，动画帧文件命名格式为 "anim/arrow1.png" ~ "anim/arrow4.png"（可根据实际帧数量调整），
     *             该常量用于拼接完整的动画帧文件路径，方便加载行走动画纹理
     */
    const std::string WALK_ANIM_BASE = "anim/arrow";
};

#endif // ARROW_SOLDIER_H_