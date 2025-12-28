//建筑生产、升级、移动逻辑
#include "Building.h"
#include "BuildingInfoLayer.h" 
#include "BuildingUpgradeLimits.h"
#include "GameScene.h"

USING_NS_CC;

// 声明全局变量（这些变量在其他文件中定义）
extern int coin_count;      // 当前金币数量
extern int water_count;     // 当前圣水数量
extern int gem_count;       // 当前宝石数量

extern int coin_limit;      // 金币上限
extern int water_limit;     // 圣水上限
extern int gem_limit;       // 宝石上限

// 创建建筑对象的静态方法
Building* Building::create(const std::string& filename, const Rect& rect, const std::string& name, int baseCost, BuildingType type)
{
    // 使用不抛出异常的方式分配内存
    Building* ret = new (std::nothrow) Building();

    // 检查内存分配和初始化是否成功
    if (ret && ret->init(filename, rect, name, baseCost, type)) // 传入建筑类型
    {
        // 成功则添加到自动释放池
        ret->autorelease();
        return ret;
    }

    // 失败则安全删除对象并返回空指针
    CC_SAFE_DELETE(ret);
    return nullptr;
}

// 初始化建筑对象
bool Building::init(const std::string& filename, const Rect& rect, const std::string& name, int baseCost, BuildingType building_type)
{
    // 检查是否需要裁剪图像
    if (rect.equals(Rect::ZERO))
    {
        // 加载整张图片
        if (!Sprite::initWithFile(filename))
        {
            return false;
        }
    }
    else
    {
        // 按照指定矩形区域裁剪图片
        if (!Sprite::initWithFile(filename, rect))
        {
            return false;
        }
        // 注意：某些Cocos版本在使用rect初始化后不需要再设置纹理矩形
        // 但为了保险起见，保持现状或根据实际显示调整
    }

    // 初始化建筑属性
    type_ = building_type;               // 保存建筑类型
    level_ = 1;                          // 初始等级为1
    building_name_ = name;               // 设置建筑名称
    base_cost_ = baseCost;               // 设置基础成本
    state_ = BuildingState::IDLE;        // 初始状态为空闲
    time_left_ = 0;                      // 升级剩余时间初始化为0

    // 初始化生产相关变量
    production_time_left_ = 0;           // 生产剩余时间
    production_amount_ = 50;             // 默认生产50资源
    is_ready_to_collect_ = false;        // 未准备好收集
    ready_indicator_ = nullptr;          // 可收集提示图标初始为空

    // 修改：金矿和圣水收集器初始状态为IDLE，不自动开始生产
    // 需要玩家手动点击生产按钮
    if (type_ == BuildingType::MINE || type_ == BuildingType::WATER)
    {
        state_ = BuildingState::IDLE;
        // 不调用startProduction()，等待玩家手动开始
    }

    // 初始化触摸监听器
    this->initTouchListener();

    // 注册每帧更新
    this->scheduleUpdate();

    return true;
}

// 节点进入场景时的回调函数
void Building::onEnter()
{
    // 1. 必须调用父类onEnter
    Sprite::onEnter();

    // 2. 进场时创建地基效果
    // 因为是加在自己身上，所以这里绝对安全，不会导致崩溃
    this->createGroundEffect();
}

// 设置升级回调函数
void Building::setOnUpgradeCallback(std::function<void()> callback)
{
    upgrade_callback_ = callback;
}

// 获取下一级的升级成本
int Building::getNextLevelCost() const
{
    // 花费随等级线性增长
    return base_cost_ * level_;
}

// 每帧自动调用的更新函数
void Building::update(float dt)
{
    // 升级倒计时处理
    if (state_ == BuildingState::UPGRADING)
    {
        time_left_ -= dt;

        if (time_left_ <= 0)
        {
            this->finishUpgrade();
        }
    }

    // 生产倒计时处理（仅对金矿和圣水收集器）
    if (type_ == BuildingType::MINE || type_ == BuildingType::WATER)
    {
        if (state_ == BuildingState::PRODUCING)
        {
            production_time_left_ -= dt;

            // 生产完成，自动转为READY状态
            if (production_time_left_ <= 0)
            {
                production_time_left_ = 0;
                this->finishProduction();
            }
        }
    }
}

// 计算加速需要的宝石数量
int Building::getSpeedUpCost() const
{
    // 简单粗暴：剩余多少秒就需要多少宝石
    return std::ceil(time_left_ / 60);
}

// 获取升级剩余时间
float Building::getTimeLeft() const
{
    // 返回剩余时间，如果小于0则返回0，防止显示负数
    return (time_left_ > 0) ? time_left_ : 0;
}

// 开始生产资源
void Building::startProduction()
{
    // 检查状态：只有在IDLE状态下才能开始生产
    if (state_ != BuildingState::IDLE)
    {
        log("%s cannot start production: current state is %d",
            building_name_.c_str(), static_cast<int>(state_));
        return;
    }

    // 检查是否正在升级
    if (state_ == BuildingState::UPGRADING)
    {
        log("%s is upgrading, cannot start production!", building_name_.c_str());
        return;
    }

    // 设置为生产状态
    state_ = BuildingState::PRODUCING;
    production_time_left_ = 5.0f; // 5秒生产周期
    is_ready_to_collect_ = false;

    // 根据等级计算生产量
    production_amount_ = 50 * level_;

    // 移除可收集提示（如果有）
    if (ready_indicator_)
    {
        ready_indicator_->removeFromParent();
        ready_indicator_ = nullptr;
    }

    log("%s started production, time left: %f, amount: %d",
        building_name_.c_str(), production_time_left_, production_amount_);
}

// 生产完成
void Building::finishProduction()
{
    if (state_ != BuildingState::PRODUCING)
    {
        log("%s cannot finish production: not in PRODUCING state", building_name_.c_str());
        return;
    }

    state_ = BuildingState::READY;
    production_time_left_ = 0;
    is_ready_to_collect_ = true;

    // 添加可收集提示
    if (!ready_indicator_)
    {
        ready_indicator_ = Sprite::create("ui/ready_indicator.png");
        if (!ready_indicator_)
        {
            // 如果图片不存在，创建一个简单的红色感叹号
            ready_indicator_ = Sprite::create();
            auto label = Label::createWithTTF("!", "fonts/Marker Felt.ttf", 72);
            label->setColor(Color3B::RED);
            label->setPosition(Vec2(15, 50));
            ready_indicator_->addChild(label);
            ready_indicator_->setContentSize(Size(30, 30));
        }
        ready_indicator_->setPosition(Vec2(this->getContentSize().width / 2,
            this->getContentSize().height + 20));
        this->addChild(ready_indicator_, 100);
    }

    log("%s production ready! Collect %d resources.",
        building_name_.c_str(), production_amount_);
}

// 收集资源
void Building::collectResources()
{
    // ================== 【拦截开始】 ==================
    // 检查是否为访客模式，访客不能收集他人资源
    auto game_scene = dynamic_cast<GameScene*>(Director::getInstance()->getRunningScene());
    extern std::string g_currentUsername;
    if (game_scene && game_scene->current_scene_owner_ != g_currentUsername)
    {
        game_scene->showToast("Cannot collect others' resources!", Color3B::RED);
        return;
    }
    // ================== 【拦截结束】 ==================

    if (state_ == BuildingState::READY && is_ready_to_collect_)
    {
        if (type_ == BuildingType::MINE)
        {
            coin_count += production_amount_;
            if (coin_count > coin_limit)
            {
                coin_count = coin_limit;
            }
            log("Collected %d gold from Gold Mine. Total: %d",
                production_amount_, coin_count);
        }
        else if (type_ == BuildingType::WATER)
        {
            water_count += production_amount_;
            if (water_count > water_limit)
            {
                water_count = water_limit;
            }
            log("Collected %d water from Water Collector. Total: %d",
                production_amount_, water_count);
        }

        // 更新UI显示
        auto scene = dynamic_cast<GameScene*>(Director::getInstance()->getRunningScene());
        if (scene)
        {
            scene->updateResourceDisplay();
        }

        // 收集后回到IDLE状态，让玩家选择下一次行动
        state_ = BuildingState::IDLE;
        is_ready_to_collect_ = false;

        // 移除可收集提示
        if (ready_indicator_)
        {
            ready_indicator_->removeFromParent();
            ready_indicator_ = nullptr;
        }

        log("%s resources collected, now in IDLE state.", building_name_.c_str());
    }
}

// 获取生产剩余时间
float Building::getProductionTimeLeft() const
{
    return production_time_left_;
}

// 获取可收集的资源量
int Building::getProducedAmount() const
{
    return production_amount_;
}

// 开始升级
void Building::startUpgrade()
{
    // 1. 基本检查：如果已经在升级，或者满了，就不能再升
    if (state_ != BuildingState::IDLE)
    {
        return;
    }

    // 【新增】检查大本营最高等级限制（10级）
    if (type_ == BuildingType::BASE && level_ >= 10)
    {
        log("Town Hall has reached maximum level (10)!");

        // 显示提示消息
        auto scene = Director::getInstance()->getRunningScene();
        auto visibleSize = Director::getInstance()->getVisibleSize();
        auto label = Label::createWithTTF("Town Hall is at max level!", "fonts/Marker Felt.ttf", 28);
        label->setPosition(visibleSize.width / 2, visibleSize.height / 2);
        label->setColor(Color3B::ORANGE);
        scene->addChild(label, 1000);
        label->runAction(Sequence::create(
            DelayTime::create(2.0f),
            RemoveSelf::create(),
            nullptr
        ));
        return;
    }

    // 【新增】检查建筑最高等级限制（大本营最高10级已在其他地方处理）
    int next_level = level_ + 1;

    // 获取当前大本营等级
    int town_hall_level = 1;
    for (auto& building : g_allPurchasedBuildings)
    {
        if (building && building->getType() == BuildingType::BASE)
        {
            town_hall_level = building->getLevel();
            break;
        }
    }

    // 检查是否可以升级到下一级
    auto upgrade_limits = BuildingUpgradeLimits::getInstance();
    int max_level_for_this_th = upgrade_limits->getMaxLevelForBuilding(type_, town_hall_level);

    if (next_level > max_level_for_this_th && type_ != BuildingType::BASE)
    {
        log("Cannot upgrade %s to level %d: Maximum level for TH%d is %d",
            building_name_.c_str(), next_level, town_hall_level, max_level_for_this_th);

        // 显示提示消息
        auto scene = Director::getInstance()->getRunningScene();
        auto visibleSize = Director::getInstance()->getVisibleSize();

        std::string message = building_name_ + " cannot be upgraded beyond level " +
            std::to_string(max_level_for_this_th) +
            " at Town Hall level " + std::to_string(town_hall_level);

        auto label = Label::createWithTTF(message, "fonts/Marker Felt.ttf", 28);
        label->setPosition(visibleSize.width / 2, visibleSize.height / 2);
        label->setColor(Color3B::ORANGE);
        scene->addChild(label, 1000);
        label->runAction(Sequence::create(
            DelayTime::create(2.5f),
            RemoveSelf::create(),
            nullptr
        ));

        //显示大本营升级提示
        if (town_hall_level < 10)
        {
            std::string unlock_info = upgrade_limits->getUnlockInfoForNextTownHallLevel(town_hall_level);
            auto info_label = Label::createWithTTF(unlock_info, "fonts/Marker Felt.ttf", 24);
            info_label->setPosition(visibleSize.width / 2, visibleSize.height / 2 - 50);
            info_label->setColor(Color3B::YELLOW);
            info_label->setDimensions(400, 0);
            info_label->setAlignment(TextHAlignment::CENTER);
            scene->addChild(info_label, 1000);
            info_label->runAction(Sequence::create(
                DelayTime::create(3.0f),
                RemoveSelf::create(),
                nullptr
            ));
        }

        return;
    }

    // 2. 计算本次升级需要的花费
    int cost = this->getNextLevelCost();

    // 3. 根据建筑类型扣除对应的资源
    bool is_enough = false;

    if (type_ == BuildingType::BARRACKS)
    {
        // 兵营：扣圣水
        if (water_count >= cost)
        {
            water_count -= cost; // 扣费
            is_enough = true;
            log("Spent %d Water for upgrade.", cost);
        }
        else
        {
            log("Not enough Water!");
        }
    }
    else
    {
        // 其他(大本营/金矿等)：扣金币
        if (coin_count >= cost)
        {
            coin_count -= cost; // 扣费
            is_enough = true;
            log("Spent %d Coin for upgrade.", cost);
        }
        else
        {
            log("Not enough Coin!");
        }
    }

    // 4. 只有钱够了，才开始升级倒计时
    if (is_enough)
    {
        state_ = BuildingState::UPGRADING;

        // 设置时间（基础时间 * 等级）
        time_left_ = 5.0f * level_;

        log("Upgrade started... Time left: %f", time_left_);
    }
    else
    {
        // 钱不够
        log("Cannot upgrade: Insufficient resources.");
    }
}

// 宝石加速
void Building::speedUp()
{
    int cost = getSpeedUpCost();
    if (gem_count >= cost)
    {
        gem_count -= cost;
        finishUpgrade(); // 瞬间完成
    }
    else
    {
        log("Not enough gems!");
    }
}

// 初始化触摸监听器
void Building::initTouchListener()
{
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);

    // --- 触摸开始（拿起建筑） ---
    listener->onTouchBegan = [=](Touch* touch, Event* event)
        {
            Vec2 touch_pos = touch->getLocation();
            Vec2 node_pos = this->getParent()->convertToNodeSpace(touch_pos);

            if (this->getBoundingBox().containsPoint(node_pos))
            {
                // ================== 【拦截开始】 ==================
                auto gameScene = dynamic_cast<GameScene*>(Director::getInstance()->getRunningScene());
                extern std::string g_currentUsername;
                if (gameScene && gameScene->current_scene_owner_ != g_currentUsername)
                {
                    // 访客模式下，点击建筑依然返回true（为了触发后面onTouchEnded的弹窗查看信息）
                    // 但我们要标记，这不是一次合法的"拿起"操作
                    this->is_dragging_ = false;
                    return true;
                }
                // ================== 【拦截结束】 =================

                // 1. 记录数据
                touch_offset_ = this->getPosition() - node_pos;
                original_pos_ = this->getPosition();
                is_dragging_ = false;

                // 2. 视觉反馈：变大
                this->setScale(0.55f);

                // 3. 【关键】拿起建筑时，移除地基！
                // 这样原来的位置就会露出草地，看起来像是恢复了原色
                this->removeGroundEffect();

                return true;
            }
            return false;
        };

    // --- 触摸移动（保持不变） ---
    listener->onTouchMoved = [=](Touch* touch, Event* event)
        {
            // ================== 【拦截开始】 ==================
            auto gameScene = dynamic_cast<GameScene*>(Director::getInstance()->getRunningScene());
            extern std::string g_currentUsername;
            if (gameScene && gameScene->current_scene_owner_ != g_currentUsername)
            {
                return; // 访客禁止移动
            }
            // ================== 【拦截结束】 ==================

            if (touch->getStartLocation().distance(touch->getLocation()) > 10.0f)
            {
                is_dragging_ = true;
            }
            Vec2 world_new_pos = touch->getLocation() + touch_offset_;
            Vec2 node_pos = this->getParent()->convertToNodeSpace(world_new_pos);
            this->setPosition(node_pos);
        };

    // --- 触摸结束（放下建筑） ---
    listener->onTouchEnded = [=](Touch* touch, Event* event)
        {
            this->setScale(0.5f); // 恢复大小

            // ================== 【核心修改：访客模式拦截】 ==================
            auto gameScene = dynamic_cast<GameScene*>(Director::getInstance()->getRunningScene());
            extern std::string g_currentUsername;

            // 如果当前场景的主人不是我，说明我在参观
            if (gameScene && gameScene->current_scene_owner_ != g_currentUsername)
            {
                log("Visitor mode: Block all building interactions.");
                is_dragging_ = false;
                // 直接返回，不执行任何拖拽判定，更不执行下面的弹窗代码
                return;
            }

            if (is_dragging_)
            {
                auto gameScene = dynamic_cast<GameScene*>(Director::getInstance()->getRunningScene());

                if (gameScene)
                {
                    // 获取碰撞检测框（使用Local坐标）
                    Rect my_local_rect = this->getBoundingBox();
                    // 稍微缩小判定范围优化手感
                    my_local_rect.origin.x += 10;
                    my_local_rect.origin.y += 10;
                    my_local_rect.size.width -= 20;
                    my_local_rect.size.height -= 20;

                    // 检测碰撞
                    if (gameScene->checkCollision(my_local_rect, this))
                    {
                        // === 发生碰撞，弹回原处 ===
                        log("COLLISION! Back to origin.");

                        auto seq = Sequence::create(
                            MoveTo::create(0.1f, original_pos_),
                            CallFunc::create([=]()
                                {
                                    // 动画结束后，强制归位
                                    this->setPosition(original_pos_);
                                    this->setLocalZOrder(10000 - static_cast<int>(original_pos_.y));

                                    // 【关键】回到原位后，重新创建地基
                                    this->createGroundEffect();
                                }),
                            NULL
                        );
                        this->runAction(seq);
                    }
                    else
                    {
                        // === 放置成功 ===
                        log("Placed OK.");
                        original_pos_ = this->getPosition();
                        this->setLocalZOrder(10000 - static_cast<int>(this->getPositionY()));

                        // 【关键】在新位置创建地基！
                        // 这样新位置的地面就变色了
                        this->createGroundEffect();
                    }
                }
                is_dragging_ = false;
            }
            else
            {
                // === 点击事件（不是拖拽） ===
                // 点击虽然没动位置，但为了保险，或者为了视觉一致性，也可以重新刷一下地基
                this->createGroundEffect();

                this->setLocalZOrder(10000 - static_cast<int>(this->getPositionY()));

                // 如果是墙、防御建筑、大炮、塔等，不弹出信息窗口
                if (this->type_ == BuildingType::WALL ||  this->type_ == BuildingType::CANNON || this->type_ == BuildingType::TOWER)
                {
                    log("Clicked on a Wall/Defense - No popup.");
                    return; // 直接结束，不执行后面的弹窗代码
                }

                // 弹出信息窗口
                auto info_layer = BuildingInfoLayer::create();
                info_layer->setBuilding(this);
                Director::getInstance()->getRunningScene()->addChild(info_layer, 999);
            }
        };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

// 完成升级
void Building::finishUpgrade()
{
    // 1. 只有正在升级才能结算
    if (state_ != BuildingState::UPGRADING)
    {
        return;
    }

    // 2. 状态变回空闲
    state_ = BuildingState::IDLE;
    time_left_ = 0;

    // 3. 等级 +1
    level_++;

    // 【新增】根据建筑类型处理不同的升级效果
    switch (type_)
    {
        case BuildingType::BARRACKS:
        {
            // 兵营：重新计算军队上限
            auto game_scene = dynamic_cast<GameScene*>(Director::getInstance()->getRunningScene());
            if (game_scene)
            {
                game_scene->recalculateArmyLimit();
                CCLOG("=== Building: Barracks upgraded to level %d, army limit recalculated ===", level_);
            }
            break;
        }

        case BuildingType::GOLD_STORAGE:
        {
            // 金币储存器：增加金币上限
            coin_limit += 1500; // 每级增加1500容量
            CCLOG("=== Building: Gold Storage upgraded to level %d, coin limit +1500 ===", level_);
            break;
        }

        case BuildingType::WATER_STORAGE:
        {
            // 圣水储存器：增加圣水上限
            water_limit += 1500; // 每级增加1500容量
            CCLOG("=== Building: Water Storage upgraded to level %d, water limit +1500 ===", level_);
            break;
        }

        case BuildingType::BASE:
        {
            // 大本营：同时增加两种资源上限（已有逻辑）
            coin_limit += 1500;
            water_limit += 1500;
            CCLOG("=== Building: Town Hall upgraded to level %d, both limits +1500 ===", level_);
            break;
        }

        case BuildingType::MINE:
        case BuildingType::WATER:
        {
            // 生产建筑：增加生产量
            production_amount_ = 50 * level_;
            CCLOG("=== Building: Production building upgraded to level %d, production +%d ===",
                level_, production_amount_);
            break;
        }
    }

    // 4. 【核心】在这里调用回调！
    // 这时候才会执行GameScene里写的coin_limit += 1500代码
    if (upgrade_callback_)
    {
        upgrade_callback_();
    }

    log("Upgrade finished! Level is now %d", level_);
}

// 创建地基效果
void Building::createGroundEffect()
{
    // 1. 如果已经存在特效，先移除，防止叠加
    if (ground_effect_node_)
    {
        ground_effect_node_->removeFromParent();
        ground_effect_node_ = nullptr;
    }

    auto ground_sprite = Sprite::create("map/dirt_patch.png");

    // 墙类建筑不显示地基
    if (this->type_ == BuildingType::WALL)
    {
        return;
    }

    if (ground_sprite)
    {
        // 3. 获取尺寸信息
        Size building_size = this->getContentSize(); // 建筑的原始大小
        Size sprite_size = ground_sprite->getContentSize(); // 图片的原始大小

        // 4. 【关键】自动拉伸图片以适应不同大小的建筑
        // 这样无论你的建筑是大本营还是兵营，这张地基图都会自动铺满底部
        ground_sprite->setScaleX(building_size.width / (sprite_size.width * 0.8));
        ground_sprite->setScaleY(building_size.height / (sprite_size.height * 1.5));

        // 5. 设置位置：居中
        // 因为是加在建筑(this)身上的，所以位置是建筑的中心点
        ground_sprite->setPosition(Vec2(building_size.width / 2, building_size.height / 2 - 50));
        ground_sprite->getTexture()->setAliasTexParameters();

        // 6. 添加到建筑本身
        // ZOrder -1 保证显示在建筑图片的底部
        this->addChild(ground_sprite, -100);

        // 7. 保存指针
        ground_effect_node_ = ground_sprite;
    }
    else
    {
        log("Error: Failed to load ground effect image: map/dirt.png");
    }
}

// 移除地基效果
void Building::removeGroundEffect()
{
    if (ground_effect_node_)
    {
        ground_effect_node_->removeFromParent();
        ground_effect_node_ = nullptr;
    }
}

// 【新增】直接设置等级，不消耗资源（用于加载存档）
void Building::setLevelDirectly(int level)
{
    if (level < 1)
    {
        level = 1;
    }

    // 大本营最高10级限制
    if (type_ == BuildingType::BASE && level > 10)
    {
        level = 10;
        CCLOG("=== Building: Town Hall max level is 10, setting to 10 ===");
    }

    // 设置等级
    level_ = level;

    CCLOG("=== Building: %s level set to %d (no resource cost) ===", building_name_.c_str(), level_);
}

// 【新增】直接设置状态
void Building::setStateDirectly(BuildingState newState)
{
    state_ = newState;

    // 如果是金矿或圣水收集器且状态为READY，显示可收集标记
    if ((type_ == BuildingType::MINE || type_ == BuildingType::WATER) && state_ == BuildingState::READY)
    {
        if (!ready_indicator_)
        {
            ready_indicator_ = Sprite::create("ui/ready_indicator.png");
            if (!ready_indicator_)
            {
                // 如果图片不存在，创建一个简单的红色感叹号
                ready_indicator_ = Sprite::create();
                auto label = Label::createWithTTF("!", "fonts/Marker Felt.ttf", 72);
                label->setColor(Color3B::RED);
                label->setPosition(Vec2(15, 50));
                ready_indicator_->addChild(label);
                ready_indicator_->setContentSize(Size(30, 30));
            }
            ready_indicator_->setPosition(Vec2(this->getContentSize().width / 2, this->getContentSize().height + 20));
            this->addChild(ready_indicator_, 100);
        }
    }
    else if (ready_indicator_)
    {
        // 如果状态不是READY，移除可收集标记
        ready_indicator_->removeFromParent();
        ready_indicator_ = nullptr;
    }

    CCLOG("=== Building: %s state set to %d ===", building_name_.c_str(), (int)state_);
}

// 设置升级剩余时间
void Building::setUpgradeTimeLeft(float time)
{
    time_left_ = time;
    if (time_left_ > 0)
    {
        state_ = BuildingState::UPGRADING;
    }
    CCLOG("=== Building: %s upgrade time left set to %.2f ===", building_name_.c_str(), time_left_);
}

// 设置生产剩余时间
void Building::setProductionTimeLeft(float time)
{
    production_time_left_ = time;
    if (production_time_left_ > 0)
    {
        state_ = BuildingState::PRODUCING;
    }
    CCLOG("=== Building: %s production time left set to %.2f ===", building_name_.c_str(), production_time_left_);
}

// 从数据初始化建筑（用于加载存档）
void Building::initFromSaveData(int level, BuildingState savedState, float upgradeTimeLeft, float productionTimeLeft)
{
    // 1. 设置等级
    setLevelDirectly(level);

    // 2. 设置状态
    setStateDirectly(savedState);

    // 3. 设置升级剩余时间
    if (savedState == BuildingState::UPGRADING)
    {
        setUpgradeTimeLeft(upgradeTimeLeft);
    }

    // 4. 设置生产相关逻辑
    if (type_ == BuildingType::MINE || type_ == BuildingType::WATER)
    {
        if (savedState == BuildingState::PRODUCING)
        {
            // 只要是在生产中，加载时直接算它生产完成了！
            // 这样既解决了离线问题，又解决了卡住问题，玩家也会很开心。
            this->state_ = BuildingState::PRODUCING; // 先假装它在生产
            this->finishProduction(); // 然后立刻完成
        }
        else if (savedState == BuildingState::READY)
        {
            // 如果存档里已经是READY，不要调用finishProduction()（因为它会因为状态不对被拦截）
            // 而是直接手动设置这一项：
            this->is_ready_to_collect_ = true;

            // 确保图标显示出来（setStateDirectly里虽然有写，但双重保险）
            if (!ready_indicator_)
            {
                // 借用setStateDirectly的逻辑或者finishProduction的一部分逻辑刷新UI
                // 为了简单，我们手动触发一次状态刷新
                this->setStateDirectly(BuildingState::IDLE); // 先重置
                this->setStateDirectly(BuildingState::READY); // 再设为READY，这会触发图标创建
            }
        }
    }

    CCLOG("=== Building: %s initialized - ReadyToCollect: %s ===",
        building_name_.c_str(), is_ready_to_collect_ ? "YES" : "NO");
}

// 获取储存器增加的容量
int Building::getStorageCapacityIncrease() const
{
    // 根据建筑类型和等级计算增加的容量
    // 这里假设每级增加1500容量（与大本营一致）
    if (type_ == BuildingType::GOLD_STORAGE || type_ == BuildingType::WATER_STORAGE)
    {
        return 1500 * level_; // 每级增加1500容量
    }
    return 0;
}