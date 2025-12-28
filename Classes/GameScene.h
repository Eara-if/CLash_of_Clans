#ifndef GAME_SCENE_H_
#define GAME_SCENE_H_

#include "cocos2d.h"
#include"Building.h"

// 全局变量声明：所有已购买的建筑容器
extern cocos2d::Vector<Building*> g_allPurchasedBuildings;

// 游戏场景类定义
class GameScene : public cocos2d::Scene
{
private:
    // UI元素：资源显示标签
    cocos2d::Label* coin_text_label_ = nullptr;
    cocos2d::Label* water_text_label_ = nullptr;
    cocos2d::Label* gem_text_label_ = nullptr;

    // 资源对象指针
    class goldcoin* my_coin_ = nullptr;
    class water* my_water_ = nullptr;
    class Gem* my_gem_ = nullptr;

    // 瓦片地图指针
    cocos2d::TMXTiledMap* tiled_map_ = nullptr;

    // 当前场景中所有建筑的容器
    cocos2d::Vector<Building*> all_buildings_;

    // 将所有已购买建筑添加到场景中
    void addAllPurchasedBuildings();

    // 地图拖动标志
    bool is_map_dragging_;

    // init()函数的辅助函数
    bool loadMap();                     // 加载地图
    void loadMapDecorations();          // 加载地图装饰物
    void loadBuildings();               // 加载建筑
    void initResourceDisplay();         // 初始化资源显示
    void initUIButtons();               // 初始化UI按钮
    void initInputListeners();          // 初始化输入监听
    void initSocialAndAutoSave();       // 初始化社交和自动保存

public:
    // 添加保存按钮
    void addSaveButton();
    // 保存游戏回调
    void menuSaveGameCallback(Ref* p_sender);

    // 障碍物容器
    cocos2d::Vector<Node*> obstacles_;

    // 静态创建场景方法，可选传入已购买的建筑
    static cocos2d::Scene* createScene(Building* purchasedBuilding = nullptr);

    // 触摸事件处理函数
    bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event);
    void onTouchMoved(cocos2d::Touch* touch, cocos2d::Event* event);
    void onTouchEnded(cocos2d::Touch* touch, cocos2d::Event* event);

    // 手动保存点击处理
    void onManualSaveClicked();
    // 显示Toast消息
    void showToast(const std::string& message, const cocos2d::Color3B& color);
    // 自动保存定时器回调
    void onAutoSave(float dt);

    // 加载其他玩家数据
    void loadOtherPlayerData(std::string targetUsername);
    // 初始化左侧面板
    void setupLeftPanel();
    // 当前场景所有者用户名
    std::string current_scene_owner_;

    // 鼠标滚轮事件处理
    void onMouseScroll(cocos2d::Event* event);

    // 检查并限制地图位置，防止地图拖出屏幕
    void checkAndClampMapPosition();

    // 根据兵营等级计算军队上限
    int calculateArmyLimit() const;
    // 重新计算并更新军队上限
    void recalculateArmyLimit();

    // 添加商店按钮
    void addShopButton();
    // 攻击按钮回调
    void menuAttackCallback(cocos2d::Ref* p_sender);

    // 场景初始化
    virtual bool init();

    // 从障碍物列表中移除节点
    void removeObstacle(Node* node);
    // 添加节点到障碍物列表
    void addObstacle(Node* node);
    // 获取节点在世界坐标系中的边界框
    cocos2d::Rect getWorldBoundingBox(Node* node) const;
    // 检查矩形与障碍物是否碰撞
    bool checkCollision(const cocos2d::Rect& target_rect, Node* ignore_node) const;
    // 获取最近空闲位置（用于建筑放置）
    cocos2d::Vec2 getNearestFreePosition(Building* building, cocos2d::Vec2 target_map_pos) const;

    // 进入场景回调
    virtual void onEnter() override;
    // 前往战斗场景回调
    void menuGotoBattleCallback(cocos2d::Ref* p_sender);
    // 返回按钮回调
    void menuBackCallback(cocos2d::Ref* p_sender);

    // 显示文本辅助函数
    cocos2d::Label* showText(const std::string& content, float x, float y, const cocos2d::Color4B& color);
    // 设置建筑辅助函数
    void setbuilding(const std::string& filename, const cocos2d::Rect& rect, const std::string& name, int cost, BuildingType type, const  cocos2d::Vec2& position);

    // 更新资源显示
    void updateResourceDisplay();
    // 从存档重新加载建筑
    void reloadBuildingsFromSave();

    // Cocos2d-x宏，用于创建类的实例
    CREATE_FUNC(GameScene);
};

#endif // GAME_SCENE_H_