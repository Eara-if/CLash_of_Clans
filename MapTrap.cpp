#include "MapTrap.h"

USING_NS_CC;

MapTrap* MapTrap::create(const Rect& area, int damage)
{
    MapTrap* ret = new (std::nothrow) MapTrap();
    if (ret && ret->init(area, damage)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool MapTrap::init(const Rect& area, int damage)
{
    if (!Node::init()) return false;

    _trapArea = area;
    _damage = damage;
    _isExploded = false;

    return true;
}

bool MapTrap::checkTrigger(const Vector<Soldier*>& soldiers)
{
    if (_isExploded) return false;

    // 遍历所有活着的士兵
    for (auto soldier : soldiers) {
        if (soldier && soldier->getCurrentHp() > 0) {

            // 【核心逻辑】判断士兵的位置点，是否在陷阱矩形内
            // getPosition() 是士兵的世界坐标或者地图相对坐标，取决于你的层级结构
            // 假设 Soldier 和 MapTrap 都添加在 _tileMap 上，坐标系一致
            if (soldier->getSoldierType() == SoldierType::AIRFORCE) {
                continue;
            }
            if (_trapArea.containsPoint(soldier->getPosition())) {

                this->explode(soldiers); // 触发爆炸
                return true;
            }
        }
    }
    return false;
}

void MapTrap::explode(const Vector<Soldier*>& soldiers)
{
    _isExploded = true;

    // 1. 播放特效
    this->playExplosionEffect();

    // 2. 造成范围伤害 (所有在矩形内的士兵都掉血)
    for (auto soldier : soldiers) {
        if (soldier && soldier->getCurrentHp() > 0) {
            if (soldier->getSoldierType() == SoldierType::AIRFORCE) {
                continue;
            }
            if (_trapArea.containsPoint(soldier->getPosition())) {
                soldier->takeDamage(_damage);
            }
        }
    }
}

void MapTrap::playExplosionEffect()
{
    // 创建爆炸 Sprite
    auto explosion = Sprite::create();
    // 放在陷阱中心
    explosion->setPosition(_trapArea.origin + _trapArea.size / 2);
    explosion->setScale(3.0f); // 根据你的素材调整大小

    // 如果 Trap 本身加在 TileMap 上，特效也加在 TileMap 上
    Node* mapNode = this->getParent();
    if (mapNode) {
        mapNode->addChild(explosion, 999); // 爆炸特效在最上层
    }
    // 加载动画帧 (复制你 EnemyBuilding 里的逻辑)
    Vector<SpriteFrame*> frames;
    char str[100] = { 0 };
    for (int i = 1; i <= 9; i++) {
        sprintf(str, "soldiers/Explosion%d.png", i);
        auto sp = Sprite::create(str);
        if (sp) frames.pushBack(sp->getSpriteFrame());
    }

    if (!frames.empty()) {
        auto anim = Animation::createWithSpriteFrames(frames, 0.1f);

        // ============================================================
        // 【核心修改】添加生成弹坑的回调
        // ============================================================
        auto leaveCraterFunc = CallFunc::create([mapNode, this]() {
            // 1. 确保地图还在
            if (!mapNode) return;

            // 2. 创建弹坑图片
            // 如果没有 crater.png，你可以暂时用 "map/buildings/mine.png" 并设为黑色
            auto crater = Sprite::create("soldiers/Bomb.png");

            if (crater) {
                // 3. 设置位置 (和陷阱位置一致)
                crater->setPosition(_trapArea.origin + _trapArea.size / 2);

                // 4. 设置外观 (稍微压扁一点更有透视感，变黑表示烧焦)
                crater->setOpacity(200); // 稍微半透明
                crater->setScale(5.0f);  // 大小根据图片调整
                crater->getTexture()->setAliasTexParameters();
                // 5. 【关键】设置 Z-Order
                // 设为 -1 或 0，确保它在士兵脚下，不会遮挡士兵
                mapNode->addChild(crater, 100);

                // (可选) 让弹坑过一段时间慢慢消失，避免地图太乱
                /*
                crater->runAction(Sequence::create(
                    DelayTime::create(10.0f),
                    FadeOut::create(2.0f),
                    RemoveSelf::create(),
                    nullptr
                ));
                */
            }
            });

        // ============================================================
        // 修改动作序列：动画 -> 生成弹坑 -> 移除爆炸特效
        // ============================================================
        explosion->runAction(Sequence::create(
            Animate::create(anim),
            leaveCraterFunc,       // 插入回调
            RemoveSelf::create(),  // 移除爆炸光效本身
            nullptr
        ));
    }
}
