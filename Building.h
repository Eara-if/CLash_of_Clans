#ifndef __BUILDING_H__
#define __BUILDING_H__

#include "cocos2d.h"
#include "SharedData.h"
#include <functional> // ���ڻص�


class Building : public cocos2d::Sprite
{
public:
    BuildingType _type;
    // ���޸ġ������� const cocos2d::Rect& rect ����
    static Building* create(const std::string& filename, const cocos2d::Rect& rect, const std::string& name, int baseCost, BuildingType type);
    float getTimeLeft();
    virtual bool init(const std::string& filename, const cocos2d::Rect& rect, const std::string& name, int baseCost, BuildingType type);
    void Building::setSpriteName(std::string name) {
        if (name == "House.png") {
            _type = BuildingType::BASE ;
        }
        else if (name == "juying.png") { // 那个拼音命名的兵营
            _type = BuildingType::BARRACKS;
        }
        else if (name == "cannon.png") {
            _type = BuildingType::CANNON;
        }
        // ...
    }
    BuildingType getType() const { return _type; }
    BuildingType getType() { return type; }
    // ��������ÿ֡���еĸ��º��� (���ڵ���ʱ)
    virtual void update(float dt) override;
    void setOnUpgradeCallback(std::function<void()> callback);

    BuildingState getState() {
        return state;
    }
    float getRemainingTime() {
        return timeLeft;
    }
    int getLevel() {
        return a_level;
    }
    int getSpeedUpCost(); // ���������Ҫ���ٱ�ʯ
    void startUpgrade();  // ��ʼ���� (�۽�ң���ʼ����ʱ)
    void speedUp();       // ��ʯ���� (�۱�ʯ��˲�����)
    int getNextLevelCost();
    void finishUpgrade();

    // ����������Դ��ط���
    void startProduction();      // ��ʼ��������ʱ
    void finishProduction();     // �������
    void collectResources();     // �ռ���Դ
    float getProductionTimeLeft(); // ��ȡ����ʣ��ʱ��
    int getProducedAmount();     // ��ȡ���ռ�����Դ��
    virtual void onEnter() override;

    void createGroundEffect();
    void removeGroundEffect();

    // ��������ֱ�����õȼ�����������Դ�����ڼ��ش浵��
    void setLevelDirectly(int level);

    // ��������ֱ������״̬
    void setStateDirectly(BuildingState newState);

    // ����������������ʣ��ʱ��
    void setUpgradeTimeLeft(float time);

    // ����������������ʣ��ʱ��
    void setProductionTimeLeft(float time);

    // ����������ȡ��������
    std::string getName() { return buildingName; }

    // �������������ݳ�ʼ�����������ڼ��ش浵��
    void initFromSaveData(int level, BuildingState savedState, float upgradeTimeLeft = 0, float productionTimeLeft = 0);
private:
    BuildingType type;
    bool isDragging;
    cocos2d::Vec2 touchOffset;
    std::string buildingName;
    int a_level;
    int baseCost1;

    BuildingState state;
    float totalTime; // �����ܺ�ʱ
    float timeLeft;  // ʣ��ʱ��

    std::function<void()> UpgradeCallback_coin;
    cocos2d::Node* groundEffectNode = nullptr;
    void initTouchListener();
    int getUpgradeTime(); // ��ȡ��һ��������Ҫ����

    // ������������Դ��ر���
    float productionTimeLeft; // ����ʣ��ʱ��
    int productionAmount;     // ���ռ�����Դ��
    bool isReadyToCollect;    // �Ƿ���ռ�
    cocos2d::Sprite* readyIndicator; // ���ռ���ʾ���
    cocos2d::Vec2 originalPos;
};




#endif