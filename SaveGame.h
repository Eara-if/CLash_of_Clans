#ifndef __SAVE_GAME_H__
#define __SAVE_GAME_H__

#include "cocos2d.h"
#include "Building.h"

class SaveGame
{
public:
    // ����ģʽ��ȡʵ��
    static SaveGame* getInstance();

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
    static SaveGame* _instance;
};

#endif // __SAVE_GAME_H__