#include "BoomSoldier.h"  // 引入自爆士兵类的头文件
#include "BattleScene.h"  // 引入战斗场景类的头文件

/**
 * @brief  自爆士兵（BoomSoldier）的初始化函数
 * @details 继承自父类Soldier的初始化，同时完成自爆士兵的属性配置和血条初始化
 * @param  battleScene  战斗场景的指针，用于关联士兵所在的战斗场景
 * @param  type         士兵类型枚举（SoldierType），指定当前自爆士兵的类型
 * @return bool         初始化成功返回true，初始化失败返回false
 */
bool BoomSoldier::init(BattleScene* battleScene, SoldierType type)
{
    // 先调用父类Soldier的初始化函数，若父类初始化失败则直接返回false
    if (!Soldier::init(battleScene, type)) return false;

    // 配置自爆士兵的核心属性（血量、攻击力等）
    this->setupProperties(type);
    // 初始化并设置自爆士兵的血条UI
    this->setupHealthBar();

    return true;
}

/**
 * @brief  配置自爆士兵的核心属性
 * @details 初始化士兵精灵纹理，并设置最大生命值、攻击力、移动速度等固有属性
 * @param  type  士兵类型枚举（SoldierType），当前暂未根据类型差异化配置，统一使用固定属性
 */
void BoomSoldier::setupProperties(SoldierType type)
{
    // 初始化自爆士兵的精灵纹理：拼接行走动画的基础路径和第一帧图片名
    // WALK_ANIM_BASE 为行走动画的基础路径常量，格式化为 "xxx1.png" 作为初始纹理
    if (!this->initWithFile(StringUtils::format("%s%d.png", WALK_ANIM_BASE.c_str(), 1))) {
        return;  // 纹理初始化失败则直接返回
    }

    // 配置自爆士兵的核心属性值
    _maxHp = 150;                // 最大生命值
    _currentHp = _maxHp;         // 当前生命值初始化为最大生命值
    _attackDamage = 20;          // 自爆伤害（攻击目标时的单次伤害值）
    _attackRange = 50.0f;        // 攻击范围（自爆士兵为近距离攻击，范围较小）
    _attackInterval = 1.0f;      // 攻击间隔（自爆为一次性攻击，该参数用于兼容父类逻辑）
    _moveSpeed = 80.0f;          // 移动速度（像素/秒）

    // 计算血条每一格对应的伤害值：血条分为5格，每格对应最大生命值的1/5
    _damagePerNotch = _maxHp / 5;
    // 确保每格伤害值不小于1（避免最大生命值过小时出现0值）
    if (_damagePerNotch < 1) {
        _damagePerNotch = 1;
    }
}

/**
 * @brief  播放自爆士兵的行走动画
 * @details 创建并运行循环行走动画，通过动作标签避免重复创建相同动画
 * @note   动画标签101为行走动画的唯一标识，用于后续停止该动画
 */
void BoomSoldier::playWalkAnim()
{
    // 检查是否已存在标签为101的行走动画，若存在则直接返回（避免重复执行）
    if (this->getActionByTag(101)) {
        return;
    }

    // 创建行走动画对象
    auto animation = Animation::create();
    // 循环加载行走动画的4帧图片（1~4帧）
    for (int i = 1; i <= 4; ++i) {
        // 拼接每帧的图片路径和名称
        std::string name = StringUtils::format("%s%d.png", WALK_ANIM_BASE.c_str(), i);
        // 向动画中添加对应帧的精灵帧
        animation->addSpriteFrameWithFile(name);
    }
    // 设置动画每帧的播放间隔（0.15秒/帧）
    animation->setDelayPerUnit(0.15f);

    // 创建永久重复的行走动画动作（Animate封装动画，RepeatForever实现循环）
    auto action = RepeatForever::create(Animate::create(animation));
    // 为行走动画设置唯一标签101，用于后续精准停止该动画
    action->setTag(101);
    // 运行行走动画
    this->runAction(action);
}

/**
 * @brief  停止自爆士兵的行走动画
 * @details 通过行走动画的唯一标签101，停止对应的循环动画
 */
void BoomSoldier::stopAnim()
{
    // 根据标签101停止行走动画（与playWalkAnim中的标签对应）
    this->stopActionByTag(101);
}

/**
 * @brief  自爆士兵攻击目标（敌方建筑）的核心逻辑
 * @details 对目标造成自爆伤害，播放爆炸特效，随后将自身血量置0并停止所有动作
 * @param  target  敌方建筑（EnemyBuilding）的指针，若为nullptr则不执行伤害逻辑
 */
void BoomSoldier::attackTarget(EnemyBuilding* target)
{
    // 1. 对目标造成自爆伤害
    if (target) {
        // 调用敌方建筑的受伤害函数，传递自爆士兵的攻击伤害值
        target->takeDamage(this->_attackDamage);
    }

    // ==================== 播放自爆爆炸特效 ====================
    // 确保当前士兵存在父节点（战斗场景节点），避免空指针异常
    if (this->getParent())
    {
        // 创建爆炸特效的精灵对象（初始为空，后续通过帧动画填充内容）
        auto explosion = Sprite::create();
        // 设置爆炸特效的位置为自爆士兵当前的位置
        explosion->setPosition(this->getPosition());
        // 放大爆炸特效（缩放比例3.0f，增强视觉效果）
        explosion->setScale(3.0f);

        // 将爆炸特效添加到父节点（战斗场景），并设置渲染层级（高于士兵本身，确保特效在顶层显示）
        this->getParent()->addChild(explosion, this->getLocalZOrder() + 10);

        // 收集爆炸特效的所有精灵帧（1~9帧）
        Vector<SpriteFrame*> animFrames;
        char str[100] = { 0 };  // 用于拼接爆炸特效图片路径的字符数组
        for (int i = 1; i <= 9; ++i) {
            // 拼接爆炸特效图片路径：soldiers/Explosion1.png ~ soldiers/Explosion9.png
            sprintf(str, "soldiers/Explosion%d.png", i);
            // 创建对应帧的精灵对象，用于获取精灵帧
            auto sprite = Sprite::create(str);
            // 确保精灵创建成功，避免空指针添加到帧集合
            if (sprite) {
                animFrames.pushBack(sprite->getSpriteFrame());
            }
        }

        // 创建爆炸动画（使用收集到的精灵帧，每帧播放间隔0.1秒）
        auto animation = Animation::createWithSpriteFrames(animFrames, 0.1f);
        // 创建爆炸特效的序列动作：播放动画 -> 播放完毕后自动移除特效精灵
        auto seq = Sequence::create(
            Animate::create(animation),  // 播放爆炸动画
            RemoveSelf::create(),        // 动画结束后自动从父节点移除自身，避免内存泄漏
            nullptr                      // 序列动作结束标记
        );
        // 运行爆炸特效的序列动作
        explosion->runAction(seq);
    }
    // ============================================================

    // 2. 停止自爆士兵的所有定时器回调（如攻击间隔定时器、移动定时器等）
    this->unscheduleAllCallbacks();
    // 停止自爆士兵的所有动作（如行走动画等）
    this->stopAllActions();

    // 3. 将自身当前血量置0，标记士兵已自爆消亡
    // 注：BattleScene会在下一帧检测士兵血量<=0时，自动移除该士兵对象
    this->_currentHp = 0;

    // 4. 更新血条显示（血条会根据当前血量0，显示为空血状态）
    this->updateHealthBar();
}