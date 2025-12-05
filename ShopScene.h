#ifndef __SHOP_SCENE_H__
#define __SHOP_SCENE_H__

#include "cocos2d.h"

// 商品类型枚举
enum class ShopItemType {
    WALL,           // 城墙
    GOLD_MINE,      // 金矿
    WATER_COLLECTOR,// 圣水收集器
    ARCHER_TOWER,   // 弓箭塔
    CANNON,         // 加农炮
    DECORATION      // 装饰物（可选）
};

// 商品数据结构
struct ShopItem {
    ShopItemType type;
    std::string name;
    std::string description;
    int coinCost;
    int waterCost;
    int gemCost;
    bool purchased;
};

class ShopScene : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();
    virtual bool init();

    // 创建菜单
    void createMenu();

    // 商品按钮回调
    void onWallPurchase(cocos2d::Ref* pSender);
    void onGoldMinePurchase(cocos2d::Ref* pSender);
    void onWaterCollectorPurchase(cocos2d::Ref* pSender);
    void onArcherTowerPurchase(cocos2d::Ref* pSender);
    void onCannonPurchase(cocos2d::Ref* pSender);

    // 购买商品通用方法
    bool purchaseItem(ShopItemType type);

    // 检查资源是否足够
    bool hasEnoughResources(int coinCost, int waterCost, int gemCost);

    // 显示购买成功/失败提示
    void showPurchaseMessage(bool success, const std::string& itemName);

    // 更新资源显示
    void updateResourceLabels();

    // 返回游戏场景
    void menuBackCallback(cocos2d::Ref* pSender);

    CREATE_FUNC(ShopScene);

private:
    cocos2d::Label* _coinLabel;
    cocos2d::Label* _waterLabel;
    cocos2d::Label* _gemLabel;
    cocos2d::Label* _currentMessageLabel; // 新增：指向当前提示标签的指针

    // 存储已购买的商品
    std::vector<ShopItem> purchasedItems;

    // 商品列表
    std::vector<ShopItem> shopItems;
};

#endif // __SHOP_SCENE_H__