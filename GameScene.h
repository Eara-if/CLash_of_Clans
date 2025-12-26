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
    // ���������ڴ洢��ǰ���������н���������
    cocos2d::Vector<Building*> _allBuildings;
    // ����������ѹ������ĸ�������
    void addAllPurchasedBuildings();
    bool isMapDragging;
public:
    void addSaveButton();
    void menuSaveGameCallback(Ref* pSender);
    cocos2d::Vector<Node*>_obstacles;
    // ���������ľ�̬����
    // �޸�createScene��ʹ����Խ���һ����ѡ�Ľ�������
    static cocos2d::Scene* createScene(Building* purchasedBuilding = nullptr);
    bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event);
    void onTouchMoved(cocos2d::Touch* touch, cocos2d::Event* event);
    void onTouchEnded(cocos2d::Touch* touch, cocos2d::Event* event);
    // GameScene.h
    void GameScene::onManualSaveClicked();
    void showToast(std::string message, cocos2d::Color3B color);
    void onAutoSave(float dt); // 定时器回调

    bool _isVisiting = false; // 是否正在拜访别人
    void loadOtherPlayerData(std::string targetUsername); // 加载别人的数据
    void setupLeftPanel(); // 初始化左侧面板
    std::string _currentSceneOwner; // 记录当前地图的主人
    // �������������ֻص� (��������)
    void onMouseScroll(cocos2d::Event* event);

    // �����������ĸ�����������鲢������ͼλ�� (��ֹ����)
    void checkAndClampMapPosition();

    // �����������ݱ�Ӫ�ȼ������˿�����
    int calculateArmyLimit();
    // �����������¼��㲢�����˿�����
    void recalculateArmyLimit();

    void addShopButton();
    void menuAttackCallback(cocos2d::Ref* pSender);
    // ��ʼ������
    virtual bool init();
    // ��������дonEnter����
    void removeObstacle(Node* node);
    void addObstacle(Node* node);
    cocos2d::Rect getWorldBoundingBox(Node* node);
    bool checkCollision(cocos2d::Rect targetRect, Node* ignoreNode);
    cocos2d::Vec2 getNearestFreePosition(Building* building, cocos2d::Vec2 targetMapPos);
    virtual void onEnter() override;
    void menuGotoBattleCallback(cocos2d::Ref* pSender); 
    void menuBackCallback(cocos2d::Ref* pSender);
    cocos2d::Label* GameScene::showText(std::string content, float x, float y, cocos2d::Color4B color);
    void setbuilding(const std::string& filename, const cocos2d::Rect& rect, const std::string& name, int cost, BuildingType type1, cocos2d::Vec2 position);

    void updateResourceDisplay();
    void GameScene::reloadBuildingsFromSave();

    CREATE_FUNC(GameScene);
};

#endif // __GAME_SCENE_H__