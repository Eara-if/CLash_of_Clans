#ifndef __SAVE_GAME_H__
#define __SAVE_GAME_H__

#include "cocos2d.h"
#include "Building.h"
#include "json/document.h"
#include "json/writer.h"
#include "json/stringbuffer.h"

class SaveGame
{
public:
    // ����ģʽ��ȡʵ��
    static SaveGame* getInstance();
    bool loadFromRemoteString(const std::string& jsonData);
    // 获取当前游戏状态的 JSON 字符串（用于上传服务器）
    std::string getGameStateAsJsonString();

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
private:
    SaveGame() {}
    // 核心逻辑：将当前所有数据填入 rapidjson 对象
    void fillJsonDocument(rapidjson::Document& document);
    static SaveGame* _instance;
};

#endif // __SAVE_GAME_H__