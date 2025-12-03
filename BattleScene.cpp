#include "BattleScene.h"
#include "GameScene.h"
#include "EnemyBuilding.h"

USING_NS_CC;

Scene* BattleScene::createScene()
{
    return BattleScene::create();
}

bool BattleScene::init()
{
    if (!Scene::init())
    {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 1. 加载敌方地图
    this->loadEnemyMap();

    // 2. 添加返回按钮 (保持你之前的代码不变)
    auto backLabel = Label::createWithTTF("Back", "fonts/Marker Felt.ttf", 28);
    backLabel->setTextColor(Color4B::WHITE);

    auto backItem = MenuItemLabel::create(
        backLabel,
        CC_CALLBACK_1(BattleScene::menuBackToGameScene, this)
    );

    backItem->setPosition(Vec2(origin.x + backLabel->getContentSize().width / 2 + 10,
        origin.y + backLabel->getContentSize().height / 2 + 10));

    auto menu = Menu::create(backItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 5);

    return true;
}

void BattleScene::menuBackToGameScene(Ref* pSender)
{
    auto scene = GameScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
}

// 【核心修改】地图生成逻辑
void BattleScene::loadEnemyMap()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // ================= 1. 地图数据定义 =================
    // 0 = Grass (草地), 1 = Bare (土路)
    int mapData[MAP_HEIGHT][MAP_WIDTH] = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
        {1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0},
        {0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0},
        {0, 1, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0},
        {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0},
        {0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
        {0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    };

    // ================= 2. 基础瓦片参数 =================
    float scaleAmount = 0.8f; // 地图缩放比例

    // 获取瓦片尺寸 (假设 grass.png 存在)
    auto tempSprite = Sprite::create("map/background/grass.png");
    float tileWidth = 32 * scaleAmount;  // 默认值
    float tileHeight = 32 * scaleAmount; // 默认值

    if (tempSprite) {
        tileWidth = tempSprite->getContentSize().width * scaleAmount;
        tileHeight = tempSprite->getContentSize().height * scaleAmount;
    }

    // 计算地图起始点 (居中)
    float mapPixelWidth = tileWidth * MAP_WIDTH;
    float mapPixelHeight = tileHeight * MAP_HEIGHT;
    float startX = origin.x + (visibleSize.width - mapPixelWidth) / 2;
    float startY = origin.y + (visibleSize.height - mapPixelHeight) / 2;

    // ================= 3. 生成地面 (循环保持不变) =================
    for (int row = 0; row < MAP_HEIGHT; row++)
    {
        for (int col = 0; col < MAP_WIDTH; col++)
        {
            int type = mapData[row][col];
            std::string filename = (type == 0) ? "map/background/grass.png" : "map/background/bare.png";

            auto tile = Sprite::create(filename);
            if (tile)
            {
                tile->setScale(scaleAmount);
                tile->getTexture()->setAliasTexParameters();

                // 坐标计算公式
                float x = startX + col * tileWidth + tileWidth / 2;
                float y = startY + (MAP_HEIGHT - 1 - row) * tileHeight + tileHeight / 2;

                tile->setPosition(Vec2(x, y));
                this->addChild(tile, -1); // Z-order -1: 地板在最下面
            }
        }
    }

    // ================= 4. 添加大本营 (Base) =================
    {
        int baseRow = 8;
        int baseCol = 5;
        float bx = startX + baseCol * tileWidth + tileWidth / 2;
        float by = startY + (MAP_HEIGHT - 1 - baseRow) * tileHeight + tileHeight / 2 - 50;

        // 【修改点】创建逻辑变了
        // 假设大本营有 4 格血(因为有4个状态图)，每格需要 8 点伤害掉落
        // 总血量 = 4 * 8 = 32
        auto baseObj = EnemyBuilding::create(
            "map/buildings/Base.png",  // 建筑图
            "ui/Heart3.png",           // 血条图
            32,                        // 总血量 (4 * 8)
            8                          // 掉一格血需要的伤害
        );

        if (baseObj) {
            baseObj->setScale(scaleAmount * 0.3f);
            baseObj->setPosition(bx, by);
            baseObj->getTexture()->setAliasTexParameters();

            // 测试一下扣血效果！(运行后你会发现血条变了)
            baseObj->takeDamage(10); // 测试：扣10血，应该掉1格多一点，显示第2个状态

            this->addChild(baseObj, 1);
        }
    }

    // ================= 5. 添加防御塔 (Towers) =================

    // 定义一个结构体来方便批量添加
    struct TowerPos { int r; int c; };

    // 【设置坐标】在这里添加所有的防御塔位置
    std::vector<TowerPos> towerPositions = {
        {1, 4},   // 第1个塔的位置：第1行，第4列
        {3, 8},   // 第2个塔的位置
        {6, 2},   // 第3个塔的位置
        {8, 12}   // ... 可以无限添加
    };

    for (const auto& pos : towerPositions)
    {
        float tx = startX + pos.c * tileWidth + tileWidth / 2;
        float ty = startY + (MAP_HEIGHT - 1 - pos.r) * tileHeight + tileHeight / 2 + 20;

        // 注意：这里为了方便，我们暂时用单张图逻辑。
        // 如果必须用 TilesetTowers.png 截取，你需要修改 EnemyBuilding::init 增加 Rect 参数
        // 这里假设你先把单张塔图存为 "single_tower.png" 或者暂时用全图代替测试

        // 假设防御塔有 4 格血，每格需要 5 点伤害
        // 总血量 = 4 * 5 = 20
        auto tower = EnemyBuilding::create(
            "map/buildings/TilesetTowers.png", // 作为单张图使用
            "ui/Heart2.png",                   
            20,                                
            5                                  
        );

        if (tower) {
            tower->setScale(scaleAmount * 0.2f); // 恢复您指定的缩放
            tower->setPosition(tx, ty);
            tower->getTexture()->setAliasTexParameters();
            
            // 测试扣血 6 点，显示第 2 个状态
            tower->takeDamage(6);

            this->addChild(tower, 1);
        }
    }
}
