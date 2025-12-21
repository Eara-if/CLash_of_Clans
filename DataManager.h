#ifndef __DATA_MANAGER_H__
#define __DATA_MANAGER_H__

#include "cocos2d.h"
#include <map>
#include <string>

class DataManager
{
public:
    // ��ȡ��������
    static DataManager* getInstance();

    // --- �������ݹ��� ---

    // ����ʿ�� (���� true ��ʾ�ɹ���false ��ʾ�˿����˻���Դ����)
    bool trainTroop(std::string type);
    //����ʿ������
    bool dismissTroop(std::string type);
    // ��ȡĳ��ʿ��������
    int getTroopCount(std::string type);

    // ��ȡ���˿�
    int getTotalPopulation();

    // ��վ��� (ս�������������Ҫ)
    void clearArmy();
    void setMaxLevelUnlocked(int lv) { _maxLevelUnlocked = lv; }
    int getMaxLevelUnlocked() { return _maxLevelUnlocked; }

private:
    DataManager() {}; // ���캯��˽�л�

    // �洢�ṹ��<��������, ����>
    // ����: "Soldier" -> 5, "Archer" -> 3
    std::map<std::string, int> _myArmy;
    int _maxLevelUnlocked = 1; // 默认只能打第 1 关
};

#endif
