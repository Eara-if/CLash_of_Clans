#ifndef __GAME_SCENE_H__
#define __GAME_SCENE_H__

#include "cocos2d.h"

class GameScene : public cocos2d::Scene
{
private:
    cocos2d::Label* _coinTextLabel = nullptr;
    cocos2d::Label* _waterTextLabel = nullptr;
    cocos2d::Label* _gemTextLabel = nullptr;
    class goldcoin* myCoin = nullptr;
    class water* mywater = nullptr;
    class Gem* mygem = nullptr;
    cocos2d::TMXTiledMap* _tiledMap = nullptr;
public:
    
    // 创建场景的静态方法
    static cocos2d::Scene* createScene();
    void setBattleButton();
    // 初始化函数
    virtual bool init();
    void menuGotoBattleCallback(cocos2d::Ref* pSender); // 前往战斗场景
    void menuBackCallback(cocos2d::Ref* pSender);
    cocos2d::Label* GameScene::showText(std::string content, float x, float y, cocos2d::Color4B color);
    void setbuilding(const std::string& filename, const cocos2d::Rect& rect, const std::string& name, int level, int cost, cocos2d::Vec2 position);
    void setresource();
    // 这是一个宏，自动帮我们实现 create() 函数
    CREATE_FUNC(GameScene);
};

#endif // __GAME_SCENE_H__