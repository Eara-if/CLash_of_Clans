/**
 * @file       OriginalSoldier.cpp
 * @brief      原始士兵（基础步兵，OriginalSoldier）类的实现文件
 * @details    该文件实现了 OriginalSoldier 头文件声明的所有虚函数，包括初始化、均衡属性配置、
 *             行走动画播放/停止等核心逻辑。原始士兵作为基础地面近战单位，属性均衡无特殊能力，
 *             是其他特种士兵的基础实现模板，其逻辑流程与其他士兵类保持统一。
 * @version    1.0
 * @note       该类依赖 Cocos2d-x 引擎的 Animation、StringUtils 组件；
 *             行走动画资源路径为 "anim/man1.png" ~ "anim/man4.png"，需确保资源存在于 Resources 目录；
 *             原始士兵复用父类默认近战攻击逻辑，无需额外自定义攻击接口。
 */
#include "OriginalSoldier.h"
#include "BattleScene.h"

 /**
  * @brief      原始士兵的初始化函数（重写父类虚函数）
  * @details    先调用父类 Soldier 的通用初始化流程，成功后依次执行自身属性配置与血条初始化，
  *             是创建原始士兵实例的唯一入口函数，流程与其他士兵类保持一致。
  * @param      battleScene  战斗场景指针，关联原始士兵所在的战斗场景上下文
  * @param      type         士兵类型枚举（SoldierType），当前暂未差异化处理
  * @return     bool         初始化成功返回 true；父类初始化失败返回 false
  * @override   Soldier::init
  */
bool OriginalSoldier::init(BattleScene* battleScene, SoldierType type)
{
    // 优先执行父类初始化，父类初始化失败则直接返回
    if (!Soldier::init(battleScene, type)) return false;

    // 配置原始士兵核心属性（需在血条初始化前执行，确保血条参数有效）
    this->setupProperties(type);

    // 初始化并显示通用血条，复用父类实现逻辑
    this->setupHealthBar();

    return true;
}

/**
 * @brief      配置原始士兵的核心属性（重写父类虚函数）
 * @details    加载原始士兵初始行走纹理（第一帧），设置均衡的近战属性值，同时计算血条每格对应的伤害值，
 *             确保血条正常显示与更新，属性配置贴合基础步兵的均衡定位。
 * @param      type         士兵类型枚举（SoldierType），当前暂未根据类型差异化配置属性
 * @override   Soldier::setupProperties
 */
void OriginalSoldier::setupProperties(SoldierType type)
{
    // 拼接行走动画基础路径与第一帧序号，加载初始纹理，加载失败则直接返回
    if (!this->initWithFile(StringUtils::format("%s%d.png", WALK_ANIM_BASE.c_str(), 1))) return;

    // 配置原始士兵核心属性（均衡值，无特殊偏向，贴合基础步兵定位）
    _maxHp = 75;                // 最大生命值（基础均衡值）
    _currentHp = _maxHp;         // 当前生命值初始化为最大生命值
    _attackDamage = 4;          // 近战攻击伤害值（基础均衡值）
    _attackRange = 80.0f;       // 近战攻击范围（与其他近战士兵保持一致）
    _attackInterval = 1.0f;     // 攻击间隔（每秒1次，通用近战配置）
    _moveSpeed = 80.0f;         // 地面行走移动速度（基础均衡值，像素/秒）

    // 计算血条每一格对应的伤害值（血条固定分为5格）
    _damagePerNotch = _maxHp / 5;
    // 保底处理：确保每格伤害值不小于1，避免低生命值时血条显示异常
    if (_damagePerNotch < 1) _damagePerNotch = 1;
}

/**
 * @brief      播放原始士兵的行走动画（重写父类虚函数）
 * @details    1.  通过动作标签101判断动画是否已存在，避免重复创建与运行，节省系统资源；
 *             2.  加载4帧行走动画纹理，设置合理播放间隔，适配基础步兵行走节奏；
 *             3.  创建永久循环动作并绑定标签101，用于后续精准停止动画。
 * @override   Soldier::playWalkAnim
 */
void OriginalSoldier::playWalkAnim()
{
    // 检查标签101对应的行走动画是否已存在，存在则直接返回
    if (this->getActionByTag(101)) return;

    // 创建行走动画对象
    auto animation = Animation::create();
    // 循环加载4帧行走动画纹理（man1.png ~ man4.png）
    for (int i = 1; i <= 4; ++i) {
        // 拼接每帧纹理的完整路径
        std::string name = StringUtils::format("%s%d.png", WALK_ANIM_BASE.c_str(), i);
        // 向动画对象中添加对应精灵帧
        animation->addSpriteFrameWithFile(name);
    }

    // 设置动画每帧播放间隔（0.15秒/帧，适配基础步兵行走视觉效果）
    animation->setDelayPerUnit(0.15f);

    // 创建永久循环行走动画，并绑定专属标签101
    auto action = RepeatForever::create(Animate::create(animation));
    action->setTag(101);
    this->runAction(action);
}

/**
 * @brief      停止原始士兵的行走动画（重写父类虚函数）
 * @details    根据行走动画的专属标签101，精准停止对应的循环行走动画，
 *             避免无效动画残留，确保士兵状态与动画展示一致。
 * @override   Soldier::stopAnim
 */
void OriginalSoldier::stopAnim()
{
    // 停止标签101对应的行走动画，与playWalkAnim函数标签一一对应
    this->stopActionByTag(101);
}