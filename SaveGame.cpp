#include "SaveGame.h"
#include "DataManager.h"
#include "GameScene.h"
#include "json/document.h"
#include "json/writer.h"
#include "json/stringbuffer.h"
#include "SharedData.h"
#include "Building.h"
#include <fstream>

USING_NS_CC;

// ����ȫ�ֱ���
extern int coin_count;
extern int water_count;
extern int gem_count;
extern int coin_limit;
extern int water_limit;
extern int gem_limit;
extern int army_limit;
extern cocos2d::Vector<Building*> g_allPurchasedBuildings;

SaveGame* SaveGame::_instance = nullptr;

SaveGame* SaveGame::getInstance()
{
    if (!_instance) {
        _instance = new SaveGame();
    }
    return _instance;
}

void SaveGame::fillJsonDocument(rapidjson::Document& document) {
    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
    document.SetObject();

    // 1. 资源
    document.AddMember("coin_count", coin_count, allocator);
    document.AddMember("water_count", water_count, allocator);
    document.AddMember("gem_count", gem_count, allocator);
    document.AddMember("coin_limit", coin_limit, allocator);
    document.AddMember("water_limit", water_limit, allocator);
    document.AddMember("gem_limit", gem_limit, allocator);

    // 2. 关卡进度
    DataManager* dm_level = DataManager::getInstance();
    document.AddMember("max_level_unlocked", dm_level->getMaxLevelUnlocked(), allocator);

    // 3. 建筑列表
    rapidjson::Value buildingsArray(rapidjson::kArrayType);
    for (auto building : g_allPurchasedBuildings) {
        if (!building) continue;
        rapidjson::Value bObj(rapidjson::kObjectType);
        bObj.AddMember("type", static_cast<int>(building->getType()), allocator);
        bObj.AddMember("level", building->getLevel(), allocator);
        bObj.AddMember("pos_x", building->getPositionX(), allocator);
        bObj.AddMember("pos_y", building->getPositionY(), allocator);
        bObj.AddMember("state", static_cast<int>(building->getState()), allocator);
        bObj.AddMember("name", rapidjson::Value(building->getName().c_str(), allocator).Move(), allocator);

        if (building->getState() == BuildingState::UPGRADING) {
            bObj.AddMember("upgrade_time_left", building->getRemainingTime(), allocator);
        }
        if (building->getType() == BuildingType::MINE || building->getType() == BuildingType::WATER) {
            bObj.AddMember("production_time_left", building->getProductionTimeLeft(), allocator);
        }
        buildingsArray.PushBack(bObj, allocator);
    }
    document.AddMember("buildings", buildingsArray, allocator);

    // 4. 军队
    rapidjson::Value armyArray(rapidjson::kArrayType);
    std::vector<std::string> troopTypes = { "Soldier", "Arrow", "Boom", "Giant", "Airforce" };
    for (const auto& troopType : troopTypes) {
        rapidjson::Value troopObj(rapidjson::kObjectType);
        troopObj.AddMember("type", rapidjson::Value(troopType.c_str(), allocator).Move(), allocator);
        troopObj.AddMember("count", DataManager::getInstance()->getTroopCount(troopType), allocator);
        armyArray.PushBack(troopObj, allocator);
    }
    document.AddMember("army", armyArray, allocator);

    // 5. 时间戳
    document.AddMember("save_timestamp", static_cast<int64_t>(time(0)), allocator);
}


bool SaveGame::saveGameState(const std::string& filename) {
    rapidjson::Document document;
    this->fillJsonDocument(document); // 使用通用逻辑

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);
    std::string jsonStr = buffer.GetString();

    std::string fullPath = FileUtils::getInstance()->getWritablePath() + filename;
    return FileUtils::getInstance()->writeStringToFile(jsonStr, fullPath);
}


bool SaveGame::loadGameState(const std::string& filename)
{
    CCLOG("=== SaveGame: Starting load process ===");

    std::string writablePath = FileUtils::getInstance()->getWritablePath();
    std::string fullPath = writablePath + filename;

    CCLOG("=== SaveGame: Looking for save file at: %s ===", fullPath.c_str());

    // ����ļ��Ƿ����
    if (!FileUtils::getInstance()->isFileExist(fullPath)) {
        CCLOG("=== SaveGame: ERROR: Save file not found: %s ===", fullPath.c_str());

        // ���������г���дĿ¼���ݽ��е���
        CCLOG("=== SaveGame: Listing files in writable path: %s ===", writablePath.c_str());
        // ע�⣺FileUtils��listFiles������Ҫ����ʵ��
        // std::vector<std::string> files = FileUtils::getInstance()->listFiles(writablePath);
        // for (const auto& file : files) {
        //     CCLOG("=== SaveGame: Found file: %s ===", file.c_str());
        // }

        return false;
    }

    CCLOG("=== SaveGame: Save file found, size: %d ===",
        (int)FileUtils::getInstance()->getFileSize(fullPath));

    // ��ȡ�ļ�����
    std::string content = FileUtils::getInstance()->getStringFromFile(fullPath);
    if (content.empty()) {
        CCLOG("=== SaveGame: ERROR: Save file is empty or cannot be read: %s ===", fullPath.c_str());
        return false;
    }

    CCLOG("=== SaveGame: File content loaded, size: %d bytes ===", (int)content.size());

    // ����JSON
    rapidjson::Document document;
    document.Parse(content.c_str());

    if (document.HasParseError()) {
        CCLOG("=== SaveGame: ERROR: JSON parse error: %s ===", document.GetParseError());
        CCLOG("=== SaveGame: ERROR: Error offset: %d ===", document.GetErrorOffset());

        // ����������ʾ�������ݣ�ǰ200�ַ���
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

    // �������ֶ�
    if (!document.HasMember("coin_count") || !document["coin_count"].IsInt()) {
        CCLOG("=== SaveGame: WARNING: Missing or invalid coin_count in save file ===");
    }

    // ������Դ����
    if (document.HasMember("coin_count")) coin_count = document["coin_count"].GetInt();
    if (document.HasMember("water_count")) water_count = document["water_count"].GetInt();
    if (document.HasMember("gem_count")) gem_count = document["gem_count"].GetInt();
    if (document.HasMember("coin_limit")) coin_limit = document["coin_limit"].GetInt();
    if (document.HasMember("water_limit")) water_limit = document["water_limit"].GetInt();
    if (document.HasMember("gem_limit")) gem_limit = document["gem_limit"].GetInt();



    CCLOG("=== SaveGame: Resources loaded - Coin:%d/%d, Water:%d/%d, Gem:%d/%d, ArmyLimit:%d ===",
        coin_count, coin_limit, water_count, water_limit, gem_count, gem_limit, army_limit);

    // �����������عؿ�����
    if (document.HasMember("max_level_unlocked")) {
        int savedMaxLevel = document["max_level_unlocked"].GetInt();
        DataManager::getInstance()->setMaxLevelUnlocked(savedMaxLevel);
        CCLOG("=== SaveGame: Level progress loaded - Max level unlocked: %d ===", savedMaxLevel);
    }
    else {
        CCLOG("=== SaveGame: WARNING: No level progress found in save file, using default ===");
    }

    // ����Ҫ�޸ġ�����������н������������¹���
 // �ȱ���ԭʼ�����б���Ժ��滻
    Vector<Building*> oldBuildings = g_allPurchasedBuildings;
    g_allPurchasedBuildings.clear();

    // ���ؽ�������
    if (document.HasMember("buildings") && document["buildings"].IsArray()) {
        const rapidjson::Value& buildingsArray = document["buildings"];
        CCLOG("=== SaveGame: Found %d buildings in save file ===", (int)buildingsArray.Size());

        for (rapidjson::SizeType i = 0; i < buildingsArray.Size(); i++) {
            const rapidjson::Value& buildingData = buildingsArray[i];

            // �������ֶ�
            if (!buildingData.HasMember("type") || !buildingData.HasMember("pos_x") ||
                !buildingData.HasMember("pos_y") || !buildingData.HasMember("level")) {
                CCLOG("=== SaveGame: WARNING: Building %d missing required fields ===", (int)i);
                continue;
            }

            // ��ȡ����������Ϣ
            BuildingType type = static_cast<BuildingType>(buildingData["type"].GetInt());
            int savedLevel = buildingData["level"].GetInt();
            float posX = buildingData["pos_x"].GetFloat();
            float posY = buildingData["pos_y"].GetFloat();
            BuildingState savedState = buildingData.HasMember("state") ?
                static_cast<BuildingState>(buildingData["state"].GetInt()) : BuildingState::IDLE;

            // ��ȡ����������ʣ��ʱ��
            float upgradeTimeLeft = 0;
            float productionTimeLeft = 0;

            if (buildingData.HasMember("upgrade_time_left")) {
                upgradeTimeLeft = buildingData["upgrade_time_left"].GetFloat();
            }

            if (buildingData.HasMember("production_time_left")) {
                productionTimeLeft = buildingData["production_time_left"].GetFloat();
            }

            CCLOG("=== SaveGame: Loading building %d - Type:%d, Level:%d, Pos:(%.1f,%.1f), State:%d ===",
                (int)i, (int)type, savedLevel, posX, posY, (int)savedState);

            // �������ʹ�������
            Building* newBuilding = nullptr;
            std::string filename;
            std::string buildingName;
            int baseCost = 300;

            // �ӱ���������л�ȡ�������ƣ�����У�
            if (buildingData.HasMember("name")) {
                buildingName = buildingData["name"].GetString();
            }

            // ���ݽ����������ò���
            switch (type) {
                case BuildingType::BASE:
                    filename = "House.png";
                    if (buildingName.empty()) buildingName = "My House";
                    baseCost = 500;
                    break;
                case BuildingType::BARRACKS:
                    filename = "junying.png";
                    if (buildingName.empty()) buildingName = "My junying";
                    baseCost = 200;
                    break;
                case BuildingType::MINE:
                    filename = "Mine.png";
                    if (buildingName.empty()) buildingName = "Gold Mine";
                    break;
                case BuildingType::WATER:
                    filename = "waterwell.png";
                    if (buildingName.empty()) buildingName = "Water Collector";
                    break;
                case BuildingType::DEFENSE:
                    filename = "TilesetTowers.png";
                    if (buildingName.empty()) buildingName = "Archer Tower";
                    break;
                case BuildingType::WALL:
                    filename = "fence.png";
                    if (buildingName.empty()) buildingName = "Wall";
                    baseCost = 0;
                    break;
                case BuildingType::GOLD_STORAGE:
                    filename = "BarGold.png";
                    if (buildingName.empty()) buildingName = "Gold Storage";
                    break;
                case BuildingType::WATER_STORAGE:
                    filename = "Water.png";
                    if (buildingName.empty()) buildingName = "Water Storage";
                    break;
                default:
                    CCLOG("=== SaveGame: WARNING: Unknown building type: %d ===", (int)type);
                    continue;
            }

            // ��������
            newBuilding = Building::create(filename, Rect::ZERO, buildingName, baseCost, type);
            if (newBuilding) {
                // ����λ��
                newBuilding->setPosition(posX, posY);
                newBuilding->setScale(0.5f);

                // �������޸ġ�ʹ���·���ֱ�ӴӴ浵���ݳ�ʼ����������������Դ��
                newBuilding->initFromSaveData(savedLevel, savedState, upgradeTimeLeft, productionTimeLeft);

                // ����Ҫ�����ûص�����
                // ע�⣺������Ҫ�������ûص���������Ϊ�´�����Buildingû�лص�
                newBuilding->setOnUpgradeCallback([=]() {
                    // ������߼���GameScene����ͬ
                    if (type == BuildingType::BASE) {
                        coin_limit += 1500;
                        water_limit += 1500;
                    }
                    else if (type == BuildingType::BARRACKS) {
                        army_limit += 10;
                        log("Army limit increased to %d", army_limit);
                    }

                    // ������Դ��ʾ
                    auto scene = Director::getInstance()->getRunningScene();
                    auto gameScene = dynamic_cast<GameScene*>(scene);
                    if (gameScene) {
                        gameScene->updateResourceDisplay();
                    }
                    });

                // ��ӵ�ȫ������
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

    // ���ؾ�������
    if (document.HasMember("army") && document["army"].IsArray()) {
        DataManager* dm = DataManager::getInstance();

        // ��������о���
        dm->clearArmy();

        const rapidjson::Value& armyArray = document["army"];
        CCLOG("=== SaveGame: Found %d troop types in save file ===", (int)armyArray.Size());

        for (rapidjson::SizeType i = 0; i < armyArray.Size(); i++) {
            const rapidjson::Value& troopData = armyArray[i];

            if (troopData.HasMember("type") && troopData.HasMember("count")) {
                std::string troopType = troopData["type"].GetString();
                int count = troopData["count"].GetInt();

                CCLOG("=== SaveGame: Loading troop %s x%d ===", troopType.c_str(), count);

                // ���ؼ��޸ġ�ֱ�������������������Դ
                dm->setTroopCount(troopType, count);
            }
        }

        // ��¼���˿�
        CCLOG("=== SaveGame: Total army population after load: %d ===", dm->getTotalPopulation());
    }
    else {
        CCLOG("=== SaveGame: No army data found in save file ===");
    }

    CCLOG("=== SaveGame: Game loaded successfully from: %s ===", fullPath.c_str());

    // ��ʾ���سɹ���ʾ
    auto scene = Director::getInstance()->getRunningScene();
    if (scene) {
        auto visibleSize = Director::getInstance()->getVisibleSize();
        auto gameScene = dynamic_cast<GameScene*>(scene);
        if (gameScene) {
            // 不仅刷新资源，还要刷新整图建筑！
            gameScene->reloadBuildingsFromSave();
        }

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

bool SaveGame::isSaveFileExist(const std::string& filename)
{
    std::string writablePath = FileUtils::getInstance()->getWritablePath();
    std::string fullPath = writablePath + filename;

    return FileUtils::getInstance()->isFileExist(fullPath);
}

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

void SaveGame::debugWritablePath()
{
    std::string writablePath = FileUtils::getInstance()->getWritablePath();
    CCLOG("=== SaveGame Debug: Writable Path ===");
    CCLOG("Path: %s", writablePath.c_str());

    // ���·���Ƿ����
    bool isDirExist = FileUtils::getInstance()->isDirectoryExist(writablePath);
    CCLOG("Directory exists: %s", isDirExist ? "YES" : "NO");

    // ���Դ��������ļ�
    std::string testPath = writablePath + "test_file.txt";
    std::ofstream testFile(testPath);
    if (testFile.is_open()) {
        testFile << "Test content";
        testFile.close();
        CCLOG("Test file created: %s", testPath.c_str());

        // ɾ�������ļ�
        std::remove(testPath.c_str());
        CCLOG("Test file deleted");
    }
    else {
        CCLOG("ERROR: Cannot create test file - check permissions!");
    }
}

void SaveGame::listSaveFiles()
{
    std::string writablePath = FileUtils::getInstance()->getWritablePath();
    CCLOG("=== SaveGame Debug: Listing files in %s ===", writablePath.c_str());

    // ע�⣺����ʵ��ȡ����FileUtils�İ汾
    // ��ĳЩ�汾�У�����ʹ�ã�
    // std::vector<std::string> files = FileUtils::getInstance()->listFiles(writablePath);
    // for (const auto& file : files) {
    //     CCLOG("File: %s", file.c_str());
    // }
}
bool SaveGame::loadFromRemoteString(const std::string& jsonData, bool isMyData)

{
    CCLOG("=== SaveGame: Starting remote load process ===");

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

    // --- 1. 恢复关卡进度 ---
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

    // --- 2. 恢复资源 (关键修改) ---
    // 只有是玩家自己的数据，才更新全局资源变量
    if (isMyData) {
        if (document.HasMember("coin_count")) coin_count = document["coin_count"].GetInt();
        if (document.HasMember("water_count")) water_count = document["water_count"].GetInt();
        if (document.HasMember("gem_count")) gem_count = document["gem_count"].GetInt();
        if (document.HasMember("coin_limit")) coin_limit = document["coin_limit"].GetInt();
        if (document.HasMember("water_limit")) water_limit = document["water_limit"].GetInt();
        if (document.HasMember("gem_limit")) gem_limit = document["gem_limit"].GetInt();
        CCLOG("=== SaveGame: Resources synced for Player ===");
    }
    else {
        // 如果是加载别人的数据，资源变量保持不变，或者你可以在 UI 上显示别人的资源（可选）
        CCLOG("=== SaveGame: Visiting mode - Skipping Resource Sync ===");
    }

    // --- 3. 彻底清理旧建筑 ---
        // 注意：如果是去攻击别人，我们清理的是“当前显示”的建筑，
        // 但千万不要让别人的建筑进入 g_allPurchasedBuildings。
    for (auto& building : g_allPurchasedBuildings) {
        if (building && building->getParent()) {
            building->removeFromParent();
        }
    }
    // 只有自己的数据才清空全局列表，如果是加载别人的数据，全局列表应该保留（暂存在内存中）
    if (isMyData) {
        g_allPurchasedBuildings.clear();
    }

    // --- 4. 恢复建筑 ---
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
                filename = "House.png"; // 或者你项目里的 "House.png"，请确保路径正确
            }
            else if (type == BuildingType::BARRACKS) filename = "junying.png";
            else if (type == BuildingType::MINE) filename = "Mine.png";
            else if (type == BuildingType::WATER) filename = "waterwell.png";
            else if (type == BuildingType::TOWER) filename = "TilesetTowers.png";
            else if (type == BuildingType::DEFENSE) filename = "TilesetTowers.png";
            else if (type == BuildingType::CANNON) filename = "Cannon.png"; // 补上加农炮
            else if (type == BuildingType::WALL) filename = "fence.png";
            else if (type == BuildingType::GOLD_STORAGE) filename = "BarGold.png";
            else if (type == BuildingType::WATER_STORAGE) filename = "Water.png";

            auto newBuilding = Building::create(filename, Rect::ZERO, bName, 300, type);
            if (newBuilding) {
                newBuilding->setPosition(posX, posY);
                newBuilding->setScale(0.5f);
                newBuilding->initFromSaveData(savedLevel, savedState, upgradeTimeLeft, productionTimeLeft);

                // 重新绑定回调
                newBuilding->setOnUpgradeCallback([=]() {
                    // 【注意】这里回调里的 limit 增加也需要判断 isMyData
                    // 但通常如果你在访问别人的基地，你应该禁用升级按钮的交互
                    if (isMyData) {
                        if (type == BuildingType::BASE) { coin_limit += 1500; water_limit += 1500; }
                        else if (type == BuildingType::BARRACKS) { army_limit += 10; }

                        auto scene = Director::getInstance()->getRunningScene();
                        auto gameScene = dynamic_cast<GameScene*>(scene);
                        if (gameScene) gameScene->updateResourceDisplay();
                    }
                    });
                if (isMyData) {
                    // 【关键点1】：如果是自己的数据，进入全局列表
                    g_allPurchasedBuildings.pushBack(newBuilding);
                    // 注意：这里不需要手动 addChild，因为 GameScene 会负责显示 g_allPurchasedBuildings
                }
                else {
                    auto scene = Director::getInstance()->getRunningScene();
                    if (scene) scene->addChild(newBuilding, 3);
                }
            }
        }
    }

    // --- 5. 恢复军队 ---
    // 只有是玩家自己的数据，才恢复到 DataManager 内存单例中
   // --- 5. 恢复军队 ---
    // 只有是玩家自己的数据，才恢复到 DataManager 内存单例中
    if (isMyData) {
        // 【修正】：无论服务器有没有数据，先清空本地内存中的旧数据！
        // 这样如果是新账号（没有 army 字段），本地数据就会正确变成 0。
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

// SaveGame.cpp
std::string SaveGame::getSpecificBuildingsJson(const cocos2d::Vector<Building*>& buildings) {
    rapidjson::Document doc;
    doc.SetObject();
    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

    rapidjson::Value buildingsArray(rapidjson::kArrayType);
    for (auto b : buildings) {
        rapidjson::Value bObj(rapidjson::kObjectType);
        bObj.AddMember("type", (int)b->getType(), allocator);
        bObj.AddMember("level", b->getLevel(), allocator);
        bObj.AddMember("pos_x", b->getPositionX(), allocator);
        bObj.AddMember("pos_y", b->getPositionY(), allocator);
        // ... 其他必要字段 ...
        buildingsArray.PushBack(bObj, allocator);
    }
    doc.AddMember("buildings", buildingsArray, allocator);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);
    return buffer.GetString();
}

std::string SaveGame::getGameStateAsJsonString() {
    rapidjson::Document document;
    // 1. 调用你原本提取的核心逻辑，填充建筑和资源数据
    this->fillJsonDocument(document);

    // 2. 获取分配器
    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

    // 3. 【新增逻辑】：将当前关卡进度存入 JSON 对象
    // 假设你的关卡变量名为 _currentLevel，如果没有该变量，请在 SaveGame.h 中定义
    int currentLevel = UserDefault::getInstance()->getIntegerForKey("CurrentLevel", 1);
    document.AddMember("currentLevel", currentLevel, allocator);

    // 4. 转换为字符串（保持你原本的逻辑不变）
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);

    return buffer.GetString();
}

// SaveGame.cpp
#include "network/HttpClient.h"
using namespace cocos2d::network;

void SaveGame::syncDataToCloud(std::function<void(bool)> callback) {
    // 1. 获取最新的 JSON 数据
    std::string currentData = this->getGameStateAsJsonString();

    // 2. 获取当前全局用户名
    extern std::string g_currentUsername;
    if (g_currentUsername.empty()) return;

    auto request = new (std::nothrow) HttpRequest();
    request->setUrl("http://100.80.248.229:5000/save");
    request->setRequestType(HttpRequest::Type::POST);
    request->setHeaders({ "Content-Type: application/json; charset=utf-8" });

    // 构造发送数据
    std::string postData = "{\"username\":\"" + g_currentUsername + "\", \"gameData\":" + currentData + "}";
    request->setRequestData(postData.c_str(), postData.length());

    request->setResponseCallback([=](HttpClient* client, HttpResponse* response) {
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

    HttpClient::getInstance()->send(request);
    request->release();
}