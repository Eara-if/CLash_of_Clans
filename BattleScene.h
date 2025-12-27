// BattleScene.h
#ifndef BATTLE_SCENE_H_
#define BATTLE_SCENE_H_
#include "MapTrap.h"
#include "cocos2d.h"
#include "EnemyBuilding.h"
#include "Soldier.h" // ��Ҫ���� Soldire ͷ�ļ�

// ����һ���򵥵Ľṹ��������ײ��ı���UI
struct SoldierUIItem {
    cocos2d::Sprite* icon;        // ͼ��
    cocos2d::Label* countLabel;   // ��������
    SoldierType type;             // ��������
    int count;                    // ʣ������
    std::string nameKey;          // ��Ӧ DataManager ���ַ���key
};

class BattleScene : public cocos2d::Scene
{
public:
    void onEnter();
    static cocos2d::Scene* createScene();
    virtual bool init();
    virtual void update(float dt) override;
    CREATE_FUNC(BattleScene);
    // 增加一个枚举或布尔值来区分模式
    enum class BattleMode {
        CAMPAIGN, // PVE 关卡
        PVP       // 攻打好友
    };

    // ��ʿ�� AI ʹ�õĹ����ӿ�
    cocos2d::Vector<EnemyBuilding*>& getTowers() { return _towers; }
    EnemyBuilding* getBase() { return _base; }
    static cocos2d::Scene* createScene(int levelIndex, std::string pvpJsonData = "");
    void setupBattle(int levelIndex, std::string pvpJsonData);

    // ��ײ���ӿ� (���ڷ��ü���δ�����ܵ�Ѱ·)
    bool isPositionBlocked(cocos2d::Vec2 worldPos);
    // ���������Զ����ʼ������
    bool initWithMap(const std::string& mapFileName);
    cocos2d::Vector<MapTrap*> _traps;
private:
    cocos2d::TMXTiledMap* _tileMap;
    std::string _mapFileName; // �����������ڴ洢Ҫ���صĵ�ͼ�ļ���
    void setMapFileName(const std::string& fileName); // �����������õ�ͼ�ļ���
    BattleMode _currentMode; // 当前模式
    // ��Ϸ�����б�
    cocos2d::Vector<EnemyBuilding*> _towers; // ���з�����
    EnemyBuilding* _base;                  // ��Ӫ
    cocos2d::Vector<Soldier*> _soldiers;    // ����ʿ��

    // �ϰ������� (��������ϵ Rect�����ڼ�����λ�úͻ��ƺ�ɫ����)
    std::vector<cocos2d::Rect> _forbiddenRects; // 

    // UI & �����߼�
    std::vector<SoldierUIItem*> _soldierUIList;
    bool _isPlacingMode;
    cocos2d::DrawNode* _forbiddenAreaNode; // ���ڻ��ƺ�ɫ������
    cocos2d::Label* _msgLabel;
    // ����������ǰѡ�еı�������
    SoldierType _currentSelectedType;
    SoldierUIItem* _currentSelectedItem; // ��ǰѡ�е�UI��ָ��
    // ���ü����ͼ�ʱ�����
    int _spawnCount;
    const int MAX_SPAWN = 10;
    cocos2d::Vec2 _currentTouchPos;
    bool _isTouchingMap;

    // ����������Ϸ״̬
    bool _isGameOver;
    bool _isGamePaused;

    void loadLevelCampaign(int levelIndex); // 加载原本的4关
    void loadLevelPVP(const std::string& json); // 加载新的JSON数据

    // UI & �����ص�
    void createUI();
    void BattleScene::onSoldierIconClicked(SoldierUIItem* item);
    void trySpawnSoldier(cocos2d::Vec2 worldPos);
    void showWarning(const std::string& msg);
    void spawnScheduler(float dt);

    // ����������Ϸ�����߼�
    void checkGameEnd();
    void showVictoryPopup();
    void showDefeatPopup();
    void hideVictoryPopup();
    // �����¼�
    bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event);
    void onTouchMoved(cocos2d::Touch* touch, cocos2d::Event* event);
    void onTouchEnded(cocos2d::Touch* touch, cocos2d::Event* event);

    // �����غ����� <--- C2039/C2065: menuBackToGameScene ������
    void menuBackToGameScene(cocos2d::Ref* pSender);
};

#endif // BATTLE_SCENE_H_