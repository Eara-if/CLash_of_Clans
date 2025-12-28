/**
 * @file       BoomSoldier.h
 * @brief      自爆士兵（BoomSoldier）类的头文件定义
 * @details    该文件声明了自爆士兵的核心接口与成员变量，自爆士兵继承自基础士兵类（Soldier），
 *             重写了父类的初始化、属性配置、动画播放及攻击逻辑，核心特性是攻击目标时触发自爆特效并消亡
 * @version    1.0
 * @note       该类依赖 Soldier 基类和 BattleScene、EnemyBuilding 相关头文件，使用前需确保依赖文件已引入
 */
#ifndef BOOM_SOLDIER_H_
#define BOOM_SOLDIER_H_

#include "Soldier.h"  // 引入基础士兵基类头文件

 /**
  * @class      BoomSoldier
  * @brief      自爆士兵类，实现自爆攻击的核心逻辑
  * @details    继承自 Soldier 基类，重写了基类的虚函数，主要扩展了自爆特效播放、一次性攻击后自身消亡的逻辑，
  *             包含行走动画配置、自爆攻击、属性初始化等核心功能
  * @extends    Soldier
  * @note       该类为具体业务类，不支持进一步继承（若需继承可将私有成员改为保护成员）
  */
class BoomSoldier : public Soldier
{
public:
    /**
     * @brief      自爆士兵的初始化函数（重写父类虚函数）
     * @details    调用父类初始化函数后，完成自爆士兵的属性配置和血条初始化，是创建自爆士兵实例的入口函数
     * @param      battleScene  战斗场景指针，用于关联士兵所在的战斗场景上下文
     * @param      type         士兵类型枚举（SoldierType），指定自爆士兵的具体类型（暂未差异化使用）
     * @return     bool         初始化成功返回 true，初始化失败（如父类初始化失败）返回 false
     * @override   Soldier::init
     */
    virtual bool init(BattleScene* battleScene, SoldierType type) override;

protected:
    /**
     * @brief      配置自爆士兵的核心属性（重写父类虚函数）
     * @details    初始化自爆士兵的精灵纹理，设置最大生命值、攻击力、攻击范围、移动速度等固有属性，
     *             同时计算血条每格对应的伤害值
     * @param      type         士兵类型枚举（SoldierType），当前暂未根据类型差异化配置属性，统一使用固定值
     * @override   Soldier::setupProperties
     */
    virtual void setupProperties(SoldierType type) override;

    /**
     * @brief      播放自爆士兵的行走动画（重写父类虚函数）
     * @details    创建并运行循环行走动画，通过动作标签避免重复创建相同动画，动画帧来源于预定义的基础路径
     * @override   Soldier::playWalkAnim
     */
    virtual void playWalkAnim() override;

    /**
     * @brief      停止自爆士兵的行走动画（重写父类虚函数）
     * @details    根据行走动画的唯一标签，精准停止对应的循环行走动画，避免无效动作残留
     * @override   Soldier::stopAnim
     */
    virtual void stopAnim() override;

    /**
     * @brief      自爆士兵攻击目标的核心逻辑（重写父类虚函数）
     * @details    实现自爆攻击的完整流程：对敌方建筑造成伤害 -> 播放爆炸特效 -> 停止自身所有动作和定时器 ->
     *             将自身血量置0 -> 更新血条，最终由战斗场景检测到血量为0后移除该士兵实例
     * @param      target       敌方建筑指针（EnemyBuilding*），指向被攻击的目标建筑，若为 nullptr 则不执行伤害逻辑
     * @override   Soldier::attackTarget
     */
    virtual void attackTarget(EnemyBuilding* target) override;

private:
    /**
     * @brief      行走动画的基础路径常量
     * @details    自爆士兵行走动画帧的文件路径前缀，动画帧文件命名格式为 "anim/boom1.png" ~ "anim/boom4.png"，
     *             该常量用于拼接完整的动画帧文件路径，方便加载行走动画
     */
    const std::string WALK_ANIM_BASE = "anim/boom";
};

#endif // BOOM_SOLDIER_H_