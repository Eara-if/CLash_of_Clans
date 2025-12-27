#pragma once

// 1. 统一建筑类型（不要再用字符串判断了，用这个枚举）
enum class BuildingType {
    BASE = 0,       // 大本营 (原 TOWN_HALL)
    BARRACKS,       // 兵营
    MINE,           // 金矿
    WATER,          // 圣水收集器
    DEFENSE,        // 防御建筑
    GOLD_STORAGE,   // 金库
    WATER_STORAGE,  // 圣水库
    CANNON,         // 加农炮
    TOWER,          // 弓箭塔
    WALL,           // 城墙
    UNKNOWN
};
enum class EnemyType {
    BASE = 0,       // 大本营 (原 TOWN_HALL)
    BARRACKS,       // 兵营
    MINE,           // 金矿
    WATER,          // 圣水收集器
    DEFENSE,        // 防御建筑
    GOLD_STORAGE,   // 金库
    WATER_STORAGE,  // 圣水库
    CANNON,         // 加农炮
    TOWER,          // 弓箭塔
    WALL,           // 城墙
    UNKNOWN
};
enum class BuildingState {
    IDLE,       // ����
    UPGRADING,   // ��������
    PRODUCING,  // ����������Դ (����)
    READY       // ��Դ���ռ� (����)
};

// 2. 定义一个纯数据结构，不包含任何 Sprite 或逻辑
// 这个结构体就是我们存进 JSON，以及发给服务器的数据格式
struct BuildingData {
    int id;                 // 唯一ID (可选)
    BuildingType type;      // 类型
    int level;              // 等级 (决定血量)
    float x;                // 地图上的 X 坐标
    float y;                // 地图上的 Y 坐标

    // 构造函数方便使用
    BuildingData(BuildingType t, int l, float px, float py)
        : type(t), level(l), x(px), y(py), id(0) {
    }
};
