/**
 * @file       GiantSoldier.cpp
 * @brief      巨人士兵（GiantSoldier）类的实现文件
 * @details    该文件实现了 GiantSoldier 头文件声明的所有虚函数（含纯虚函数），包括初始化、
 *             肉盾属性配置、行走动画播放/停止等核心逻辑，适配巨人士兵高生命值、近战肉盾的定位，
 *             行走动画依赖私有常量指定的专属纹理路径
 * @version    1.0
 * @note       该类依赖 Cocos2d-x 引擎的 Animation、StringUtils 等组件；
 *             行走动画资源路径为 "anim/giant1.png" ~ "anim/giant4.png"，需确保资源文件存在于 Resources 目录下；
 *             巨人士兵暂使用父类默认近战攻击逻辑，若需自定义可扩展 attackTarget 函数
 */
#include "GiantSoldier.h"
#include "BattleScene.h"

 /**
  * @brief      巨人士兵的初始化函数（重写父类虚函数）
  * @details    先调用父类 Soldier 的初始化函数，初始化成功后依次完成：
  *             1.  配置巨人士兵的肉盾专属属性；
  *             2.  初始化并显示血条UI；
  *             是创建巨人士兵实例的唯一入口函数
  * @param      battleScene  战斗场景指针，关联巨人士兵所在的战斗场景上下文
  * @param      type         士兵类型枚举（SoldierType），当前暂未差异化处理
  * @return     bool         初始化成功返回 true；父类初始化失败返回 false
  * @override   Soldier::init
  */
bool GiantSoldier::init(BattleScene* battleScene, SoldierType type)
{
    // 1. 先执行父类初始化流程，若父类初始化失败则直接返回
    if (!Soldier::init(battleScene, type)) return false;

    // 2. 配置巨人士兵的核心属性（需在血条初始化前执行，确保血条参数有效）
    this->setupProperties(type);

    // 3. 初始化血条，使用父类实现的通用血条逻辑
    this->setupHealthBar();

    return true;
}

/**
 * @brief      配置巨人士兵的核心属性（重写父类纯虚函数）
 * @details    加载巨人士兵的初始行走纹理（第一帧），设置专属的肉盾型属性（高生命值、低攻击力），
 *             同时定义血格数量并计算每格对应的伤害值，确保血条正常显示
 * @param      type         士兵类型枚举（SoldierType），当前暂未根据类型差异化配置属性
 * @override   Soldier::setupProperties
 */
void GiantSoldier::setupProperties(SoldierType type)
{
    // 拼接行走动画基础路径与第一帧序号，加载初始纹理，加载失败则直接返回
    if (!this->initWithFile(StringUtils::format("%s%d.png", WALK_ANIM_BASE.c_str(), 1))) return;

    // 配置巨人士兵的核心属性值（肉盾近战单位专属配置：高生命值、低攻击力）
    _maxHp = 100;                // 最大生命值（远高于其他士兵，适配肉盾定位）
    _currentHp = _maxHp;         // 当前生命值初始化为最大生命值
    _attackDamage = 3;           // 近战攻击伤害值（肉盾单位侧重抗伤，攻击力较低）
    _attackRange = 80.0f;        // 近战攻击范围
    _attackInterval = 1.0f;      // 攻击间隔（每秒1次近战攻击）
    _moveSpeed = 80.0f;          // 地面行走移动速度（像素/秒）

    // 计算血条每一格对应的伤害值
    const int NOTCH_COUNT = 5;   // 血条总格数（常量定义，便于后续修改维护）
    _damagePerNotch = _maxHp / NOTCH_COUNT;
    // 确保每格伤害值不小于1，避免低生命值时血条显示异常
    if (_damagePerNotch < 1) _damagePerNotch = 1;
}

/**
 * @brief      播放巨人士兵的行走动画（重写父类纯虚函数）
 * @details    1.  通过动作标签101判断是否已存在行走动画，避免重复创建与运行；
 *             2.  加载4帧行走动画纹理，设置每帧播放间隔，创建永久循环动画；
 *             3.  为动画设置专属标签101，用于后续精准停止动画
 * @override   Soldier::playWalkAnim
 */
void GiantSoldier::playWalkAnim()
{
    // 检查标签101对应的行走动画是否已存在，存在则直接返回
    if (this->getActionByTag(101)) return;

    // 创建行走动画对象
    auto animation = Animation::create();
    // 循环加载4帧行走动画纹理（giant1.png ~ giant4.png）
    for (int i = 1; i <= 4; ++i) {
        // 拼接每帧纹理的完整路径
        std::string name = StringUtils::format("%s%d.png", WALK_ANIM_BASE.c_str(), i);
        // 向动画对象中添加对应精灵帧
        animation->addSpriteFrameWithFile(name);
    }

    // 设置动画每帧播放间隔（0.15秒/帧，适配巨人士兵庞大体型的行走视觉效果）
    animation->setDelayPerUnit(0.15f);

    // 创建永久循环的行走动画动作，并设置标签101
    auto action = RepeatForever::create(Animate::create(animation));
    action->setTag(101);
    this->runAction(action);
}

/**
 * @brief      停止巨人士兵的行走动画（重写父类纯虚函数）
 * @details    根据行走动画的专属标签101，精准停止对应的循环行走动画，
 *             避免无效动画残留占用系统资源
 * @override   Soldier::stopAnim
 */
void GiantSoldier::stopAnim()
{
    // 停止标签101对应的行走动画
    this->stopActionByTag(101);
}