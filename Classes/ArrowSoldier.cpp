/**
 * @file       ArrowSoldier.cpp
 * @brief      弓箭士兵（ArrowSoldier）类的实现文件
 * @details    该文件实现了 ArrowSoldier 头文件声明的所有虚函数，包括初始化、属性配置、
 *             行走动画播放/停止及核心远程射箭攻击逻辑，通过 Cocos2d-x 精灵和序列动作实现
 *             箭支弹道、旋转及伤害触发，是远程地面士兵的核心业务实现
 * @version    1.0
 * @note       该类依赖 Cocos2d-x 引擎的 Animation、Sprite 等组件；
 *             箭支资源路径为 "weapon/Arrow.png"，行走动画资源路径为 "anim/arrow1.png" ~ "anim/arrow4.png"；
 *             需确保资源文件存在于 Resources 目录下，否则箭支会直接触发伤害无视觉特效
 */
#include "ArrowSoldier.h"
#include "BattleScene.h"
#include "EnemyBuilding.h" // 引入敌方建筑头文件，确保可正常使用 EnemyBuilding 类

 /**
  * @brief      弓箭士兵的初始化函数（重写父类虚函数）
  * @details    先调用父类 Soldier 的初始化函数，初始化成功后依次完成弓箭士兵的属性配置
  *             与血条初始化，是创建弓箭士兵实例的唯一入口函数
  * @param      battleScene  战斗场景指针，关联弓箭士兵所在的战斗场景上下文
  * @param      type         士兵类型枚举（SoldierType），当前暂未差异化处理
  * @return     bool         初始化成功返回 true；父类初始化失败返回 false
  * @override   Soldier::init
  */
bool ArrowSoldier::init(BattleScene* battleScene, SoldierType type)
{
    // 先执行父类初始化流程，若父类初始化失败则直接返回
    if (!Soldier::init(battleScene, type)) return false;

    // 配置弓箭士兵的核心属性（生命值、远程攻击力等）
    this->setupProperties(type);
    // 初始化并显示弓箭士兵的血条UI
    this->setupHealthBar();

    return true;
}

/**
 * @brief      配置弓箭士兵的核心属性（重写父类虚函数）
 * @details    加载弓箭士兵的初始行走纹理（第一帧），设置专属的最大生命值、远程攻击力、
 *             超远攻击范围、移动速度等属性，同时计算血条每格对应的伤害值，确保血条正常显示
 * @param      type         士兵类型枚举（SoldierType），当前暂未根据类型差异化配置属性
 * @override   Soldier::setupProperties
 */
void ArrowSoldier::setupProperties(SoldierType type)
{
    // 拼接行走动画基础路径与第一帧序号，加载初始纹理，加载失败则直接返回
    if (!this->initWithFile(StringUtils::format("%s%d.png", WALK_ANIM_BASE.c_str(), 1))) return;

    // 配置弓箭士兵的核心属性值（远程单位专属配置）
    _maxHp = 60;                // 最大生命值
    _currentHp = _maxHp;         // 当前生命值初始化为最大生命值
    _attackDamage = 8;          // 远程射箭伤害值
    _attackRange = 250.0f;      // 远程攻击范围（远超近战单位，适配射箭逻辑）
    _attackInterval = 1.0f;     // 攻击间隔（每秒1次射箭）
    _moveSpeed = 80.0f;         // 地面行走移动速度（像素/秒）

    // 计算血条每一格对应的伤害值（血条分为5格）
    _damagePerNotch = _maxHp / 5;
    // 确保每格伤害值不小于1，避免低生命值时血条显示异常
    if (_damagePerNotch < 1) _damagePerNotch = 1;
}

/**
 * @brief      播放弓箭士兵的行走动画（重写父类虚函数）
 * @details    1.  通过动作标签101判断是否已存在行走动画，避免重复创建与运行；
 *             2.  加载4帧行走动画纹理，设置每帧播放间隔，创建永久循环动画；
 *             3.  为动画设置专属标签101，用于后续精准停止动画
 * @override   Soldier::playWalkAnim
 */
void ArrowSoldier::playWalkAnim()
{
    // 检查标签101对应的行走动画是否已存在，存在则直接返回
    if (this->getActionByTag(101)) return;

    // 创建行走动画对象
    auto animation = Animation::create();
    // 循环加载4帧行走动画纹理（arrow1.png ~ arrow4.png）
    for (int i = 1; i <= 4; ++i) {
        // 拼接每帧纹理的完整路径
        std::string name = StringUtils::format("%s%d.png", WALK_ANIM_BASE.c_str(), i);
        // 向动画对象中添加对应精灵帧
        animation->addSpriteFrameWithFile(name);
    }
    // 设置动画每帧播放间隔（0.15秒/帧，适配行走视觉效果）
    animation->setDelayPerUnit(0.15f);

    // 创建永久循环的行走动画动作，并设置标签101
    auto action = RepeatForever::create(Animate::create(animation));
    action->setTag(101);
    this->runAction(action);
}

/**
 * @brief      停止弓箭士兵的行走动画（重写父类虚函数）
 * @details    根据行走动画的专属标签101，精准停止对应的循环行走动画，
 *             避免无效动画残留占用系统资源
 * @override   Soldier::stopAnim
 */
void ArrowSoldier::stopAnim()
{
    // 停止标签101对应的行走动画
    this->stopActionByTag(101);
}

/**
 * @brief      弓箭士兵攻击目标的核心逻辑（重写父类虚函数）
 * @details    实现远程射箭攻击的完整流程：
 *             1.  目标有效性校验（非空+未被摧毁）；
 *             2.  创建箭支精灵，加载箭支纹理（加载失败则直接触发伤害）；
 *             3.  设置箭支初始位置与父节点，确保视觉层级正确；
 *             4.  计算箭支弹道参数（距离、飞行时间、旋转角度）；
 *             5.  创建序列动作实现“飞行→触发伤害→销毁箭支”的完整流程；
 *             6.  运行序列动作，完成远程攻击
 * @param      target       敌方建筑指针（EnemyBuilding*），指向被攻击的目标建筑
 * @override   Soldier::attackTarget
 */
void ArrowSoldier::attackTarget(EnemyBuilding* target)
{
    // 1. 目标有效性校验：目标为空或已被摧毁，则直接返回，不执行攻击逻辑
    if (!target || target->isDestroyed()) {
        return;
    }

    // 2. 创建箭支精灵，加载箭支纹理资源
    auto arrow = Sprite::create("weapon/Arrow.png");
    if (!arrow) {
        // 箭支纹理加载失败时，直接触发目标受伤害，无视觉特效
        target->takeDamage(this->_attackDamage);
        return;
    }

    // 3. 设置箭支初始位置与父节点（确保箭支正常显示）
    Vec2 startPos = this->getPosition(); // 获取弓箭士兵当前在场景中的位置
    arrow->setPosition(startPos);        // 设置箭支初始位置为士兵当前位置

    // 将箭支添加到父节点（优先添加到士兵的父节点，即战斗场景，确保层级正确）
    if (this->getParent()) {
        // 箭支ZOrder设为士兵层级+1，确保箭支显示在士兵上方
        this->getParent()->addChild(arrow, this->getLocalZOrder() + 1);
    }
    else {
        // 若士兵无父节点，将箭支添加到士兵自身节点，居中显示
        this->addChild(arrow);
        arrow->setPosition(this->getContentSize() / 2);
    }

    // 4. 计算箭支飞行的弹道参数
    Vec2 targetPos = target->getPosition(); // 获取目标建筑的当前位置
    float distance = startPos.distance(targetPos); // 计算士兵与目标的直线距离
    float speed = 800.0f; // 箭支飞行速度（像素/秒）
    float duration = distance / speed; // 计算箭支飞行到目标的耗时

    // 5. 设置箭支旋转角度，使其朝向目标（增强视觉真实感）
    Vec2 direction = targetPos - startPos; // 计算士兵到目标的方向向量
    float angle = CC_RADIANS_TO_DEGREES(atan2(direction.y, direction.x)); // 计算弧度转角度
    arrow->setRotation(-angle); // 修正Cocos2d-x旋转方向，使箭支指向目标
    arrow->setScale(3.0f); // 放大箭支，适配视觉显示效果

    // 6. 创建箭支序列动作，实现“飞行→伤害→销毁”的完整流程
    auto seq = Sequence::create(
        MoveTo::create(duration, targetPos), // 箭支飞行到目标位置
        CallFunc::create([target, this, arrow]() {
            // Lambda回调：箭支到达目标位置时执行

            // 再次校验目标有效性（避免目标在箭支飞行中被摧毁）
            if (target && target->getCurrentHp() > 0) {
                // 触发目标受伤害，传递弓箭士兵的攻击伤害值
                target->takeDamage(this->_attackDamage);
            }
            // 销毁箭支精灵，释放内存，避免内存泄漏
            arrow->removeFromParent();
            }),
        nullptr // 序列动作结束标记
    );

    // 运行箭支序列动作，执行远程攻击流程
    arrow->runAction(seq);
}