#ifndef SHOP_SCENE_H_
#define SHOP_SCENE_H_

#include "cocos2d.h"
#include "Building.h"

// 商品类型枚举
enum class ShopItemType
{
    WALL,           // 城墙
    GOLD_MINE,      // 金矿
    WATER_COLLECTOR,// 圣水收集器
    ARCHER_TOWER,   // 箭塔
    CANNON,         // 加农炮
    BARRACKS,       // 兵营
    GOLD_STORAGE,   // 金币存储器
    WATER_STORAGE   // 圣水存储器
};

// 商品信息结构体
struct ShopItem
{
    ShopItemType type;        // 商品类型
    std::string name;         // 商品名称
    std::string description;  // 商品描述
    int coin_cost;            // 金币价格
    int water_cost;           // 圣水价格
    int gem_cost;             // 宝石价格
    bool purchased;           // 是否已购买
};

class ShopScene : public cocos2d::Scene
{
public:
    // 创建场景静态方法
    static cocos2d::Scene* createScene();

    // 初始化方法
    virtual bool init();

    // 进入场景回调
    void onEnter();

    // 创建菜单
    void createMenu();

    // 商品购买回调函数
    void onWallPurchase(cocos2d::Ref* p_sender);
    void onGoldMinePurchase(cocos2d::Ref* p_sender);
    void onWaterCollectorPurchase(cocos2d::Ref* p_sender);
    void onArcherTowerPurchase(cocos2d::Ref* p_sender);
    void onCannonPurchase(cocos2d::Ref* p_sender);
    void onBarracksPurchase(cocos2d::Ref* p_sender);       
    void onGoldStoragePurchase(cocos2d::Ref* p_sender);  
    void onWaterStoragePurchase(cocos2d::Ref* p_sender);   

    // 核心购买商品方法
    bool purchaseItem(ShopItemType type);

    // 检查是否有足够资源购买商品
    bool hasEnoughResources(int coin_cost, int water_cost, int gem_cost) const;

    // 显示购买成功/失败消息
    void showPurchaseMessage(bool success, const std::string& item_name);

    // 更新资源显示标签
    void updateResourceLabels();

    // 返回按钮回调
    void menuBackCallback(cocos2d::Ref* p_sender);

    // Cocos2d-x的CREATE_FUNC宏，用于创建对象
    CREATE_FUNC(ShopScene);

    // 创建建筑信息标签
    void createBuildingInfoLabels(cocos2d::MenuItemSprite* itemBtn, const ShopItem& info, int currentCount, int maxCount, bool isUnlocked);

private:
    // 资源显示标签
    cocos2d::Label* coin_label_;
    cocos2d::Label* water_label_;
    cocos2d::Label* gem_label_;

    // 当前显示的消息标签
    cocos2d::Label* current_message_label_;

    // 商品缩放常量
    static constexpr float ITEM_SCALE_NORMAL = 0.5f;
    static constexpr float ITEM_SCALE_SELECTED = 0.55f;

    // 颜色常量
    static const cocos2d::Color3B LOCKED_COLOR;
    static const cocos2d::Color3B SELECTED_COLOR;

    // 创建商店菜单项
    cocos2d::MenuItemSprite* createShopMenuItem(const std::string& image_path, bool is_unlocked, const cocos2d::ccMenuCallback& callback);

    // 统计指定类型的建筑数量
    int countBuildingsByType(BuildingType type) const;

    // createMenu函数的辅助方法
    std::vector<cocos2d::MenuItemSprite*> createFirstRowMenuItems();
    std::vector<cocos2d::MenuItemSprite*> createSecondRowMenuItems();

    void getBuildingCounts(int& mineCount, int& waterCount, int& wallCount,
        int& archerCount, int& cannonCount, int& barracksCount,
        int& goldStorageCount, int& waterStorageCount);

    void getBuildingMaxCounts(int& mineMax, int& waterMax, int& wallMax,
        int& archerMax, int& cannonMax, int& barracksMax,
        int& goldStorageMax, int& waterStorageMax);

    void createFirstRowInfoLabels(const std::vector<cocos2d::MenuItemSprite*>& items,
        const std::vector<ShopItem>& infos,
        int mineCount, int mineMax, bool mineUnlocked,
        int waterCount, int waterMax, bool waterUnlocked,
        int wallCount, int wallMax, bool wallUnlocked,
        int archerCount, int archerMax, bool archerUnlocked);

    void createSecondRowInfoLabels(const std::vector<cocos2d::MenuItemSprite*>& items,
        const std::vector<ShopItem>& infos,
        int cannonCount, int cannonMax, bool cannonUnlocked,
        int barracksCount, int barracksMax, bool barracksUnlocked,
        int goldStorageCount, int goldStorageMax, bool goldStorageUnlocked,
        int waterStorageCount, int waterStorageMax, bool waterStorageUnlocked);

    void setMenuItemsPosition(const std::vector<cocos2d::MenuItemSprite*>& items,
        float startX, float yPos, float spacing);

    // purchaseItem函数的辅助方法
    int getBuildingId(ShopItemType type);
    ShopItem getShopItemInfo(ShopItemType type);

    bool canPurchaseItem(ShopItemType type, int& buildingId,
        bool& isUnlocked, int& requiredTHLevel,
        int& maxCount, int& currentCount);

    void deductResources(int coinCost, int waterCost, int gemCost);
    Building* createNewBuilding(ShopItemType type);
    void handleStorageBuildingEffect(ShopItemType type);
    void returnToGameScene();
};

#endif