/**
 * @file       OriginalSoldier.h
 * @brief      原始士兵（基础步兵，OriginalSoldier）类的头文件定义
 * @details    该文件声明了原始士兵的核心接口与成员变量，原始士兵继承自基础士兵类（Soldier），
 *             作为最基础的地面近战单位，重写了父类的初始化、属性配置、行走动画播放/停止等接口，
 *             具备均衡的各项属性，无特殊差异化能力，是其他特种士兵的基础模板
 * @version    1.0
 * @note       该类依赖 Soldier 基类和 BattleScene 相关头文件，使用前需确保依赖文件已正确引入；
 *             原始士兵暂使用父类默认近战攻击逻辑，若需自定义攻击特效可扩展 attackTarget 函数
 */
#ifndef ORIGINAL_SOLDIER_H_
#define ORIGINAL_SOLDIER_H_

#include "Soldier.h"  // 引入基础士兵基类头文件，提供通用士兵核心接口

 /**
  * @class      OriginalSoldier
  * @brief      原始士兵类，实现基础近战步兵的核心逻辑
  * @details    继承自 Soldier 基类，重写了父类的虚函数，核心特性是属性均衡、无特殊能力，
  *             作为地面近战单位，行走动画依赖私有常量指定的纹理路径，是其他特种士兵（自爆、弓箭等）
  *             的基础参考模板
  * @extends    Soldier
  * @note       该类保护成员可被子类继承扩展，私有成员为行走动画基础路径，仅内部可访问；
  *             若需实现专属攻击逻辑，可在 cpp 文件中重写 attackTarget 函数
  */
class OriginalSoldier : public Soldier
{
public:
    /**
     * @brief      原始士兵的初始化函数（重写父类虚函数）
     * @details    先调用父类 Soldier 的初始化函数，初始化成功后依次完成原始士兵的属性配置
     *             与血条初始化，是创建原始士兵实例的唯一入口函数
     * @param      battleScene  战斗场景指针，用于关联原始士兵所在的战斗场景上下文
     * @param      type         士兵类型枚举（SoldierType），指定原始士兵的具体类型（暂未差异化使用）
     * @return     bool         初始化成功返回 true；父类初始化失败或自身配置异常返回 false
     * @override   Soldier::init
     */
    virtual bool init(BattleScene* battleScene, SoldierType type) override;

protected:
    /**
     * @brief      配置原始士兵的核心属性（重写父类虚函数）
     * @details    初始化原始士兵的精灵纹理，设置均衡的最大生命值、近战攻击力、
     *             攻击范围、移动速度等基础属性，同时计算血条每格对应的伤害值
     * @param      type         士兵类型枚举（SoldierType），当前暂未根据类型差异化配置属性
     * @override   Soldier::setupProperties
     */
    virtual void setupProperties(SoldierType type) override;

    /**
     * @brief      播放原始士兵的行走动画（重写父类虚函数）
     * @details    根据私有常量指定的行走动画基础路径，加载并运行循环行走动画，
     *             通过动作标签避免重复创建相同动画实例，适配基础步兵的地面移动特性
     * @override   Soldier::playWalkAnim
     */
    virtual void playWalkAnim() override;

    /**
     * @brief      停止原始士兵的行走动画（重写父类虚函数）
     * @details    根据行走动画的唯一标识标签，精准停止对应的循环行走动画，
     *             避免无效动画残留占用系统资源
     * @override   Soldier::stopAnim
     */
    virtual void stopAnim() override;

private:
    /**
     * @brief      原始士兵行走动画的基础路径常量
     * @details    原始士兵行走动画帧的文件路径前缀，动画帧文件命名格式为 "anim/man1.png" ~ "anim/manN.png"（N为动画帧总数），
     *             该常量用于拼接完整的动画帧文件路径，方便加载基础步兵专属行走动画纹理
     */
    const std::string WALK_ANIM_BASE = "anim/man";
};

#endif // ORIGINAL_SOLDIER_H_