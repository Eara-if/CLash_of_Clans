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
    // 【修改点】增加最后一个 bool 参数，并确保不是 static
    void createTroopCard(std::string name, std::string image, int index, cocos2d::Vec2 pos, bool isLocked);
    cocos2d::Label* showText(std::string content, float x, float y, cocos2d::Color4B color);
    // ˢ�½����ϵ�������ʾ
    void refreshLabels();

    // �������е�������ǩ������ˢ��
    std::vector<cocos2d::Label*> _countLabels;
};

#endif#pragma once
