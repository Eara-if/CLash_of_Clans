#ifndef __GAME_SCENE_H__
#define __GAME_SCENE_H__

#include "cocos2d.h"

class GameScene : public cocos2d::Scene
{
public:
    cocos2d::Label* _coinTextLabel = nullptr;
    cocos2d::Label* _waterTextLabel = nullptr;
    cocos2d::Label* _gemTextLabel = nullptr;
    // 创建场景的静态方法
    static cocos2d::Scene* createScene();

    // 初始化函数
    virtual bool init();
    void menuBackCallback(cocos2d::Ref* pSender);
    cocos2d::Label* GameScene::showText(std::string content, float x, float y, cocos2d::Color4B color);
    void setdby();
    void setresource();
    // 这是一个宏，自动帮我们实现 create() 函数
    CREATE_FUNC(GameScene);
};

#endif // __GAME_SCENE_H__