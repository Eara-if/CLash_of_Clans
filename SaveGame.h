#ifndef __SAVE_GAME_H__
#define __SAVE_GAME_H__

#include "cocos2d.h"
#include "Building.h"

class SaveGame
{
public:
    // 单例模式获取实例
    static SaveGame* getInstance();

    // 保存游戏状态
    bool saveGameState(const std::string& filename = "game_save.json");

    // 加载游戏状态
    bool loadGameState(const std::string& filename = "game_save.json");

    // 检查存档是否存在
    bool isSaveFileExist(const std::string& filename = "game_save.json");

    // 删除存档
    bool deleteSaveFile(const std::string& filename = "game_save.json");
    // 调试函数：打印可写路径信息
    void debugWritablePath();

    // 调试函数：列出保存目录中的文件
    void listSaveFiles();
private:
    SaveGame() {}
    static SaveGame* _instance;
};

#endif // __SAVE_GAME_H__