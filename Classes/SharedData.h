/**
 * @file       SharedData.h
 * @brief      全局共享数据定义头文件
 * @details    该文件声明了项目中通用的枚举类型（建筑类型、敌方建筑类型、建筑状态）与纯数据结构体（BuildingData），
 *             提供统一的类型判断标准（替代字符串判断，提升类型安全与执行效率），同时定义了建筑数据的标准化格式，
 *             用于 JSON 本地存储与服务器数据传输，不包含任何可视化组件（Sprite）或业务逻辑，保证数据独立性
 * @version    1.0
 * @note       该文件无第三方依赖，可直接引入项目使用；枚举类型支持扩展新类型，只需在对应枚举末尾添加（保持UNKNOWN为最后一项）；
 *             BuildingData 结构体仅保存纯数据，业务逻辑与可视化渲染需在其他模块实现
 */
#ifndef SHARED_DATA_H_
#define SHARED_DATA_H_

 /**
  * @enum       BuildingType
  * @brief      己方建筑类型枚举
  * @details    定义了所有己方建筑的类型标识，用于统一类型判断（替代字符串比较），提升代码可读性、类型安全性与执行效率，
  *             每个枚举值对应一种具体己方建筑，便于建筑创建、升级、状态管理等逻辑的统一处理
  */
enum class BuildingType {
    BASE = 0,              ///< 大本营（原 TOWN_HALL，核心建筑，决定游戏胜负）
    BARRACKS,              ///< 兵营（用于训练各类士兵单位）
    MINE,                  ///< 金矿（用于生产金币资源）
    WATER,                 ///< 圣水收集器（用于生产圣水资源）
    GOLD_STORAGE,          ///< 金库（用于存储金币资源，防止被敌方掠夺）
    WATER_STORAGE,         ///< 圣水库（用于存储圣水资源，防止被敌方掠夺）
    CANNON,                ///< 加农炮（己方防御建筑，攻击地面敌方单位）
    TOWER,                 ///< 弓箭塔（己方防御建筑，攻击地面/空中敌方单位）
    WALL,                  ///< 城墙（己方防御建筑，阻挡敌方单位移动，提升防御能力）
    UNKNOWN                ///< 未知建筑类型（用于容错判断，默认无效类型）
};

/**
 * @enum       EnemyType
 * @brief      敌方建筑类型枚举
 * @details    定义了所有敌方建筑的类型标识，与己方 BuildingType 一一对应，用于敌方建筑识别、士兵AI寻靶、
 *             伤害结算等逻辑，替代字符串判断，提升代码规范性与执行效率
 */
enum class EnemyType {
    BASE = 0,              ///< 敌方大本营（核心目标，摧毁后判定胜利）
    BARRACKS,              ///< 敌方兵营（敌方士兵训练建筑）
    MINE,                  ///< 敌方金矿（敌方金币生产建筑）
    WATER,                 ///< 敌方圣水收集器（敌方圣水生产建筑）
    GOLD_STORAGE,          ///< 敌方金库（敌方金币存储建筑，掠夺目标）
    WATER_STORAGE,         ///< 敌方圣水库（敌方圣水存储建筑，掠夺目标）
    CANNON,                ///< 敌方加农炮（敌方防御建筑，攻击己方地面单位）
    TOWER,                 ///< 敌方弓箭塔（敌方防御建筑，攻击己方地面/空中单位）
    WALL,                  ///< 敌方城墙（敌方防御建筑，阻挡己方单位移动）
    UNKNOWN                ///< 未知敌方建筑类型（用于容错判断，默认无效类型）
};

/**
 * @enum       BuildingState
 * @brief      建筑运行状态枚举
 * @details    定义了建筑的四种核心运行状态，用于建筑状态机管理，控制建筑的行为逻辑切换（如生产资源、升级建筑等），
 *             所有建筑均遵循该状态规范，便于统一的状态更新与UI展示
 */
enum class BuildingState {
    IDLE,                  ///< 闲置状态：建筑无任务执行（未升级、未生产资源）
    UPGRADING,             ///< 升级状态：建筑正在执行升级操作，不可进行其他任务
    PRODUCING,             ///< 生产状态：资源建筑正在生产金币/圣水，防御建筑无此状态
    READY                  ///< 就绪状态：资源生产完成待收取，或建筑升级完成待激活
};

/**
 * @struct     BuildingData
 * @brief      建筑纯数据结构体
 * @details    该结构体仅保存建筑的核心纯数据信息，不包含任何 Cocos2d-x 可视化组件（Sprite）或业务逻辑，
 *             是建筑数据的标准化格式，用于 JSON 本地持久化存储与服务器数据传输，保证数据的独立性与可移植性
 */
struct BuildingData {
    int id;                ///< 建筑唯一标识ID（可选字段，用于区分同一类型的多个建筑）
    BuildingType type;     ///< 建筑类型（己方建筑类型标识）
    int level;             ///< 建筑等级（决定建筑的生命值、生产效率、防御能力等核心属性）
    float x;               ///< 建筑在地图上的X坐标（世界坐标，用于建筑布局与渲染定位）
    float y;               ///< 建筑在地图上的Y坐标（世界坐标，用于建筑布局与渲染定位）

    /**
     * @brief      结构体构造函数，用于快速初始化建筑数据
     * @details    初始化建筑的类型、等级、地图坐标，默认将唯一ID设为0（可后续手动赋值），
     *             简化 BuildingData 实例的创建流程，提升代码编写效率
     * @param      t   建筑类型（BuildingType）
     * @param      l   建筑等级
     * @param      px  建筑地图X坐标
     * @param      py  建筑地图Y坐标
     */
    BuildingData(BuildingType t, int l, float px, float py)
        : type(t), level(l), x(px), y(py), id(0) {
    }
};

#endif // SHARED_DATA_H_