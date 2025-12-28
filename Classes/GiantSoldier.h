/**
 * @file       GiantSoldier.h
 * @brief      巨人士兵（GiantSoldier）类的头文件定义
 * @details    该文件声明了巨人士兵的核心接口与成员变量，巨人士兵继承自基础士兵类（Soldier），
 *             作为近战肉盾型地面单位，重写了父类的初始化、属性配置、行走动画播放/停止等接口，
 *             具备更高的生命值和抗伤能力，通过私有常量指定专属行走动画路径
 * @version    1.0
 * @note       该类依赖 Soldier 基类和 BattleScene 相关头文件，使用前需确保依赖文件已正确引入；
 *             巨人士兵的近战攻击逻辑暂使用父类默认实现（若需自定义可扩展 attackTarget 函数）
 */
#ifndef GIANT_SOLDIER_H_
#define GIANT_SOLDIER_H_

#include "Soldier.h"  // 引入基础士兵基类头文件，提供通用士兵核心接口

 /**
  * @class      GiantSoldier
  * @brief      巨人士兵类，实现肉盾型近战士兵的核心逻辑
  * @details    继承自 Soldier 基类，重写了父类的纯虚函数与初始化函数，核心特性是高生命值、高抗伤能力，
  *             作为地面近战单位，行走动画依赖私有常量指定的纹理路径，适配其庞大体型的移动视觉效果
  * @extends    Soldier
  * @note       该类保护成员可被子类继承扩展，私有成员为行走动画基础路径，仅内部可访问；
  *             若需实现专属近战攻击逻辑，可在 cpp 文件中重写 attackTarget 函数
  */
class GiantSoldier : public Soldier
{
public:
    /**
     * @brief      巨人士兵的初始化函数（重写父类虚函数）
     * @details    先配置巨人士兵的子类特有初始值，再调用父类 Soldier 的初始化函数，
     *             完成血条初始化等通用流程，是创建巨人士兵实例的唯一入口函数
     * @param      battleScene  战斗场景指针，用于关联巨人士兵所在的战斗场景上下文
     * @param      type         士兵类型枚举（SoldierType），指定巨人士兵的具体类型（暂未差异化使用）
     * @return     bool         初始化成功返回 true；父类初始化失败或自身配置异常返回 false
     * @override   Soldier::init
     */
    virtual bool init(BattleScene* battleScene, SoldierType type) override;

protected:
    /**
     * @brief      配置巨人士兵的核心属性（重写父类纯虚函数）
     * @details    初始化巨人士兵的精灵纹理，设置专属的高生命值、近战攻击力、
     *             攻击范围、移动速度等肉盾型单位属性，同时计算血条每格对应的伤害值
     * @param      type         士兵类型枚举（SoldierType），当前暂未根据类型差异化配置属性
     * @override   Soldier::setupProperties
     */
    virtual void setupProperties(SoldierType type) override;

    /**
     * @brief      播放巨人士兵的行走动画（重写父类纯虚函数）
     * @details    根据私有常量指定的行走动画基础路径，加载并运行循环行走动画，
     *             通过动作标签避免重复创建相同动画实例，适配巨人士兵庞大体型的移动特性
     * @override   Soldier::playWalkAnim
     */
    virtual void playWalkAnim() override;

    /**
     * @brief      停止巨人士兵的行走动画（重写父类纯虚函数）
     * @details    根据行走动画的唯一标识标签，精准停止对应的循环行走动画，
     *             避免无效动画残留占用系统资源
     * @override   Soldier::stopAnim
     */
    virtual void stopAnim() override;

private:
    /**
     * @brief      巨人士兵行走动画的基础路径常量
     * @details    巨人士兵行走动画帧的文件路径前缀，动画帧文件命名格式为 "anim/giant1.png" ~ "anim/giantN.png"（N为动画帧总数），
     *             该常量用于拼接完整的动画帧文件路径，方便加载专属行走动画纹理
     */
    const std::string WALK_ANIM_BASE = "anim/giant";
};

#endif // GIANT_SOLDIER_H_