#include "SaveGame.h"
#include "DataManager.h"
#include "GameScene.h"
#include "json/document.h"
#include "json/writer.h"
#include "json/stringbuffer.h"
#include <fstream>

USING_NS_CC;

// 声明全局变量
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

bool SaveGame::saveGameState(const std::string& filename)
{
    CCLOG("=== SaveGame: Starting save process ===");

    // 使用 rapidjson 创建文档
    rapidjson::Document document;
    document.SetObject();
    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

    // 保存资源数据
    document.AddMember("coin_count", coin_count, allocator);
    document.AddMember("water_count", water_count, allocator);
    document.AddMember("gem_count", gem_count, allocator);
    document.AddMember("coin_limit", coin_limit, allocator);
    document.AddMember("water_limit", water_limit, allocator);
    document.AddMember("gem_limit", gem_limit, allocator);


    CCLOG("=== SaveGame: Resources saved ===");
    CCLOG("=== SaveGame: Global buildings count: %d ===", (int)g_allPurchasedBuildings.size());

    DataManager* dm_level = DataManager::getInstance();
    int maxLevelUnlocked = dm_level->getMaxLevelUnlocked();
    document.AddMember("max_level_unlocked", maxLevelUnlocked, allocator);
    CCLOG("=== SaveGame: Level progress saved - Max level unlocked: %d ===", maxLevelUnlocked);

    // 【重要修改】保存所有建筑，不管是否有父节点
    Vector<Building*> tempBuildings;
    for (auto& building : g_allPurchasedBuildings) {
        if (building) {
            tempBuildings.pushBack(building);
            CCLOG("=== SaveGame: Found building for save - Type: %d, Name: %s, Pos: (%.1f, %.1f) ===",
                (int)building->getType(), building->getName().c_str(),
                building->getPositionX(), building->getPositionY());
        }
    }

    CCLOG("=== SaveGame: Total buildings to save: %d ===", (int)tempBuildings.size());

    // 保存建筑数据
    rapidjson::Value buildingsArray(rapidjson::kArrayType);

    for (auto& building : tempBuildings) {
        if (building) {
            rapidjson::Value buildingObj(rapidjson::kObjectType);

            // 保存建筑基本信息
            buildingObj.AddMember("type", static_cast<int>(building->getType()), allocator);
            buildingObj.AddMember("level", building->getLevel(), allocator);

            // 保存位置
            buildingObj.AddMember("pos_x", building->getPositionX(), allocator);
            buildingObj.AddMember("pos_y", building->getPositionY(), allocator);
            buildingObj.AddMember("state", static_cast<int>(building->getState()), allocator);

            // 【新增】保存建筑名称（用于调试）
            buildingObj.AddMember("name",
                rapidjson::Value(building->getName().c_str(), allocator).Move(),
                allocator);

            // 保存升级剩余时间
            if (building->getState() == BuildingState::UPGRADING) {
                buildingObj.AddMember("upgrade_time_left", building->getRemainingTime(), allocator);
                CCLOG("=== SaveGame: Building UPGRADING, time left: %f ===", building->getRemainingTime());
            }

            // 保存生产状态
            if (building->getType() == BuildingType::MINE || building->getType() == BuildingType::WATER) {
                buildingObj.AddMember("production_time_left", building->getProductionTimeLeft(), allocator);
                CCLOG("=== SaveGame: Production building, time left: %f ===", building->getProductionTimeLeft());
            }

            buildingsArray.PushBack(buildingObj, allocator);

            CCLOG("=== SaveGame: Saved building - Type:%d, Name:%s, Level:%d, Pos:(%.1f,%.1f) ===",
                (int)building->getType(), building->getName().c_str(),
                building->getLevel(), building->getPositionX(), building->getPositionY());
        }
    }

    document.AddMember("buildings", buildingsArray, allocator);

    // 保存军队数据
    rapidjson::Value armyArray(rapidjson::kArrayType);
    DataManager* dm = DataManager::getInstance();

    // 获取所有兵种类型
    std::vector<std::string> troopTypes = { "Soldier", "Arrow", "Boom", "Giant", "Airforce" };

    for (const auto& troopType : troopTypes) {
        int count = dm->getTroopCount(troopType);
        rapidjson::Value troopObj(rapidjson::kObjectType);
        troopObj.AddMember("type", rapidjson::Value(troopType.c_str(), allocator).Move(), allocator);
        troopObj.AddMember("count", count, allocator);
        armyArray.PushBack(troopObj, allocator);

        CCLOG("=== SaveGame: Saving troop %s x%d ===", troopType.c_str(), count);
    }

    document.AddMember("army", armyArray, allocator);

    // 添加保存时间戳
    time_t now = time(0);
    document.AddMember("save_timestamp", static_cast<int64_t>(now), allocator);

    // 转换为JSON字符串
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);

    std::string jsonStr = buffer.GetString();

    // 【核心修复】使用Cocos2d-x的FileUtils，它会处理路径编码问题
    std::string writablePath = FileUtils::getInstance()->getWritablePath();
    std::string fullPath = writablePath + filename;

    CCLOG("=== SaveGame: Writable path: %s ===", writablePath.c_str());
    CCLOG("=== SaveGame: Full path: %s ===", fullPath.c_str());
    CCLOG("=== SaveGame: JSON size: %d bytes ===", (int)jsonStr.length());

    // 使用FileUtils写入文件
    if (FileUtils::getInstance()->writeStringToFile(jsonStr, fullPath)) {
        CCLOG("=== SaveGame: Game saved successfully to: %s ===", fullPath.c_str());

        // 显示保存成功的提示
        auto scene = Director::getInstance()->getRunningScene();
        if (scene) {
            auto visibleSize = Director::getInstance()->getVisibleSize();
            auto label = Label::createWithTTF("Game Saved!", "fonts/Marker Felt.ttf", 32);
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
    else {
        CCLOG("=== SaveGame: ERROR: Failed to save game to: %s ===", fullPath.c_str());

        // 显示错误提示
        auto scene = Director::getInstance()->getRunningScene();
        if (scene) {
            auto visibleSize = Director::getInstance()->getVisibleSize();
            auto label = Label::createWithTTF("Save Failed!", "fonts/Marker Felt.ttf", 32);
            label->setPosition(visibleSize.width / 2, visibleSize.height / 2);
            label->setColor(Color3B::RED);
            scene->addChild(label, 999);

            label->runAction(Sequence::create(
                DelayTime::create(3.0f),
                RemoveSelf::create(),
                nullptr
            ));
        }

        return false;
    }
}

bool SaveGame::loadGameState(const std::string& filename)
{
    CCLOG("=== SaveGame: Starting load process ===");

    std::string writablePath = FileUtils::getInstance()->getWritablePath();
    std::string fullPath = writablePath + filename;

    CCLOG("=== SaveGame: Looking for save file at: %s ===", fullPath.c_str());

    // 检查文件是否存在
    if (!FileUtils::getInstance()->isFileExist(fullPath)) {
        CCLOG("=== SaveGame: ERROR: Save file not found: %s ===", fullPath.c_str());

        // 【新增】列出可写目录内容进行调试
        CCLOG("=== SaveGame: Listing files in writable path: %s ===", writablePath.c_str());
        // 注意：FileUtils的listFiles可能需要具体实现
        // std::vector<std::string> files = FileUtils::getInstance()->listFiles(writablePath);
        // for (const auto& file : files) {
        //     CCLOG("=== SaveGame: Found file: %s ===", file.c_str());
        // }

        return false;
    }

    CCLOG("=== SaveGame: Save file found, size: %d ===",
        (int)FileUtils::getInstance()->getFileSize(fullPath));

    // 读取文件内容
    std::string content = FileUtils::getInstance()->getStringFromFile(fullPath);
    if (content.empty()) {
        CCLOG("=== SaveGame: ERROR: Save file is empty or cannot be read: %s ===", fullPath.c_str());
        return false;
    }

    CCLOG("=== SaveGame: File content loaded, size: %d bytes ===", (int)content.size());

    // 解析JSON
    rapidjson::Document document;
    document.Parse(content.c_str());

    if (document.HasParseError()) {
        CCLOG("=== SaveGame: ERROR: JSON parse error: %s ===", document.GetParseError());
        CCLOG("=== SaveGame: ERROR: Error offset: %d ===", document.GetErrorOffset());

        // 【新增】显示错误内容（前200字符）
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

    // 检查必需字段
    if (!document.HasMember("coin_count") || !document["coin_count"].IsInt()) {
        CCLOG("=== SaveGame: WARNING: Missing or invalid coin_count in save file ===");
    }

    // 加载资源数据
    if (document.HasMember("coin_count")) coin_count = document["coin_count"].GetInt();
    if (document.HasMember("water_count")) water_count = document["water_count"].GetInt();
    if (document.HasMember("gem_count")) gem_count = document["gem_count"].GetInt();
    if (document.HasMember("coin_limit")) coin_limit = document["coin_limit"].GetInt();
    if (document.HasMember("water_limit")) water_limit = document["water_limit"].GetInt();
    if (document.HasMember("gem_limit")) gem_limit = document["gem_limit"].GetInt();



    CCLOG("=== SaveGame: Resources loaded - Coin:%d/%d, Water:%d/%d, Gem:%d/%d, ArmyLimit:%d ===",
        coin_count, coin_limit, water_count, water_limit, gem_count, gem_limit, army_limit);

    // 【新增】加载关卡进度
    if (document.HasMember("max_level_unlocked")) {
        int savedMaxLevel = document["max_level_unlocked"].GetInt();
        DataManager::getInstance()->setMaxLevelUnlocked(savedMaxLevel);
        CCLOG("=== SaveGame: Level progress loaded - Max level unlocked: %d ===", savedMaxLevel);
    }
    else {
        CCLOG("=== SaveGame: WARNING: No level progress found in save file, using default ===");
    }

    // 【重要修改】不再清空现有建筑，而是重新构建
 // 先保存原始建筑列表，稍后替换
    Vector<Building*> oldBuildings = g_allPurchasedBuildings;
    g_allPurchasedBuildings.clear();

    // 加载建筑数据
    if (document.HasMember("buildings") && document["buildings"].IsArray()) {
        const rapidjson::Value& buildingsArray = document["buildings"];
        CCLOG("=== SaveGame: Found %d buildings in save file ===", (int)buildingsArray.Size());

        for (rapidjson::SizeType i = 0; i < buildingsArray.Size(); i++) {
            const rapidjson::Value& buildingData = buildingsArray[i];

            // 检查必需字段
            if (!buildingData.HasMember("type") || !buildingData.HasMember("pos_x") ||
                !buildingData.HasMember("pos_y") || !buildingData.HasMember("level")) {
                CCLOG("=== SaveGame: WARNING: Building %d missing required fields ===", (int)i);
                continue;
            }

            // 获取建筑基本信息
            BuildingType type = static_cast<BuildingType>(buildingData["type"].GetInt());
            int savedLevel = buildingData["level"].GetInt();
            float posX = buildingData["pos_x"].GetFloat();
            float posY = buildingData["pos_y"].GetFloat();
            BuildingState savedState = buildingData.HasMember("state") ?
                static_cast<BuildingState>(buildingData["state"].GetInt()) : BuildingState::IDLE;

            // 获取升级和生产剩余时间
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

            // 根据类型创建建筑
            Building* newBuilding = nullptr;
            std::string filename;
            std::string buildingName;
            int baseCost = 300;

            // 从保存的数据中获取建筑名称（如果有）
            if (buildingData.HasMember("name")) {
                buildingName = buildingData["name"].GetString();
            }

            // 根据建筑类型设置参数
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

            // 创建建筑
            newBuilding = Building::create(filename, Rect::ZERO, buildingName, baseCost, type);
            if (newBuilding) {
                // 设置位置
                newBuilding->setPosition(posX, posY);
                newBuilding->setScale(0.5f);

                // 【核心修改】使用新方法直接从存档数据初始化建筑，不消耗资源！
                newBuilding->initFromSaveData(savedLevel, savedState, upgradeTimeLeft, productionTimeLeft);

                // 【重要】设置回调函数
                // 注意：这里需要重新设置回调函数，因为新创建的Building没有回调
                newBuilding->setOnUpgradeCallback([=]() {
                    // 这里的逻辑与GameScene中相同
                    if (type == BuildingType::BASE) {
                        coin_limit += 1500;
                        water_limit += 1500;
                    }
                    else if (type == BuildingType::BARRACKS) {
                        army_limit += 10;
                        log("Army limit increased to %d", army_limit);
                    }

                    // 更新资源显示
                    auto scene = Director::getInstance()->getRunningScene();
                    auto gameScene = dynamic_cast<GameScene*>(scene);
                    if (gameScene) {
                        gameScene->updateResourceDisplay();
                    }
                    });

                // 添加到全局容器
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

    // 加载军队数据
    if (document.HasMember("army") && document["army"].IsArray()) {
        DataManager* dm = DataManager::getInstance();

        // 先清空现有军队
        dm->clearArmy();

        const rapidjson::Value& armyArray = document["army"];
        CCLOG("=== SaveGame: Found %d troop types in save file ===", (int)armyArray.Size());

        for (rapidjson::SizeType i = 0; i < armyArray.Size(); i++) {
            const rapidjson::Value& troopData = armyArray[i];

            if (troopData.HasMember("type") && troopData.HasMember("count")) {
                std::string troopType = troopData["type"].GetString();
                int count = troopData["count"].GetInt();

                CCLOG("=== SaveGame: Loading troop %s x%d ===", troopType.c_str(), count);

                // 【关键修改】直接设置数量，不检查资源
                dm->setTroopCount(troopType, count);
            }
        }

        // 记录总人口
        CCLOG("=== SaveGame: Total army population after load: %d ===", dm->getTotalPopulation());
    }
    else {
        CCLOG("=== SaveGame: No army data found in save file ===");
    }

    CCLOG("=== SaveGame: Game loaded successfully from: %s ===", fullPath.c_str());

    // 显示加载成功提示
    auto scene = Director::getInstance()->getRunningScene();
    if (scene) {
        auto visibleSize = Director::getInstance()->getVisibleSize();
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

    // 检查路径是否存在
    bool isDirExist = FileUtils::getInstance()->isDirectoryExist(writablePath);
    CCLOG("Directory exists: %s", isDirExist ? "YES" : "NO");

    // 尝试创建测试文件
    std::string testPath = writablePath + "test_file.txt";
    std::ofstream testFile(testPath);
    if (testFile.is_open()) {
        testFile << "Test content";
        testFile.close();
        CCLOG("Test file created: %s", testPath.c_str());

        // 删除测试文件
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

    // 注意：具体实现取决于FileUtils的版本
    // 在某些版本中，可以使用：
    // std::vector<std::string> files = FileUtils::getInstance()->listFiles(writablePath);
    // for (const auto& file : files) {
    //     CCLOG("File: %s", file.c_str());
    // }
}