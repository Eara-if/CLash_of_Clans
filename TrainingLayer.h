#ifndef __TRAINING_LAYER_H__
#define __TRAINING_LAYER_H__

#include "cocos2d.h"

class TrainingLayer : public cocos2d::Layer
{
public:
    virtual bool init();
    CREATE_FUNC(TrainingLayer);

    void closeLayer();

private:
    // 辅助函数：创建一个兵种的训练卡片
    void createTroopCard(std::string name, std::string image, int index, cocos2d::Vec2 startPos);
    cocos2d::Label* showText(std::string content, float x, float y, cocos2d::Color4B color);
    // 刷新界面上的数量显示
    void refreshLabels();

    // 保存所有的数量标签，方便刷新
    std::vector<cocos2d::Label*> _countLabels;
};

#endif#pragma once
