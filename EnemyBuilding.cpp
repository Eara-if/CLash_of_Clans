#include "EnemyBuilding.h"
// 【新增】在这里包含 Soldier.h
#include "Soldier.h"

USING_NS_CC;


// 1. 实现 create
EnemyBuilding* EnemyBuilding::create(const std::string& filename, const std::string& hpBarFilename, int totalHp, int damagePerNotch, int attack, float range)
{
    EnemyBuilding* pRet = new(std::nothrow) EnemyBuilding();
    // 传入新增参数
    if (pRet && pRet->init(filename, hpBarFilename, totalHp, damagePerNotch, attack, range))
    {
        pRet->autorelease();
        return pRet;
    }
    else
    {
        delete pRet;
        pRet = nullptr;
        return nullptr;
    }
}

bool EnemyBuilding::init(const std::string& filename, const std::string& hpBarFilename, int totalHp, int damagePerNotch, int attack, float range)
{
    // 1. 初始化建筑本身的图片 
    if (!Sprite::initWithFile(filename)) return false;

    _maxHp = totalHp;
    _currentHp = totalHp;
    _damagePerNotch = damagePerNotch;

    // 【新增】初始化摧毁状态为 false
    _isDestroyed = false;

    // 2. 创建血条
    _healthBar = Sprite::create(hpBarFilename);
    if (_healthBar)
    {
        // 【核心逻辑】血条四等分初始化
        // 获取整张血条图片的尺寸
        float textureWidth = _healthBar->getContentSize().width;
        float textureHeight = _healthBar->getContentSize().height;

        // 计算每一格的宽度 (一共4个状态)
        float frameWidth = textureWidth / 4.0f;

        // 初始状态：显示第 4 格 (满血)，x 坐标应为 3 * frameWidth
        float startX = 3.0f * frameWidth;
        _healthBar->setTextureRect(Rect(startX, 0, frameWidth, textureHeight));

        // 把血条放在建筑头顶
        // Vec2(内容宽度的一半, 内容高度 + 一点偏移)
        _healthBar->setPosition(Vec2(this->getContentSize().width / 2, this->getContentSize().height + 20));
        _healthBar->setScale(3.0f);

        this->addChild(_healthBar);
    }

    // 【新增】初始化攻击属性
    _attackPower = attack;
    _attackRange = range;
    _attackCooldown = 1.0f; // 假设1秒攻击一次，也可以做成参数传入
    _attackTimer = 0.0f;    // 计时器归零

    return true;
}

void EnemyBuilding::updateTowerLogic(float dt, const cocos2d::Vector<Soldier*>& soldiers)
{
    // 【修改】1. 如果防御塔已被摧毁，直接返回
    if (_isDestroyed) return;

    // 1. 如果不是防御塔(比如只是Base或者攻击力为0)，直接返回
    if (_attackPower <= 0) return;

    // 2. 更新冷却计时器
    _attackTimer += dt;
    if (_attackTimer < _attackCooldown) {
        return; // 还在冷却中
    }

    // 3. 寻找最近的敌人
    Soldier* target = nullptr;
    float minDistance = _attackRange; // 只找射程内的

    Vec2 myPos = this->getPosition();

    for (auto soldier : soldiers) {
        // 确保士兵存活且不仅是空指针
        if (soldier && soldier->getParent()) {
            float dist = myPos.distance(soldier->getPosition());
            if (dist < minDistance) {
                minDistance = dist;
                target = soldier;
            }
        }
    }

    // 4. 如果找到了目标，发射导弹
    if (target) {
        fireMissile(target);
        _attackTimer = 0.0f; // 重置计时器
    }
}

void EnemyBuilding::fireMissile(Soldier* target)
{
    // 【修改】检查防御塔是否已被摧毁
    if (_isDestroyed) return;

    // 1. 创建导弹 Sprite
    auto missile = Sprite::create("weapon/cannonball.png"); // 替换成你的导弹图片
    missile->setPosition(this->getContentSize().width / 2, this->getContentSize().height / 2 + 50); // 从塔中心发射
    missile->setScale(0.8f); // 调整大小
    this->addChild(missile, 10); // 加在塔上，或者加在 BattleScene 地图层更好(需坐标转换)，这里简化先加在塔上

    // 2. 计算目标位置
    // 注意：因为 missile 是加在 Tower 上的，target 的位置是世界坐标(或地图坐标)，需要转换
    // 如果 Tower 和 Soldier 都在 tileMap 上，计算相对位置即可：
    Vec2 targetPosInTower = this->convertToNodeSpace(target->getParent()->convertToWorldSpace(target->getPosition()));

    // 3. 计算飞行时间 (距离越远飞得越久，保持速度恒定)
    float distance = missile->getPosition().distance(targetPosInTower);
    float speed = 400.0f; // 导弹速度 (像素/秒)
    float duration = distance / speed;

    // 4. 创建动作序列
    // 旋转角度 (可选)：让导弹头朝向敌人
    Vec2 direction = targetPosInTower - missile->getPosition();
    float angle = CC_RADIANS_TO_DEGREES(atan2(direction.y, direction.x));
    missile->setRotation(-angle); // Cocos 的旋转方向和数学相反

    // 动作：移动 -> 造成伤害 -> 移除导弹
    auto seq = Sequence::create(
        MoveTo::create(duration, targetPosInTower),
        CallFunc::create([target, this, missile]() {
            // 这是一个 Lambda 回调，导弹到达时执行

            // 【重要】再次检查 target 是否还活着，防止导弹飞行途中士兵死了导致空指针崩溃
            // 简单的判断方法是看它是否还在内存池或者 flag
            // 实际项目中建议给 Soldier 加个 isDead() 标记
            if (target && target->getParent()) {
                // 造成伤害 (假设 Soldier 有 takeDamage 函数)
                target->takeDamage(this->_attackPower);
            }

            // 移除导弹
            missile->removeFromParent();
            }),
        nullptr
    );

    missile->runAction(seq);
}



void EnemyBuilding::takeDamage(int damage)
{
    // 如果已经被摧毁，不再处理伤害
    if (_isDestroyed) return;

    _currentHp -= damage;
    if (_currentHp < 0) _currentHp = 0;

    // 扣血后刷新血条
    this->updateHealthBar();

    if (_currentHp == 0) {
        // 标记为已摧毁
        _isDestroyed = true;

        // 【新增】播放爆炸动画
        this->playExplosionEffect();

        // 死亡逻辑
        log("Building Destroyed!");

        // 变灰表示损毁 (你可以保留这个，也可以去掉，看爆炸后的效果)
        this->setColor(Color3B::GRAY);

        // 将攻击力设置为0
        _attackPower = 0;

        // 隐藏血条（可选：通常建筑炸了血条也应该消失）
        if (_healthBar) {
            _healthBar->setVisible(false);
        }
    }
}


void EnemyBuilding::updateHealthBar()
{
    if (!_healthBar) return;

    // 【核心逻辑】根据血量计算显示第几张图
    // 假设图片顺序是：[满血] [少血] [残血] [空血] (索引 0, 1, 2, 3)

    // 计算当前掉血掉了几个 Notch (格)
    int lostHp = _maxHp - _currentHp;
    int lostNotches = lostHp / _damagePerNotch;

    // 限制索引在 0 到 3 之间
    if (lostNotches > 3) lostNotches = 3;

    // 计算切图区域
    float textureWidth = _healthBar->getTexture()->getContentSize().width; // 原图总宽
    float textureHeight = _healthBar->getTexture()->getContentSize().height;
    float frameWidth = textureWidth / 4.0f; // 单格宽

    // 设置显示的区域
    // x 坐标 = 索引 * 单格宽
    _healthBar->setTextureRect(Rect((3 - lostNotches) * frameWidth, 0, frameWidth, textureHeight));
}

void EnemyBuilding::playExplosionEffect()
{
    // 1. 创建爆炸 Sprite
    auto explosionSprite = Sprite::create();

    // 【修改点1】坐标设置
    // 因为我们要把它加到地图上（和塔同级），所以位置直接等于塔在地图上的位置
    explosionSprite->setPosition(this->getPosition());

    explosionSprite->setScale(5.0f); // 保持你调整好的大小

    // 【修改点2】添加到父节点（地图）而不是 this
    // this->getLocalZOrder() + 10 确保它永远比塔（this）高 10 层，显示在最前面
    if (this->getParent()) {
        this->getParent()->addChild(explosionSprite, this->getLocalZOrder() + 10);
    }
    else {
        //以此防备万一塔还没有父节点的情况（虽然不太可能）
        this->addChild(explosionSprite, 20);
        explosionSprite->setPosition(Vec2(this->getContentSize().width / 2, this->getContentSize().height / 2));
    }

    // 2. 加载动画帧 (保持原样)
    Vector<SpriteFrame*> animFrames;
    char str[100] = { 0 };

    for (int i = 1; i <= 9; i++)
    {
        sprintf(str, "soldiers/Explosion%d.png", i);
        auto sprite = Sprite::create(str);
        if (sprite) {
            animFrames.pushBack(sprite->getSpriteFrame());
        }
    }

    // 3. 创建动画 (保持原样)
    auto animation = Animation::createWithSpriteFrames(animFrames, 0.1f);

    // 4. 动作序列 (保持原样)
    auto seq = Sequence::create(
        Animate::create(animation),
        RemoveSelf::create(),
        nullptr
    );

    explosionSprite->runAction(seq);
}