#include "SaveGame.h"
#include "DataManager.h"
#include "GameScene.h"
#include "json/document.h"
#include "json/writer.h"
#include "json/stringbuffer.h"
#include "SharedData.h"
#include "Building.h"
#include <fstream>
#include "network/HttpClient.h"
using namespace cocos2d::network;

USING_NS_CC;

extern int coin_count;
extern int water_count;
extern int gem_count;
extern int coin_limit;
extern int water_limit;
extern int gem_limit;
extern int army_limit;
//获取已购买的全建筑列表
extern cocos2d::Vector<Building*> g_allPurchasedBuildings;
SaveGame* SaveGame::_instance = nullptr;

//只维护一个存档管理器
SaveGame* SaveGame::getInstance()
{
    if (!_instance) {
        _instance = new SaveGame();
    }
    return _instance;
}
//将游戏数据填充到JSON对象中
void SaveGame::fillJsonDocument(rapidjson::Document& document)
{
    //获取分配器用于获取内存
    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
    //初始化当前JSON文档
    document.SetObject();

    // 存储一些资源
    document.AddMember("coin_count", coin_count, allocator);
    document.AddMember("water_count", water_count, allocator);
    document.AddMember("gem_count", gem_count, allocator);
    document.AddMember("coin_limit", coin_limit, allocator);
    document.AddMember("water_limit", water_limit, allocator);
    document.AddMember("gem_limit", gem_limit, allocator);

    // 存储关卡进度
    DataManager* dm_level = DataManager::getInstance();
    document.AddMember("max_level_unlocked", dm_level->getMaxLevelUnlocked(), allocator);

    //存储所有已经购买的建筑
    rapidjson::Value buildingsArray(rapidjson::kArrayType);
    for (auto building : g_allPurchasedBuildings) {
        if (!building) 
            continue;
        //创建单个对象的数据包 包括类型、等级、位置、状态
        rapidjson::Value bObj(rapidjson::kObjectType);
        //JSON 只识别int整数
        bObj.AddMember("type", static_cast<int>(building->getType()), allocator);
        bObj.AddMember("level", building->getLevel(), allocator);
        bObj.AddMember("pos_x", building->getPositionX(), allocator);
        bObj.AddMember("pos_y", building->getPositionY(), allocator);
        bObj.AddMember("state", static_cast<int>(building->getState()), allocator);
        bObj.AddMember("name", rapidjson::Value(building->getName().c_str(), allocator).Move(), allocator);

        //如果正在升级或者生产，存储一下剩余时间
        if (building->getState() == BuildingState::UPGRADING) {
            bObj.AddMember("upgrade_time_left", building->getRemainingTime(), allocator);
        }
        if (building->getType() == BuildingType::MINE || building->getType() == BuildingType::WATER) {
            bObj.AddMember("production_time_left", building->getProductionTimeLeft(), allocator);
        }
        //把打包好的数据放入数组
        buildingsArray.PushBack(bObj, allocator);
    }
    //把建筑列表也加进来
    document.AddMember("buildings", buildingsArray, allocator);

    // 存储军队信息 类型和数量
    rapidjson::Value armyArray(rapidjson::kArrayType);
    std::vector<std::string> troopTypes = { "Soldier", "Arrow", "Boom", "Giant", "Airforce" };
    for (const auto& troopType : troopTypes) {
        rapidjson::Value troopObj(rapidjson::kObjectType);
        troopObj.AddMember("type", rapidjson::Value(troopType.c_str(), allocator).Move(), allocator);
        troopObj.AddMember("count", DataManager::getInstance()->getTroopCount(troopType), allocator);
        armyArray.PushBack(troopObj, allocator);
    }
    document.AddMember("army", armyArray, allocator);

    // 存一下时间，防止存档出现错误
    document.AddMember("save_timestamp", static_cast<int64_t>(time(0)), allocator);
}

// 整理JSON文件
bool SaveGame::saveGameState(const std::string& filename) {
    rapidjson::Document document;
    //读入数据
    this->fillJsonDocument(document);
    //准备一个容器，接收转换后的字符
    rapidjson::StringBuffer buffer;
    //准备写入这个容器
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    //写
    document.Accept(writer);
    //最终拿到的结果
    std::string jsonStr = buffer.GetString();   

    //创建路径
    std::string fullPath = FileUtils::getInstance()->getWritablePath() + filename;
    //创建文件，写入
    return FileUtils::getInstance()->writeStringToFile(jsonStr, fullPath);
}

//加载游戏数据
bool SaveGame::loadGameState(const std::string& filename)
{
    CCLOG("=== SaveGame: Starting load process ===");
    
    //确定完整路径
    std::string writablePath = FileUtils::getInstance()->getWritablePath();
    std::string fullPath = writablePath + filename;

    CCLOG("=== SaveGame: Looking for save file at: %s ===", fullPath.c_str());

    // 检查这个读档文件是否存在
    if (!FileUtils::getInstance()->isFileExist(fullPath)) {
        
        return false;//加载失败
    }

    //getStringFromFile读取文件，将所有东西转换为一个string
    std::string content = FileUtils::getInstance()->getStringFromFile(fullPath);
    if (content.empty()) {
        
        return false;  //如果是空的加载失败
    }

    
    //将字符串解析出来
    rapidjson::Document document;
    document.Parse(content.c_str());

    //打印一些错误类型
    if (document.HasParseError()) {
        
        if (content.length() > 200) {
            CCLOG("=== SaveGame: ERROR: JSON preview (first 200 chars): %s ===",
                content.substr(0, 200).c_str());
        }
        else {
            CCLOG("=== SaveGame: ERROR: JSON content: %s ===", content.c_str());
        }

        return false;
    }

    CCLOG("=== SaveGame: JSON parsed successfully ===");

    if (!document.HasMember("coin_count") || !document["coin_count"].IsInt()) {
        CCLOG("=== SaveGame: WARNING: Missing or invalid coin_count in save file ===");
    }

    // 将资源填充回全局变量
    if (document.HasMember("coin_count")) 
        coin_count = document["coin_count"].GetInt();
    if (document.HasMember("water_count")) 
        water_count = document["water_count"].GetInt();
    if (document.HasMember("gem_count")) 
        gem_count = document["gem_count"].GetInt();
    if (document.HasMember("coin_limit")) 
        coin_limit = document["coin_limit"].GetInt();
    if (document.HasMember("water_limit")) 
        water_limit = document["water_limit"].GetInt();
    if (document.HasMember("gem_limit")) 
        gem_limit = document["gem_limit"].GetInt();



    CCLOG("=== SaveGame: Resources loaded - Coin:%d/%d, Water:%d/%d, Gem:%d/%d, ArmyLimit:%d ===",
        coin_count, coin_limit, water_count, water_limit, gem_count, gem_limit, army_limit);

    // 在数据管理器恢复一些关卡进度
    if (document.HasMember("max_level_unlocked")) {
        int savedMaxLevel = document["max_level_unlocked"].GetInt();
        DataManager::getInstance()->setMaxLevelUnlocked(savedMaxLevel);
        CCLOG("=== SaveGame: Level progress loaded - Max level unlocked: %d ===", savedMaxLevel);
    }
    else {
        CCLOG("=== SaveGame: WARNING: No level progress found in save file, using default ===");
    }

    
    Vector<Building*> oldBuildings = g_allPurchasedBuildings;
    //清空列表，准备装入新数据
    g_allPurchasedBuildings.clear();

    // 遍历存档中的建筑数组
    if (document.HasMember("buildings") && document["buildings"].IsArray()) {
        const rapidjson::Value& buildingsArray = document["buildings"];
        CCLOG("=== SaveGame: Found %d buildings in save file ===", (int)buildingsArray.Size());

        for (rapidjson::SizeType i = 0; i < buildingsArray.Size(); i++) {
            const rapidjson::Value& buildingData = buildingsArray[i];
            //检查这个建筑的相关信息是否完整
            if (!buildingData.HasMember("type") || !buildingData.HasMember("pos_x") ||
                !buildingData.HasMember("pos_y") || !buildingData.HasMember("level")) {
                CCLOG("=== SaveGame: WARNING: Building %d missing required fields ===", (int)i);
                continue;
            }

            //提取出一些基本数据
            BuildingType type = static_cast<BuildingType>(buildingData["type"].GetInt());
            int savedLevel = buildingData["level"].GetInt();
            float posX = buildingData["pos_x"].GetFloat();
            float posY = buildingData["pos_y"].GetFloat();
            BuildingState savedState = buildingData.HasMember("state") ?
                static_cast<BuildingState>(buildingData["state"].GetInt()) : BuildingState::IDLE;

            float upgradeTimeLeft = 0;
            float productionTimeLeft = 0;
            //如果有剩余时间，恢复
            if (buildingData.HasMember("upgrade_time_left")) {
                upgradeTimeLeft = buildingData["upgrade_time_left"].GetFloat();
            }

            if (buildingData.HasMember("production_time_left")) {
                productionTimeLeft = buildingData["production_time_left"].GetFloat();
            }

            CCLOG("=== SaveGame: Loading building %d - Type:%d, Level:%d, Pos:(%.1f,%.1f), State:%d ===",
                (int)i, (int)type, savedLevel, posX, posY, (int)savedState);

          //根据类型决定外观
            Building* newBuilding = nullptr;
            std::string filename;
            std::string buildingName;
            int baseCost = 300;
            //获取名字
            if (buildingData.HasMember("name")) {
                buildingName = buildingData["name"].GetString();
            }
            //起名字，给出图片路径
            switch (type) {
                case BuildingType::BASE:
                    filename = "House.png";
                    if (buildingName.empty()) 
                        buildingName = "My House";
                    baseCost = 500;
                    break;
                case BuildingType::BARRACKS:
                    filename = "junying.png";
                    if (buildingName.empty()) 
                        buildingName = "My junying";
                    baseCost = 200;
                    break;
                case BuildingType::MINE:
                    filename = "Mine.png";
                    if (buildingName.empty()) 
                        buildingName = "Gold Mine";
                    break;
                case BuildingType::WATER:
                    filename = "waterwell.png";
                    if (buildingName.empty())
                        buildingName = "Water Collector";
                    break;
                case BuildingType::TOWER:
                    filename = "TilesetTowers.png";
                    if (buildingName.empty())
                        buildingName = "Archer Tower";
                case BuildingType::CANNON:
                    filename = "Cannon.png";
                    if (buildingName.empty()) 
                        buildingName = "Cannon";
                    break;
                case BuildingType::WALL:
                    filename = "fence.png";
                    if (buildingName.empty()) 
                        buildingName = "Wall";
                    baseCost = 0;
                    break;
                case BuildingType::GOLD_STORAGE:
                    filename = "BarGold.png";
                    if (buildingName.empty()) 
                        buildingName = "Gold Storage";
                    break;
                case BuildingType::WATER_STORAGE:
                    filename = "Water.png";
                    if (buildingName.empty())
                        buildingName = "Water Storage";
                    break;
                default:
                    CCLOG("=== SaveGame: WARNING: Unknown building type: %d ===", (int)type);
                    continue;
            }

            //创建建筑对象
            newBuilding = Building::create(filename, Rect::ZERO, buildingName, baseCost, type);
            if (newBuilding) {
                newBuilding->setPosition(posX, posY);
                newBuilding->setScale(0.5f);
                //调用专用的恢复初始化函数
                newBuilding->initFromSaveData(savedLevel, savedState, upgradeTimeLeft, productionTimeLeft);
                // 函数逻辑是不能存进 JSON 的。
                // 虽然建筑恢复了，但“升级后增加上限”的功能丢失了。
                // 在这里手动重新写一遍 Lambda 表达式绑定进去。
                newBuilding->setOnUpgradeCallback([=]() {
                    if (type == BuildingType::BASE) {
                        coin_limit += 1500;
                        water_limit += 1500;
                    }
                    else if (type == BuildingType::BARRACKS) {
                        army_limit += 10;
                        log("Army limit increased to %d", army_limit);
                    }

                    //通知UI刷新
                    auto scene = Director::getInstance()->getRunningScene();
                    //强制转换类型
                    auto gameScene = dynamic_cast<GameScene*>(scene);
                    if (gameScene) {
                        gameScene->updateResourceDisplay();
                    }
                    });

                //双重保险管理建筑
                newBuilding->retain();
                g_allPurchasedBuildings.pushBack(newBuilding);
                newBuilding->release();

                CCLOG("=== SaveGame: Building %d created successfully (Level: %d, State: %d) ===",
                    (int)i, savedLevel, (int)savedState);
            }
            else {
                CCLOG("=== SaveGame: ERROR: Failed to create building %d ===", (int)i);
            }
        }
    }
    else {
        CCLOG("=== SaveGame: No buildings found in save file ===");
    }
    //恢复军队
    if (document.HasMember("army") && document["army"].IsArray()) {
        DataManager* dm = DataManager::getInstance();
        //先清空防止数据叠加
        dm->clearArmy();

        const rapidjson::Value& armyArray = document["army"];
        CCLOG("=== SaveGame: Found %d troop types in save file ===", (int)armyArray.Size());

        for (rapidjson::SizeType i = 0; i < armyArray.Size(); i++) {
            const rapidjson::Value& troopData = armyArray[i];

            if (troopData.HasMember("type") && troopData.HasMember("count")) {
                std::string troopType = troopData["type"].GetString();
                int count = troopData["count"].GetInt();

                CCLOG("=== SaveGame: Loading troop %s x%d ===", troopType.c_str(), count);
                //直接读取type和count，放到数据资源管理器中
                dm->setTroopCount(troopType, count);
            }
        }

        CCLOG("=== SaveGame: Total army population after load: %d ===", dm->getTotalPopulation());
    }
    else {
        CCLOG("=== SaveGame: No army data found in save file ===");
    }

    CCLOG("=== SaveGame: Game loaded successfully from: %s ===", fullPath.c_str());

    auto scene = Director::getInstance()->getRunningScene();
    if (scene) {
        auto visibleSize = Director::getInstance()->getVisibleSize();
        auto gameScene = dynamic_cast<GameScene*>(scene);
        if (gameScene) {
            //遍历 g_allPurchasedBuildings，把里面的 Sprite 添加到地图层上。
            // 不仅刷新资源，还要刷新整图建筑！
            gameScene->reloadBuildingsFromSave();
        }

        //给出一定的加载提示
        auto label = Label::createWithTTF("Game Loaded!", "fonts/Marker Felt.ttf", 32);
        label->setPosition(visibleSize.width / 2, visibleSize.height / 2);
        label->setColor(Color3B::GREEN);
        scene->addChild(label, 999);

        label->runAction(Sequence::create(
            DelayTime::create(3.0f),
            RemoveSelf::create(),
            nullptr
        ));
    }

    return true;
}
//检查存档文件内容是否存在
bool SaveGame::isSaveFileExist(const std::string& filename)
{
    std::string writablePath = FileUtils::getInstance()->getWritablePath();
    std::string fullPath = writablePath + filename;

    return FileUtils::getInstance()->isFileExist(fullPath);
}
//重新开始，删除原来的存档
bool SaveGame::deleteSaveFile(const std::string& filename)
{
    std::string writablePath = FileUtils::getInstance()->getWritablePath();
    std::string fullPath = writablePath + filename;

    if (remove(fullPath.c_str()) == 0) {
        CCLOG("Save file deleted: %s", fullPath.c_str());
        return true;
    }
    return false;
}
// 调试用，检查存储情况
void SaveGame::debugWritablePath()
{
    std::string writablePath = FileUtils::getInstance()->getWritablePath();
    CCLOG("=== SaveGame Debug: Writable Path ===");
    CCLOG("Path: %s", writablePath.c_str());

    bool isDirExist = FileUtils::getInstance()->isDirectoryExist(writablePath);
    CCLOG("Directory exists: %s", isDirExist ? "YES" : "NO");

    std::string testPath = writablePath + "test_file.txt";
    std::ofstream testFile(testPath);
    if (testFile.is_open()) {
        testFile << "Test content";
        testFile.close();
        CCLOG("Test file created: %s", testPath.c_str());

        std::remove(testPath.c_str());
        CCLOG("Test file deleted");
    }
    else {
        CCLOG("ERROR: Cannot create test file - check permissions!");
    }
}
//调试用打印文件位置
void SaveGame::listSaveFiles()
{
    std::string writablePath = FileUtils::getInstance()->getWritablePath();
    CCLOG("=== SaveGame Debug: Listing files in %s ===", writablePath.c_str());

}
//云端下载器，支持回自己家和去别人家
bool SaveGame::loadFromRemoteString(const std::string& jsonData, bool isMyData)

{
    CCLOG("=== SaveGame: Starting remote load process ===");
    //检查数据是不是空的
    if (jsonData.empty() || jsonData == "\"\"" || jsonData == "{}") {
        CCLOG("=== SaveGame: Remote data is empty, treating as new player. ===");
        return false;
    }

    rapidjson::Document document;
    document.Parse(jsonData.c_str());

    if (document.HasParseError()) {
        CCLOG("=== SaveGame: ERROR: Remote JSON parse error: %s ===", document.GetParseError());
        return false;
    }

        // 只有当 isMyData 为 true 时，才修改本地的 UserDefault 和 DataManager
    if (isMyData) {
        if (document.HasMember("currentLevel") && document["currentLevel"].IsInt()) {
            int levelFromServer = document["currentLevel"].GetInt();
            this->_currentLevel = levelFromServer;

            // 只有是自己的数据，才覆盖本地记录
            UserDefault::getInstance()->setIntegerForKey("CurrentLevel", levelFromServer);
            DataManager::getInstance()->setMaxLevelUnlocked(levelFromServer);
            UserDefault::getInstance()->flush();

            CCLOG("=== SaveGame: Syncing Level from Server: %d ===", levelFromServer);
        }

        // 恢复最大解锁关卡
        if (document.HasMember("max_level_unlocked")) {
            DataManager::getInstance()->setMaxLevelUnlocked(document["max_level_unlocked"].GetInt());
        }
    }
    else {
        CCLOG("=== SaveGame: Visiting mode - Skipping Level Sync ===");
    }

    // 只有是玩家自己的数据，才更新全局资源变量
    if (isMyData) {
        if (document.HasMember("coin_count")) 
            coin_count = document["coin_count"].GetInt();
        if (document.HasMember("water_count")) 
            water_count = document["water_count"].GetInt();
        if (document.HasMember("gem_count")) 
            gem_count = document["gem_count"].GetInt();
        if (document.HasMember("coin_limit")) 
            coin_limit = document["coin_limit"].GetInt();
        if (document.HasMember("water_limit")) 
            water_limit = document["water_limit"].GetInt();
        if (document.HasMember("gem_limit")) 
            gem_limit = document["gem_limit"].GetInt();
        CCLOG("=== SaveGame: Resources synced for Player ===");
    }
    else {
        // 如果是加载别人的数据，资源变量保持不变，或者你可以在 UI 上显示别人的资源（可选）
        CCLOG("=== SaveGame: Visiting mode - Skipping Resource Sync ===");
    }

    // 彻底清理旧建筑 
        // 如果是去攻击别人，清理的是“当前显示”的建筑，
        // 不让别人的建筑进入 g_allPurchasedBuildings。
    for (auto& building : g_allPurchasedBuildings) {
        if (building && building->getParent()) {
            building->removeFromParent();
        }
    }
    // 只有自己的数据才清空全局列表，如果是加载别人的数据，全局列表应该保留（暂存在内存中）
    if (isMyData) {
        g_allPurchasedBuildings.clear();
    }

    // 恢复建筑 
    if (document.HasMember("buildings") && document["buildings"].IsArray()) {
        const rapidjson::Value& buildingsArray = document["buildings"];
        // 获取当前正在运行的场景，以便非本人数据时可以将建筑挂载上去
        auto runningScene = Director::getInstance()->getRunningScene();
        for (rapidjson::SizeType i = 0; i < buildingsArray.Size(); i++) {
            const rapidjson::Value& bData = buildingsArray[i];

            BuildingType type = static_cast<BuildingType>(bData["type"].GetInt());
            int savedLevel = bData["level"].GetInt();
            float posX = bData["pos_x"].GetFloat();
            float posY = bData["pos_y"].GetFloat();
            BuildingState savedState = bData.HasMember("state") ?
                static_cast<BuildingState>(bData["state"].GetInt()) : BuildingState::IDLE;

            float upgradeTimeLeft = bData.HasMember("upgrade_time_left") ? bData["upgrade_time_left"].GetFloat() : 0;
            float productionTimeLeft = bData.HasMember("production_time_left") ? bData["production_time_left"].GetFloat() : 0;
            std::string bName = bData.HasMember("name") ? bData["name"].GetString() : "Building";

            // 图片选择逻辑
            std::string filename = "House.png";
            if (type == BuildingType::BASE) {
                filename = "House.png"; 
            }
            else if (type == BuildingType::BARRACKS) 
                filename = "junying.png";
            else if (type == BuildingType::MINE) 
                filename = "Mine.png";
            else if (type == BuildingType::WATER) 
                filename = "waterwell.png";
            else if (type == BuildingType::TOWER) 
                filename = "TilesetTowers.png";
            else if (type == BuildingType::CANNON) 
                filename = "Cannon.png"; 
            else if (type == BuildingType::WALL) 
                filename = "fence.png";
            else if (type == BuildingType::GOLD_STORAGE) 
                filename = "BarGold.png";
            else if (type == BuildingType::WATER_STORAGE) 
                filename = "Water.png";

            auto newBuilding = Building::create(filename, Rect::ZERO, bName, 300, type);
            if (newBuilding) {
                newBuilding->setPosition(posX, posY);
                newBuilding->setScale(0.5f);
                newBuilding->initFromSaveData(savedLevel, savedState, upgradeTimeLeft, productionTimeLeft);

                // 重新绑定回调
                newBuilding->setOnUpgradeCallback([=]() {
                    // 【这里回调里的 limit 增加也需要判断 isMyData
                    // 如果在访问别人的基地，禁用升级按钮的交互
                    if (isMyData) {
                        if (type == BuildingType::BASE) { coin_limit += 1500; water_limit += 1500; }
                        else if (type == BuildingType::BARRACKS) { army_limit += 10; }

                        auto scene = Director::getInstance()->getRunningScene();
                        auto gameScene = dynamic_cast<GameScene*>(scene);
                        if (gameScene) gameScene->updateResourceDisplay();
                    }
                    });
                if (isMyData) {
                    // 如果是自己的数据，进入全局列表
                    g_allPurchasedBuildings.pushBack(newBuilding);
                }
                else {
                    auto scene = Director::getInstance()->getRunningScene();
                    if (scene) scene->addChild(newBuilding, 3);
                }
            }
        }
    }

    // 只有是玩家自己的数据，才恢复到 DataManager 内存单例中
    if (isMyData) {
        // 先清空本地内存中的旧数据
        DataManager::getInstance()->clearArmy();

        if (document.HasMember("army") && document["army"].IsArray()) {
            const rapidjson::Value& armyArray = document["army"];
            for (rapidjson::SizeType i = 0; i < armyArray.Size(); i++) {
                DataManager::getInstance()->setTroopCount(
                    armyArray[i]["type"].GetString(),
                    armyArray[i]["count"].GetInt()
                );
            }
        }
        CCLOG("=== SaveGame: Army synced for Player (Cleared and Reloaded) ===");

    }

    CCLOG("=== SaveGame: Remote game state loaded successfully! ===");
    return true;
}


// 获取当前游戏状态的 JSON 字符串（用于上传服务器）
std::string SaveGame::getGameStateAsJsonString() 
{
    rapidjson::Document document;
    // 调用你原本提取的核心逻辑，填充建筑和资源数据
    this->fillJsonDocument(document);

    // 获取分配器
    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

    // 将当前关卡进度存入 JSON 对象
    // 关卡变量名为 _currentLevel
    int currentLevel = UserDefault::getInstance()->getIntegerForKey("CurrentLevel", 1);
    document.AddMember("currentLevel", currentLevel, allocator);

    // 转换为字符串
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);

    return buffer.GetString();
}
//将存档上传
void SaveGame::syncDataToCloud(std::function<void(bool)> callback) 
{
    // 获取最新的 JSON 数据
    std::string currentData = this->getGameStateAsJsonString();

    // 获取当前全局用户名
    extern std::string g_currentUsername;
    if (g_currentUsername.empty()) 
        return;

    auto request = new (std::nothrow) HttpRequest();
    request->setUrl("http://100.80.248.229:5000/save");
    request->setRequestType(HttpRequest::Type::POST);
    request->setHeaders({ "Content-Type: application/json; charset=utf-8" });

    // 构造发送数据
    std::string postData = "{\"username\":\"" + g_currentUsername + "\", \"gameData\":" + currentData + "}";
    request->setRequestData(postData.c_str(), postData.length());

    request->setResponseCallback([=](HttpClient* client, HttpResponse* response) {
        //是否上传成功； 
        bool success = (response && response->isSucceed());
        if (success) {
            log("Cloud Save: OK");
        }
        else {
            log("Cloud Save: FAILED");
        }

        // 如果外部（比如 GameScene）传入了处理 UI 的回调，就执行它
        if (callback) {
            callback(success);
        }
        });
    //发出去
    HttpClient::getInstance()->send(request);
    request->release();
}