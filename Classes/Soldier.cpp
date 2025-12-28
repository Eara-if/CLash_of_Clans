/**
 * @file       Soldier.cpp
 * @brief      士兵抽象基类（Soldier）的实现文件
 * @details    该文件实现了 Soldier 头文件声明的所有通用接口，包括工厂方法创建士兵实例、
 *             通用初始化流程、AI 状态机逻辑（寻靶、移动、攻击）、受击处理、血条管理等核心功能，
 *             同时实现了飞行单位与陆军的差异化行为逻辑，为子类提供统一的基础支撑，子类可重写虚函数实现差异化特性
 * @version    1.0
 * @note       该文件依赖各士兵子类（GiantSoldier/OriginalSoldier 等）及 BattleScene、EnemyBuilding 头文件；
 *             工厂方法支持扩展新的士兵类型，只需在 switch 分支中添加对应子类创建逻辑；
 *             飞行单位与陆军的寻路逻辑已分离，新增飞行士兵无需修改核心寻路逻辑
 */
#include "Soldier.h"
#include "GiantSoldier.h"    // 巨人士兵子类
#include "OriginalSoldier.h" // 原始士兵子类
#include "ArrowSoldier.h"    // 弓箭士兵子类
#include "BoomSoldier.h"     // 自爆士兵子类
#include "AirforceSoldier.h" // 空军士兵子类
#include "BattleScene.h"
#include "EnemyBuilding.h" 

 // =========================================================
 // 1. 工厂方法：创建具体士兵实例
 // =========================================================
 /**
  * @brief      静态工厂方法，创建对应类型的士兵实例
  * @details    根据传入的士兵类型枚举，动态创建对应子类实例，完成初始化与自动内存管理，
  *             是外部创建士兵对象的唯一入口，支持扩展新士兵类型，创建失败时自动释放内存并返回 nullptr
  * @param      battleScene  战斗场景指针，关联士兵所在的战斗场景上下文
  * @param      type         士兵类型枚举（SoldierType），指定要创建的士兵类型
  * @return     Soldier*     创建成功返回士兵实例指针；创建失败或类型未知返回 nullptr
  */
Soldier* Soldier::create(BattleScene* battleScene, SoldierType type)
{
    Soldier* ret = nullptr;

    switch (type) {
        case SoldierType::ORIGINAL:
        {
            // 创建原始士兵实例
            auto original = new (std::nothrow) OriginalSoldier();
            if (original && original->init(battleScene, type)) {
                original->autorelease(); // 加入Cocos2d-x自动内存管理
                ret = original;
            }
            else {
                CC_SAFE_DELETE(original); // 创建失败，安全释放内存
            }
            break;
        }
        case SoldierType::ARROW:
        {
            // 创建弓箭士兵实例
            auto arrow = new (std::nothrow) ArrowSoldier();
            if (arrow && arrow->init(battleScene, type)) {
                arrow->autorelease();
                ret = arrow;
            }
            else {
                CC_SAFE_DELETE(arrow);
            }
            break;
        }
        case SoldierType::BOOM:
        {
            // 创建自爆士兵实例
            auto boom = new (std::nothrow) BoomSoldier();
            if (boom && boom->init(battleScene, type)) {
                boom->autorelease();
                ret = boom;
            }
            else {
                CC_SAFE_DELETE(boom);
            }
            break;
        }
        case SoldierType::GIANT:
        {
            // 创建巨人士兵实例
            auto giant = new (std::nothrow) GiantSoldier();
            if (giant && giant->init(battleScene, type)) {
                giant->autorelease();
                ret = giant;
            }
            else {
                CC_SAFE_DELETE(giant);
            }
            break;
        }
        case SoldierType::AIRFORCE:
        {
            // 创建空军士兵实例
            auto air = new (std::nothrow) AirforceSoldier();
            if (air && air->init(battleScene, type)) {
                air->autorelease();
                ret = air;
            }
            else {
                CC_SAFE_DELETE(air);
            }
            break;
        }
        default:
        {
            // 未知士兵类型，输出错误日志
            cocos2d::log("Error: Unknown SoldierType %d", (int)type);
            ret = nullptr;
            break;
        }
    }

    return ret;
}

// =========================================================
// 2. 通用初始化：士兵基础属性与状态初始化
// =========================================================
/**
 * @brief      士兵通用初始化函数
 * @details    完成士兵的基础初始化流程，包括父类 Sprite 初始化、成员变量赋值、状态初始化、
 *             缩放设置与 update 调度开启，子类重写时需优先调用该函数
 * @param      battleScene  战斗场景指针，关联士兵所在的战斗场景上下文
 * @param      type         士兵类型枚举（SoldierType），保存当前士兵类型用于后续AI判断
 * @return     bool         初始化成功返回 true；父类 Sprite 初始化失败返回 false
 */
bool Soldier::init(BattleScene* battleScene, SoldierType type)
{
    // 初始化父类 Sprite
    if (!Sprite::init()) return false;

    // 初始化战斗场景关联与计时变量
    _battleScene = battleScene;
    _attackTimer = 0.0f;
    // 初始化士兵状态与攻击目标
    _state = State::IDLE;
    _target = nullptr;
    // 保存士兵类型，用于后续AI逻辑差异化处理
    _soldierType = type;

    // 设置士兵缩放比例，适配视觉显示
    this->setScale(3.0f);
    // 开启每帧update调度，驱动AI逻辑
    this->scheduleUpdate();
    return true;
}

// =========================================================
// 3. 通用AI逻辑：状态机驱动与行为控制
// =========================================================

/**
 * @brief      士兵AI主更新函数，每帧调用
 * @details    核心状态机驱动函数，先校验当前攻击目标的有效性，目标失效时自动寻找新目标；
 *             再根据士兵与目标的距离，切换移动/攻击/闲置状态，并执行对应行为逻辑
 * @param      dt  帧间隔时间（秒），用于时间相关逻辑计算（移动距离、攻击计时）
 */
void Soldier::update(float dt)
{
    // 校验当前目标有效性：目标为空/生命值为0/已被摧毁，标记目标失效
    if (!_target || _target->getCurrentHp() <= 0 || _target->isDestroyed()) {
        _target = nullptr;
        // 目标失效，寻找新的攻击目标
        findNewTarget();
    }

    // 无有效目标时，切换为闲置状态并停止动画
    if (!_target) {
        if (_state != State::IDLE) {
            _state = State::IDLE;
            stopAnim(); // 停止移动动画
        }
        return;
    }

    // ==========================================
    // 距离判断：切换移动/攻击状态
    // ==========================================
    // 计算士兵与目标的直线距离
    float dist = this->getPosition().distance(_target->getPosition());
    // 判断士兵与目标是否碰撞（直接接触）
    bool isTouching = this->getBoundingBox().intersectsRect(_target->getBoundingBox());

    // 进入攻击范围或直接接触，切换为攻击状态
    if (dist <= _attackRange || isTouching) {
        if (_state != State::ATTACKING) {
            _state = State::ATTACKING;
            stopAnim(); // 停止移动动画
        }
        // 执行攻击逻辑
        attackLogic(dt);
    }
    // 未进入攻击范围，切换为移动状态
    else {
        if (_state != State::MOVING) {
            _state = State::MOVING;
            playWalkAnim(); // 播放移动动画
        }
        // 执行移动逻辑
        moveLogic(dt);
    }
}

/**
 * @brief      寻找新的有效攻击目标
 * @details    遍历战斗场景中的防御塔与基地，根据士兵类型设置差异化优先级（分数越低优先级越高），
 *             筛选出最优目标并赋值给 _target，实现士兵的差异化寻靶逻辑（空军无视围墙、巨人优先攻击防御塔等）
 */
void Soldier::findNewTarget()
{
    // 战斗场景为空，直接返回
    if (!_battleScene) return;

    // 获取场景中所有防御塔
    auto& towers = _battleScene->getTowers();
    EnemyBuilding* bestTarget = nullptr;
    // 初始化最小优先级分数为浮点数最大值
    float minScore = FLT_MAX;
    // 获取士兵当前位置
    Vec2 myPos = this->getPosition();

    // 遍历所有防御塔，计算优先级分数
    for (auto tower : towers) {
        // 防御塔无效（为空/已被摧毁），跳过
        if (tower && !tower->isDestroyed()) {
            // 基础分数：士兵与防御塔的距离（距离越近分数越低，优先级越高）
            float dist = myPos.distance(tower->getPosition());
            float score = dist;
            // 获取防御塔类型
            EnemyType enemyType = tower->getType();

            // ============================================================
            // 优先级核心逻辑：按士兵类型差异化调整分数
            // ============================================================
            if (_soldierType == SoldierType::AIRFORCE) {
                // 空军逻辑：无视围墙，优先攻击加农炮，其次防御塔
                if (enemyType == EnemyType::WALL) {
                    // 围墙添加巨额罚分，优先级最低（几乎不会选择）
                    score += 1000000.0f;
                }
                else if (enemyType == EnemyType::CANNON) {
                    // 加农炮减巨额分数，优先级最高
                    score -= 5000.0f;
                }
                else if (enemyType == EnemyType::TOWER) {
                    // 防御塔减适量分数，优先级次之
                    score -= 2000.0f;
                }
            }
            else if (_soldierType == SoldierType::GIANT) {
                // 巨人逻辑：讨厌围墙，优先攻击防御塔与加农炮
                if (enemyType == EnemyType::WALL) {
                    // 围墙添加高额罚分，优先级降低
                    score += 10000.0f;
                }
                else if (enemyType == EnemyType::CANNON || enemyType == EnemyType::TOWER) {
                    // 防御塔与加农炮减巨额分数，优先级最高
                    score -= 5000.0f;
                }
            }
            else {
                // 其他陆军（原始/弓箭/自爆）：讨厌围墙，优先攻击最近目标
                if (enemyType == EnemyType::WALL) {
                    // 围墙添加高额罚分，除非挡路否则不选择
                    score += 10000.0f;
                }
            }

            // 更新最优目标（分数越低优先级越高）
            if (score < minScore) {
                minScore = score;
                bestTarget = tower;
            }
        }
    }

    // 单独校验基地优先级，对比当前最优目标
    auto base = _battleScene->getBase();
    if (base && !base->isDestroyed()) {
        float dist = myPos.distance(base->getPosition());
        float score = dist; // 基地基础分数为距离

        // 基地优先级无需额外调整，自然低于防御塔（空军/巨人逻辑）
        if (score < minScore) {
            bestTarget = base;
        }
    }

    // 赋值最优目标
    _target = bestTarget;
}

/**
 * @brief      寻找距离当前士兵最近的围墙建筑
 * @details    遍历战斗场景中的所有防御塔，筛选出围墙类型且未被摧毁的建筑，
 *             计算并返回距离士兵最近的围墙指针，用于陆军寻路被阻挡时切换攻击目标
 * @return     EnemyBuilding*  最近的围墙建筑指针；无有效围墙时返回 nullptr
 */
EnemyBuilding* Soldier::findNearestWall()
{
    if (!_battleScene) return nullptr;
    EnemyBuilding* nearest = nullptr;
    // 初始化最小距离，仅寻找近距离围墙（避免无效远距离匹配）
    float minDst = 100.0f;

    Vec2 myPos = this->getPosition();
    // 遍历所有防御塔，筛选围墙类型
    for (auto b : _battleScene->getTowers()) {
        if (b && !b->isDestroyed() && b->getType() == EnemyType::WALL) {
            float dst = myPos.distance(b->getPosition());
            // 更新最近围墙
            if (dst < minDst) {
                minDst = dst;
                nearest = b;
            }
        }
    }
    return nearest;
}

/**
 * @brief      士兵移动逻辑，仅在移动状态下执行
 * @details    实现飞行单位与陆军的差异化移动逻辑：飞行单位无视地形阻挡，直接向目标移动；
 *             陆军需校验移动位置是否被阻挡，被阻挡时自动切换攻击最近围墙，同时实现士兵朝向调整
 * @param      dt  帧间隔时间（秒），用于计算本次帧的移动距离
 */
void Soldier::moveLogic(float dt)
{
    // 无有效目标，直接返回
    if (!_target) return;

    // 获取士兵当前位置与目标位置
    Vec2 myPos = this->getPosition();
    Vec2 targetPos = _target->getPosition();
    // 计算归一化移动方向（确保移动速度一致）
    Vec2 direction = (targetPos - myPos).getNormalized();
    // 计算本次帧的目标移动位置
    Vec2 nextPos = myPos + direction * _moveSpeed * dt;

    // 【核心差异化】飞行单位：无视阻挡，直接移动
    if (this->isFlying()) {
        // 朝向调整：根据X轴方向翻转精灵
        if (direction.x > 0) this->setFlippedX(false);
        else if (direction.x < 0) this->setFlippedX(true);

        // 直接设置移动后的位置
        this->setPosition(nextPos);
        return;
    }

    // --- 陆军移动逻辑：包含阻挡判断与被迫打墙逻辑 ---
    // 将本地坐标转换为世界坐标，用于地形阻挡判断
    Vec2 nextWorldPos = this->getParent()->convertToWorldSpace(nextPos);

    // 校验移动位置是否被阻挡
    if (_battleScene->isPositionBlocked(nextWorldPos))
    {
        // 士兵与目标已碰撞，无需继续移动
        Rect myRect = this->getBoundingBox();
        Rect targetRect = _target->getBoundingBox();
        if (myRect.intersectsRect(targetRect)) { return; }

        // 判断是否为弓箭士兵（远程单位），已进入攻击范围则无需移动
        bool isArcher = (_attackRange > 150.0f);
        float distToTarget = myPos.distance(targetPos);
        if (isArcher && distToTarget <= _attackRange) { return; }

        // 被阻挡时，切换攻击最近的围墙
        EnemyBuilding* wall = findNearestWall();
        if (wall && _target != wall) {
            _target = wall;
        }
        return;
    }

    // 移动位置有效，更新士兵位置与朝向
    this->setPosition(nextPos);
    if (direction.x > 0) this->setFlippedX(false);
    else if (direction.x < 0) this->setFlippedX(true);
}

/**
 * @brief      士兵攻击逻辑，仅在攻击状态下执行
 * @details    每帧更新攻击计时器，当计时器达到攻击间隔时，执行攻击动画、触发通用攻击逻辑，
 *             并重置计时器；同时包含士兵与目标的有效性容错判断，避免空指针异常
 * @param      dt  帧间隔时间（秒），用于更新攻击计时器
 */
void Soldier::attackLogic(float dt) {
    // 更新攻击计时器
    _attackTimer += dt;
    // 达到攻击间隔，执行攻击操作
    if (_attackTimer >= _attackInterval) {
        _attackTimer = 0.0f; // 重置攻击计时器
        if (_target) {
            // 播放攻击缩放动画（视觉反馈）
            auto seq = Sequence::create(ScaleTo::create(0.1f, 2.8f), ScaleTo::create(0.1f, 3.0f), nullptr);
            this->runAction(seq);
            // 执行通用攻击逻辑
            this->attackTarget(_target);
            // 容错判断：士兵已销毁或生命值为0，直接返回
            if (this->getReferenceCount() == 0 || this->getCurrentHp() <= 0)
                return;
            // 目标被摧毁，标记目标失效
            if (_target && _target->getCurrentHp() <= 0) _target = nullptr;
        }
    }
}

/**
 * @brief      士兵受击逻辑，处理伤害扣除与血条更新
 * @details    扣除士兵对应生命值，做保底处理（确保生命值不小于0），并调用血条更新函数，
 *             直观展示士兵当前存活状态
 * @param      damage  本次受到的伤害值
 */
void Soldier::takeDamage(int damage)
{
    // 扣除生命值
    _currentHp -= damage;
    // 保底处理：确保生命值不小于0
    if (_currentHp < 0) _currentHp = 0;

    // 更新血条显示
    updateHealthBar();
}

// =========================================================
// 4. 通用血条逻辑：初始化与动态更新
// =========================================================

/**
 * @brief      士兵血条初始化函数
 * @details    加载血条精灵纹理，根据血条总格数计算单格宽度，初始化血条显示状态（满血量），
 *             设置血条位置与缩放，并添加到士兵节点上，子类可重写该函数扩展血条样式
 */
void Soldier::setupHealthBar()
{
    // 加载血条精灵纹理（路径：ui/Heart.png）
    _healthBar = Sprite::create("ui/Heart.png");

    // 注：_damagePerNotch 需在子类 setupProperties 中初始化后生效

    if (_healthBar)
    {
        // 1. 获取血条纹理原始尺寸
        float textureWidth = _healthBar->getContentSize().width;
        float textureHeight = _healthBar->getContentSize().height;

        // 2. 计算血条单格宽度（固定5格）
        float frameWidth = textureWidth / 5.0f;

        // 3. 初始化血条显示为满血量（第4帧，对应5格血）
        _healthBar->setTextureRect(Rect(4.0f * frameWidth, 0, frameWidth, textureHeight));

        // 4. 设置血条位置（士兵节点居中，顶部显示）
        _healthBar->setPosition(Vec2(this->getContentSize().width / 2, this->getContentSize().height));

        // 5. 设置血条缩放比例，适配视觉显示
        _healthBar->setScale(1.0f);

        // 将血条添加到士兵节点上
        this->addChild(_healthBar);
    }
}

/**
 * @brief      士兵血条更新函数，根据当前生命值动态调整显示
 * @details    计算士兵剩余血格数，转换为血条纹理帧索引，做边界容错处理后，
 *             更新血条纹理矩形，实现血条的动态变化展示，子类可重写该函数扩展血条更新逻辑
 */
void Soldier::updateHealthBar()
{
    // 血条精灵无效，直接返回
    if (!_healthBar) return;

    const int NOTCH_COUNT = 5; // 血条固定总格数

    // 1. 计算损失生命值与损失血格数
    int lostHp = _maxHp - _currentHp;
    int lostNotches = lostHp / _damagePerNotch;
    // 计算当前剩余血格数
    int actualNotches = NOTCH_COUNT - lostNotches;

    // 2. 转换为血条纹理帧索引（0=空血，4=满血）
    int frameIndex = actualNotches - 1;
    // 边界容错：确保帧索引在[0,4]范围内
    if (frameIndex < 0) frameIndex = 0;
    if (frameIndex >= NOTCH_COUNT) frameIndex = NOTCH_COUNT - 1;

    // 3. 计算血条单格宽度，更新纹理矩形
    float textureWidth = _healthBar->getTexture()->getContentSize().width;
    float textureHeight = _healthBar->getTexture()->getContentSize().height;
    float frameWidth = textureWidth / NOTCH_COUNT;

    // 更新血条纹理显示
    _healthBar->setTextureRect(Rect(frameIndex * frameWidth, 0, frameWidth, textureHeight));
}

/**
 * @brief      士兵通用攻击逻辑（虚函数）
 * @details    提供默认攻击实现：对目标造成对应伤害，子类可重写该函数实现差异化攻击特效（如射箭、自爆），
 *             是士兵攻击逻辑的扩展入口
 * @param      target  被攻击的敌方建筑指针，为空时不执行任何操作
 */
void Soldier::attackTarget(EnemyBuilding* target)
{
    // 目标有效时，触发目标受击逻辑
    if (target) {
        target->takeDamage(this->_attackDamage);
    }
}