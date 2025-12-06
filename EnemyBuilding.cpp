#include "EnemyBuilding.h"

USING_NS_CC;

EnemyBuilding* EnemyBuilding::create(const std::string& filename, const std::string& hpBarFilename, int totalHp, int damagePerNotch)
{
    EnemyBuilding* ret = new (std::nothrow) EnemyBuilding();
    if (ret && ret->init(filename, hpBarFilename, totalHp, damagePerNotch))
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool EnemyBuilding::init(const std::string& filename, const std::string& hpBarFilename, int totalHp, int damagePerNotch)
{
    // 1. 初始化建筑本身的图片 (如果是 TilesetTowers 需要用 Rect 截取，这里先假设是单张或已处理好的)
    // 为了简单，我们这里先假设传入的是普通图片路径。如果是防御塔那种大图，可以在外面截取好 Rect 再传进来，或者重载 create 函数
    if (!Sprite::initWithFile(filename)) return false;

    _maxHp = totalHp;
    _currentHp = totalHp;
    _damagePerNotch = damagePerNotch;

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

    return true;
}

void EnemyBuilding::takeDamage(int damage)
{
    _currentHp -= damage;
    if (_currentHp < 0) _currentHp = 0;

    // 扣血后刷新血条
    this->updateHealthBar();

    if (_currentHp == 0) {
        // 死亡逻辑 (比如播放爆炸动画，从父节点移除等)
        log("Building Destroyed!");
         // 简单示例：变灰表示损毁
        this->setColor(Color3B::GRAY);
        // 或者直接移除： this->removeFromParent();
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
    _healthBar->setTextureRect(Rect((3-lostNotches) * frameWidth, 0, frameWidth, textureHeight));
}