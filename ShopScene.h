#ifndef __SHOP_SCENE_H__
#define __SHOP_SCENE_H__

#include "cocos2d.h"

// ��Ʒ����ö��
enum class ShopItemType {
    WALL,           // ��ǽ
    GOLD_MINE,      // ���
    WATER_COLLECTOR,// ʥˮ�ռ���
    ARCHER_TOWER,   // ������
    CANNON,         // ��ũ��
    BARRACKS,       // ��Ӫ (����)
    GOLD_STORAGE,   // ��Ҵ洢�� (����)
    WATER_STORAGE   // ʥˮ�洢�� (����)
};

// ��Ʒ���ݽṹ
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
    void onEnter();
    // �����˵�
    void createMenu();

    // ��Ʒ����ص�
    void onWallPurchase(cocos2d::Ref* pSender);
    void onGoldMinePurchase(cocos2d::Ref* pSender);
    void onWaterCollectorPurchase(cocos2d::Ref* pSender);
    void onArcherTowerPurchase(cocos2d::Ref* pSender);
    void onCannonPurchase(cocos2d::Ref* pSender);
    void onBarracksPurchase(cocos2d::Ref* pSender);       // ����
    void onGoldStoragePurchase(cocos2d::Ref* pSender);    // ����
    void onWaterStoragePurchase(cocos2d::Ref* pSender);   // ����

    // ������Ʒ���ķ���
    bool purchaseItem(ShopItemType type);

    // ����Ƿ����㹻��Դ
    bool hasEnoughResources(int coinCost, int waterCost, int gemCost);

    // ��ʾ����ɹ�/ʧ����Ϣ
    void showPurchaseMessage(bool success, const std::string& itemName);

    // ������Դ��ʾ
    void updateResourceLabels();

    // ���ذ�ť�ص�
    void menuBackCallback(cocos2d::Ref* pSender);

    CREATE_FUNC(ShopScene);

private:
    cocos2d::Label* _coinLabel;
    cocos2d::Label* _waterLabel;
    cocos2d::Label* _gemLabel;
    cocos2d::Label* _currentMessageLabel; // ��������ǰ��ʾ����Ϣ��ǩ

    // �洢�ѹ������Ʒ
    std::vector<ShopItem> purchasedItems;

    // ��Ʒ�б�
    std::vector<ShopItem> shopItems;
};

#endif // __SHOP_SCENE_H__