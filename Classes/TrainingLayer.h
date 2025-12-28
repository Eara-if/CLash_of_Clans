#ifndef TRAINING_LAYER_H_
#define TRAINING_LAYER_H_

#include "cocos2d.h"

class TrainingLayer : public cocos2d::Layer
{
public:
    virtual bool init();
    CREATE_FUNC(TrainingLayer);
    void closeLayer();

private:
    void createTroopCard(std::string name, std::string image, int index, cocos2d::Vec2 pos, bool isLocked);
    cocos2d::Label* showText(std::string content, float x, float y, cocos2d::Color4B color);
    void refreshLabels();
    //存储兵种名字
    std::vector<cocos2d::Label*> countLabels;
};

#endif
