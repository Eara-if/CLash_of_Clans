#ifndef SAVE_GAME_H_
#define SAVE_GAME_H_

#include "cocos2d.h"
#include "Building.h"
#include "json/document.h"
#include "json/writer.h"
#include "json/stringbuffer.h"
#include "network/HttpClient.h"

class SaveGame
{
public:
    static SaveGame* getInstance();
    std::string SaveGame::getSpecificBuildingsJson(const cocos2d::Vector<Building*>& buildings);
    
    std::string getGameStateAsJsonString();
    // 修改声明：增加 bool isMyData，默认为 true（为了兼容旧代码，不传参默认当成自己）
    bool loadFromRemoteString(const std::string& jsonData, bool isMyData = true);

    // ������Ϸ״̬
    bool saveGameState(const std::string& filename = "game_save.json");

    // ������Ϸ״̬
    bool loadGameState(const std::string& filename = "game_save.json");

    // ���浵�Ƿ����
    bool isSaveFileExist(const std::string& filename = "game_save.json");

    // ɾ���浵
    bool deleteSaveFile(const std::string& filename = "game_save.json");
    // ���Ժ�������ӡ��д·����Ϣ
    void debugWritablePath();

    // ���Ժ������г�����Ŀ¼�е��ļ�
    void listSaveFiles();
    void syncDataToCloud(std::function<void(bool)> callback = nullptr);
    int _currentLevel;
private:
    SaveGame() {}
    // 核心逻辑：将当前所有数据填入 rapidjson 对象
    void fillJsonDocument(rapidjson::Document& document);
    static SaveGame* _instance;
};

#endif // SAVE_GAME_H_