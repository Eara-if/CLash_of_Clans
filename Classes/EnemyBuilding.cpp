#include "EnemyBuilding.h"
#include "Soldier.h"
USING_NS_CC;
//安全的创造一个敌人建筑
EnemyBuilding* EnemyBuilding::create(const std::string& filename, const std::string& hpBarFilename, int totalHp, int damagePerNotch, int attack, float range)
{
    //申请内存创造一个空对象
    EnemyBuilding* pRet = new(std::nothrow) EnemyBuilding();
    if (pRet && pRet->init(filename, hpBarFilename, totalHp, damagePerNotch, attack, range))
    {
        //创建成功之后自动管理
        pRet->autorelease();
        return pRet;
    }
    else
    {
        delete pRet;
        //指针置空，防止野指针
        pRet = nullptr;
        return nullptr;
    }
}
//强制修改血条的属性
void EnemyBuilding::setHealthBarScale(float s) 
{
    if (_healthBar) {
        _healthBar->stopAllActions();
        _healthBar->setScale(s);
        // 调试用 打印日志
        log("DEBUG: Building at %f, %f set HealthBar Scale to %f", this->getPositionX(), this->getPositionY(), s);
    }
}
//EnemyBuilding的初始化函数
bool EnemyBuilding::init(const std::string& filename, const std::string& hpBarFilename, int totalHp, int damagePerNotch, int attack, float range)
{
    //同样先调用父类检测
    if (!Sprite::initWithFile(filename)) 
        return false;

    //最大血量
    _maxHp = totalHp;
    //当前血量
    _currentHp = totalHp;
    //设置单位血量
    _damagePerNotch = damagePerNotch;
    //是否被摧毁
    _isDestroyed = false;

    //创造一个血条
    _healthBar = Sprite::create(hpBarFilename);
    if (_healthBar)
    {
        //由于血条图片是资格一组的长图，这里根据血量占最大容量的比例进行计算
        float textureWidth = _healthBar->getContentSize().width;
        float textureHeight = _healthBar->getContentSize().height;
        float frameWidth = textureWidth / 4.0f;
        float startX = 3.0f * frameWidth;
        _healthBar->setTextureRect(Rect(startX, 0, frameWidth, textureHeight));

        //放在建筑的头顶
        _healthBar->setPosition(Vec2(this->getContentSize().width / 2, this->getContentSize().height + 20));

        //把血条放大一点
        _healthBar->setScale(3.0f);
        //把血条放在建筑上
        this->addChild(_healthBar,100);
    }
    else
    {
        //防止野指针闪退
        _healthBar = nullptr;
    }

    //设置攻击力
    _attackPower = attack;
    //攻击范围
    _attackRange = range;
    //每次攻击的间隔
    _attackCooldown = 1.0f; 
    //计时器归零，用于按帧计算是否要开炮了
    _attackTimer = 0.0f;    

    return true;
}
//建筑的攻击函数
void EnemyBuilding::updateTowerLogic(float dt, const cocos2d::Vector<Soldier*>& soldiers)
{
    //如果不是攻击类或者已经被摧毁的，直接退出
    if (_isDestroyed || _attackPower <= 0) 
        return;
    //如果冷却时间还没到，退出
    _attackTimer += dt;
    if (_attackTimer < _attackCooldown) 
        return;

    Soldier* target = nullptr;
    float minDistance = _attackRange;

    // 获取建筑自身的世界坐标
    Vec2 myWorldPos = this->getParent() ? this->getParent()->convertToWorldSpace(this->getPosition()) : this->getPosition();

    // 内部 Lambda 函数用于统一判定逻辑，减少重复代码
    auto findTarget = [&](bool onlyFlying, bool ignoreFlying) {
        for (auto soldier : soldiers) {
            if (soldier && soldier->getParent() && soldier->getCurrentHp() > 0) {
                //区分建筑的不同，有一些可以攻击天空兵种有一些不行
                if (onlyFlying && !soldier->isFlying())
                    continue;
                if (ignoreFlying && soldier->isFlying()) 
                    continue;

                // 使用士兵的世界坐标进行距离计算
                Vec2 soldierWorldPos = soldier->getParent()->convertToWorldSpace(soldier->getPosition());
                float dist = myWorldPos.distance(soldierWorldPos);
                //如果士兵在塔的攻击范围内，设定士兵为目标，有更近的打更近的
                if (dist < minDistance) {
                    minDistance = dist;
                    target = soldier;
                }
            }
        }
        };
    //加农炮的攻击逻辑
    if (_type == EnemyType::CANNON) {
        findTarget(true, false);  // 先找空军
        if (!target) {
            minDistance = _attackRange; // 重置距离
            findTarget(false, true); // 再找陆军
        }
    }
    //防御塔的攻击逻辑
    else if (_type == EnemyType::TOWER) {
        findTarget(false, true); // 只打陆军
    }
    else {
        findTarget(false, false); // 什么都打
    }

    if (target) {
        //如果有目标，发射炮弹
        fireMissile(target);
        //计时器清零，进入下一次冷却周期
        _attackTimer = 0.0f;
    }
}
//发射导弹对士兵造成伤害的函数
void EnemyBuilding::fireMissile(Soldier* target)
{
    //如果被摧毁了或者没有目标，退出
    if (_isDestroyed || !target) 
        return;

    // 创建导弹对象
    auto missile = Sprite::create("weapon/cannonball.png"); 
    if (!missile) 
        return;
    //设定导弹位置
    missile->setPosition(this->getContentSize().width / 2, this->getContentSize().height / 2 + 50);
    missile->setScale(0.8f);
    this->addChild(missile, 10);

    //计算导弹飞行所需的时长
    //先进行坐标系转换，找到要被打的士兵
    Vec2 targetPosInTower = this->convertToNodeSpace(target->getParent()->convertToWorldSpace(target->getPosition()));
    //计算当前导弹和士兵之间的距离
    float distance = missile->getPosition().distance(targetPosInTower);
    //设定速度，之后计算出飞行时间
    float speed = 400.0f;
    float duration = distance / speed;

    //计算方向向量
    Vec2 direction = targetPosInTower - missile->getPosition();
    //计算角度，决定导弹飞行的方向
    float angle = CC_RADIANS_TO_DEGREES(atan2(direction.y, direction.x));
    //旋转导弹
    missile->setRotation(-angle);
    //锁定伤害值，不要因为瞬间塔被摧毁影响
    int damage = this->_attackPower;
    //锁定目标内存，防止闪退，不要让导弹找不到人
    target->retain();

   //创建一个动作序列
    auto seq = Sequence::create(
        //先飞过去
        MoveTo::create(duration, targetPosInTower),
        //确认士兵还活着吗
        CallFunc::create([target, missile, damage]() { 
           //活着，造成伤害
            if (target->getParent()) {
                target->takeDamage(damage);
            }
            //销毁导弹
            missile->removeFromParent();
            //这个时候士兵的内存才会真正销毁
            target->release();
            }),
        nullptr
    );
    //动作序列实施
    missile->runAction(seq);

}

//对目标士兵造成伤害
void EnemyBuilding::takeDamage(int damage)
{
    //被摧毁的退出
    if (_isDestroyed) 
        return;
    //扣血
    _currentHp -= damage;
    if (_currentHp < 0) 
        _currentHp = 0;

    // 更新血条图片
    this->updateHealthBar();
    //如果血量为0了
    if (_currentHp == 0) {
        _isDestroyed = true;

        // 实现一个爆炸的效果
        this->playExplosionEffect();
        //调试用
        log("Building Destroyed!");

        //将被摧毁后的建筑变成灰色
        this->setColor(Color3B::GRAY);

        // 攻击力清零
        _attackPower = 0;

        if (_healthBar) {
            _healthBar->setVisible(false);
        }
    }
}

//更新血量值的图片
void EnemyBuilding::updateHealthBar()
{
    if (!_healthBar) 
        return;

    //计算掉了多少血
    int lostHp = _maxHp - _currentHp;
    //换算成掉了几格
    int lostNotches = lostHp / _damagePerNotch;

    // 血量格数0-3，掉到3封顶
    if (lostNotches > 3)
        lostNotches = 3;

    // 计算裁剪区域
    float textureWidth = _healthBar->getTexture()->getContentSize().width; 
    float textureHeight = _healthBar->getTexture()->getContentSize().height;
    float frameWidth = textureWidth / 4.0f; 
    //最终的图片设置
    _healthBar->setTextureRect(Rect((3 - lostNotches) * frameWidth, 0, frameWidth, textureHeight));
}

//制造建筑摧毁时的爆炸效果
void EnemyBuilding::playExplosionEffect()
{
    //创建一个爆炸效果对象
    auto explosionSprite = Sprite::create();
    //设定位置和大小
    explosionSprite->setPosition(this->getPosition());
    explosionSprite->setScale(5.0f);
    //把爆炸添加到地图上
    if (this->getParent()) {
        this->getParent()->addChild(explosionSprite, this->getLocalZOrder() + 10);
    }
    //备用 应该没用
    else {
        this->addChild(explosionSprite, 20);
        explosionSprite->setPosition(Vec2(this->getContentSize().width / 2, this->getContentSize().height / 2));
    }
    //加载动画帧
    Vector<SpriteFrame*> animFrames;
    char str[100] = { 0 };

    for (int i = 1; i <= 9; i++)
    {
        sprintf(str, "soldiers/Explosion%d.png", i);
        auto sprite = Sprite::create(str);
        if (sprite) {
            //把这个动画帧放在一个Vector容器里
            animFrames.pushBack(sprite->getSpriteFrame());
        }
    }
    //间隔0.1播放
    auto animation = Animation::createWithSpriteFrames(animFrames, 0.1f);
    //创建动作序列，播放后毁掉这个对象
    auto seq = Sequence::create(
        Animate::create(animation),
        RemoveSelf::create(),
        nullptr
    );
    //使用这个动作序列
    explosionSprite->runAction(seq);
}