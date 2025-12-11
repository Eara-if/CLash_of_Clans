#ifndef __GAME_SCENE_H__
#define __GAME_SCENE_H__

#include "cocos2d.h"
#include"Building.h"

extern cocos2d::Vector<Building*> g_allPurchasedBuildings;

class GameScene : public cocos2d::Scene
{
private:
    cocos2d::Label* _coinTextLabel = nullptr;
    cocos2d::Label* _waterTextLabel = nullptr;
    cocos2d::Label* _gemTextLabel = nullptr;
    class goldcoin* myCoin = nullptr;
    class water* mywater = nullptr;
    class Gem* mygem = nullptr;
    cocos2d::TMXTiledMap* _tiledMap = nullptr;
    // 新增：用于存储当前场景中所有建筑的容器
    cocos2d::Vector<Building*> _allBuildings;
    // 新增：添加已购买建筑的辅助函数
    void addAllPurchasedBuildings();
    void addPurchasedBuilding(Building* building); // 新增：添加建筑的函数
public:
    void addSaveButton();
    void menuSaveGameCallback(Ref* pSender);
    cocos2d::Vector<Node*>_obstacles;
    // 创建场景的静态方法
    // 修改createScene，使其可以接收一个可选的建筑参数
    static cocos2d::Scene* createScene(Building* purchasedBuilding = nullptr);
    void setBattleButton();
    void addShopButton();
    // 初始化函数
    virtual bool init();
    // 新增：重写onEnter函数
    void removeObstacle(Node* node);
    void addObstacle(Node* node);
    cocos2d::Rect getWorldBoundingBox(Node* node);
    bool checkCollision(cocos2d::Rect targetRect, Node* ignoreNode);
    cocos2d::Vec2 getNearestFreePosition(Building* building, cocos2d::Vec2 targetMapPos);
    virtual void onEnter() override;
    void menuGotoBattleCallback(cocos2d::Ref* pSender); // 前往战斗场景
    void menuBackCallback(cocos2d::Ref* pSender);
    cocos2d::Label* GameScene::showText(std::string content, float x, float y, cocos2d::Color4B color);
    void setbuilding(const std::string& filename, const cocos2d::Rect& rect, const std::string& name, int cost, BuildingType type1, cocos2d::Vec2 position);
    // 新增：资源更新函数
    void updateResourceDisplay();
    // 这是一个宏，自动帮我们实现 create() 函数
    CREATE_FUNC(GameScene);
};

#endif // __GAME_SCENE_H__