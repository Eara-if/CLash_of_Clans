/**
 * @file       AirforceSoldier.cpp
 * @brief      空军士兵（AirforceSoldier）类的实现文件
 * @details    该文件实现了 AirforceSoldier 头文件声明的所有虚函数，包括初始化、属性配置、
 *             飞行动画播放/停止等核心逻辑，通过绘制阴影和上下浮动动作模拟猫头鹰的飞行效果，
 *             同时重写父类接口实现空军士兵的专属特性
 * @version    1.0
 * @note       该类依赖 Cocos2d-x 引擎的 DrawNode、Animation 等组件；
 *             动画资源路径为 "anim/Owl1.png" ~ "anim/Owl4.png"，需确保资源文件存在于 Resources 目录下
 */
#include "AirforceSoldier.h"
#include "BattleScene.h"

 /**
  * @brief      空军士兵的初始化函数（重写父类虚函数）
  * @details    先调用父类 Soldier 的初始化函数，初始化成功后依次完成：属性配置、血条初始化、
  *             飞行阴影绘制，阴影通过 DrawNode 绘制半透明扁椭圆，模拟空中单位的投影效果
  * @param      battleScene  战斗场景指针，关联空军士兵所在的战斗场景上下文
  * @param      type         士兵类型枚举（SoldierType），当前暂未差异化处理
  * @return     bool         初始化成功返回 true；父类初始化失败返回 false
  * @override   Soldier::init
  */
bool AirforceSoldier::init(BattleScene* battleScene, SoldierType type)
{
    // 先执行父类初始化，父类初始化失败则直接返回
    if (!Soldier::init(battleScene, type)) return false;

    // 配置空军士兵的核心属性（生命值、攻击力等）
    this->setupProperties(type);
    // 初始化并显示空军士兵的血条UI
    this->setupHealthBar();

    // ========== 绘制飞行单位专属阴影（增强视觉真实感） ==========
    // 创建绘制节点，用于绘制半透明黑色椭圆阴影
    auto shadow = DrawNode::create();
    // 绘制实心圆并通过缩放实现扁椭圆效果：圆心(0,0)、半径15、0度起始角、20个分段、x/y缩放比(0.5/1.0)、半透黑色
    shadow->drawSolidCircle(Vec2::ZERO, 15.0f, 0.0f, 20, 0.5f, 1.0f, Color4F(0, 0, 0, 0.4f));
    shadow->setScaleY(0.5f); // 垂直方向压扁，将圆形转为椭圆，模拟地面投影

    // 设置阴影位置：在士兵精灵底部下方30像素处（模拟飞行高度），水平居中
    shadow->setPosition(Vec2(this->getContentSize().width / 2, -30.0f));
    // 将阴影添加到士兵节点，ZOrder设为-1，确保阴影显示在士兵身体下方
    this->addChild(shadow, -1);

    return true;
}

/**
 * @brief      配置空军士兵的核心属性（重写父类虚函数）
 * @details    加载空军士兵（猫头鹰）的初始纹理，设置专属的生命值、攻击力、攻击范围、
 *             移动速度等属性，同时计算血条每格对应的伤害值，确保血条显示正常
 * @param      type         士兵类型枚举（SoldierType），当前暂未根据类型差异化配置属性
 * @override   Soldier::setupProperties
 */
void AirforceSoldier::setupProperties(SoldierType type)
{
    // 加载空军士兵初始纹理（猫头鹰第一帧），加载失败则直接返回
    // 资源路径：Resources/anim/Owl1.png
    if (!this->initWithFile("anim/Owl1.png")) return;

    // 配置空军士兵的核心属性值
    _maxHp = 50;                // 最大生命值
    _currentHp = _maxHp;         // 当前生命值初始化为最大生命值
    _attackDamage = 6;          // 攻击伤害值
    _attackRange = 80.0f;       // 攻击范围（近战范围，适配猫头鹰攻击逻辑）
    _attackInterval = 1.0f;     // 攻击间隔（每秒1次攻击）
    _moveSpeed = 80.0f;         // 飞行移动速度（像素/秒）

    // 计算血条每一格对应的伤害值（血条分为5格）
    _damagePerNotch = _maxHp / 5;
    // 确保每格伤害值不小于1，避免低生命值时出现无效血条
    if (_damagePerNotch < 1) _damagePerNotch = 1;
}

/**
 * @brief      播放空军士兵的飞行动画（重写父类虚函数，原父类为行走动画）
 * @details    1.  通过动作标签101判断是否已存在飞行动画，避免重复创建；
 *             2.  加载猫头鹰4帧飞行纹理，创建循环飞行动画；
 *             3.  添加上下浮动动作（标签102），增强飞行真实感；
 *             4.  两个动作均设置永久循环，标签用于后续精准停止
 * @override   Soldier::playWalkAnim
 * @note       函数名保留`playWalkAnim`是为了兼容父类接口，实际功能为播放飞行动画
 */
void AirforceSoldier::playWalkAnim()
{
    // 检查标签101对应的飞行动画是否已存在，存在则直接返回
    if (this->getActionByTag(101)) return;

    // 创建飞行动画对象
    auto animation = Animation::create();
    // 循环加载4帧猫头鹰飞行纹理（Owl1.png ~ Owl4.png）
    for (int i = 1; i <= 4; ++i) {
        // 拼接每帧纹理的完整路径
        std::string name = StringUtils::format("anim/Owl%d.png", i);
        // 向动画对象中添加对应精灵帧
        animation->addSpriteFrameWithFile(name);
    }
    // 设置动画每帧播放间隔（0.1秒/帧，模拟快速扇动翅膀）
    animation->setDelayPerUnit(0.1f);

    // 创建永久循环的飞行动画动作，并设置标签101
    auto action = RepeatForever::create(Animate::create(animation));
    action->setTag(101);
    this->runAction(action);

    // ========== 添加上下浮动动作，模拟飞行时的轻微晃动 ==========
    auto floatAction = RepeatForever::create(Sequence::create(
        MoveBy::create(0.5f, Vec2(0, 5)),  // 0.5秒向上移动5像素
        MoveBy::create(0.5f, Vec2(0, -5)), // 0.5秒向下移动5像素
        nullptr                            // 序列动作结束标记
    ));
    floatAction->setTag(102); // 设置浮动动作标签102，用于后续停止
    this->runAction(floatAction);
}

/**
 * @brief      停止空军士兵的所有动画（重写父类虚函数）
 * @details    根据动作标签精准停止两个循环动作：
 *             1.  标签101：猫头鹰飞行动画（翅膀扇动）
 *             2.  标签102：上下浮动动作（飞行晃动）
 *             避免无效动画残留，节省系统资源
 * @override   Soldier::stopAnim
 */
void AirforceSoldier::stopAnim()
{
    // 停止标签101对应的飞行动画
    this->stopActionByTag(101);
    // 停止标签102对应的上下浮动动作
    this->stopActionByTag(102);
}