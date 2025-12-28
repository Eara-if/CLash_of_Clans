#ifndef RESOURCE_H_
#define RESOURCE_H_

#include "cocos2d.h"

USING_NS_CC;
extern int coin_count = 5000;
extern int water_count = 5000;
extern int gem_count = 500;

extern int coin_limit = 5000;
extern int water_limit = 5000;
extern int gem_limit = 5000;


class resource
{
public:
    resource() {};    //构造函数，传入
    virtual ~resource() {};
    std::string filename = "ui/06.png";  
    float start_x = 0;
    float start_y = 0;
    float tileWidth = 43;       //长宽一致，公共使用
    float tileHeight = 10;
    int count = 0;
    cocos2d::Sprite* displaySprite = nullptr;
    virtual void print(Node* parentNode) {};   //允许后面三种子类重写，打印资源栏图标
    virtual Rect calculateRect() { return Rect(start_x, start_y, tileWidth, tileHeight); };     //允许三种子类重写，裁剪图片的函数
};

class goldcoin : public resource
{
private:
   
    Rect calculateRect() {
        //设置初始位置和长宽
        start_y = 63;  
       //根据当前所有的资源数占最大资源数的比例设置不同的图片
        if (coin_count > 0 && coin_count <= coin_limit / 4) {
            start_x = 137;
        }
        else if (coin_count == 0) {
            start_x = 182;
        }
        else if (coin_count > coin_limit / 4 && coin_count <= coin_limit / 2) {
            start_x = 92;
        }
        else if (coin_count > coin_limit / 2 && coin_count <= coin_limit * 0.75) {
            start_x = 47;
        }
        else if (coin_count > coin_limit * 0.75 && coin_count <= coin_limit) {
            start_x = 2;
        }

        return Rect(start_x, start_y, tileWidth, tileHeight);
    }
public:

    goldcoin(){};
    ~goldcoin() {};

    void print(Node* parentNode) override 
    {
        //获取边界值
        auto visibleSize = Director::getInstance()->getVisibleSize();
        Vec2 origin = Director::getInstance()->getVisibleOrigin();
        //获取图片
        Rect tileRect = this->calculateRect();

        displaySprite = Sprite::create(filename, tileRect);

        if (displaySprite) {
            //设置图片位置等属性
            displaySprite->getTexture()->setAliasTexParameters();
            displaySprite->setScale(6.0f);
            displaySprite->setPosition(origin.x + visibleSize.width - 150, origin.y + visibleSize.height - 50);
            //将资源栏添加为子节点
            parentNode->addChild(displaySprite, 2);
        }
    }
    void refresh()
    {  //根据变化之后的资源数量刷新图片
        if (displaySprite != nullptr) {
            Rect newRect = this->calculateRect();
            displaySprite->setTextureRect(newRect);
        }
    }
};
class water : public resource
{
private:
    cocos2d::Sprite* displaySprite = nullptr;
    Rect calculateRect() {
        start_y = 19;

        if (water_count > 0 && water_count <= water_limit / 4) {
            start_x = 137;
        }
        else if (water_count == 0) {
            start_x = 182;
        }
        else if (water_count > water_limit / 4 && water_count <= water_limit / 2) {
            start_x = 92;
        }
        else if (water_count > water_limit / 2 && water_count <= water_limit * 0.75) {
            start_x = 47;
        }
        else if (water_count > water_limit * 0.75 && water_count <= water_limit) {
            start_x = 2;
        }

        return Rect(start_x, start_y, tileWidth, tileHeight);
    }
public:
    water(){};
    ~water() {};

    void print(Node* parentNode) override 
    {
        auto visibleSize = Director::getInstance()->getVisibleSize();
        Vec2 origin = Director::getInstance()->getVisibleOrigin();

        Rect tileRect = this->calculateRect();
        displaySprite = Sprite::create(filename, tileRect);

        if (displaySprite) {
            displaySprite->getTexture()->setAliasTexParameters();
             displaySprite->setScale(6.0f); 

            displaySprite->setPosition(origin.x + visibleSize.width - 150, origin.y + visibleSize.height - 120);
            parentNode->addChild(displaySprite, 2);
        }
    }
    void refresh() 
    {
        if (displaySprite != nullptr) 
        {
            Rect newRect = this->calculateRect();
            displaySprite->setTextureRect(newRect);

        }
    }

};
class Gem : public resource
{
private:
    Rect calculateRect() {
        start_y = 34;

        if (gem_count > 0 && gem_count <= gem_limit / 4) {
            start_x = 137;
        }
        else if (gem_count == 0) {
            start_x = 182;
        }
        else if (gem_count > gem_limit / 4 && gem_count <= gem_limit / 2) {
            start_x = 92;
        }
        else if (gem_count > gem_limit / 2 && gem_count <= gem_limit * 0.75) {
            start_x = 47;
        }
        else if (gem_count > gem_limit * 0.75 && gem_count <= gem_limit) {
            start_x = 2;
        }

        return Rect(start_x, start_y, tileWidth, tileHeight);
    }
public:
    Gem() {};
    ~Gem() {};

    void print(Node* parentNode) override 
    {

        auto visibleSize = Director::getInstance()->getVisibleSize();
        Vec2 origin = Director::getInstance()->getVisibleOrigin();

        Rect tileRect = this->calculateRect();

        displaySprite = Sprite::create(filename, tileRect);

        if (displaySprite) {
            displaySprite->getTexture()->setAliasTexParameters();
            displaySprite->setScale(6.0f); 

            displaySprite->setPosition(origin.x + visibleSize.width - 150, origin.y + visibleSize.height - 182);

            parentNode->addChild(displaySprite, 2);
        }
    }
    void refresh() {
        if (displaySprite != nullptr) {
            Rect newRect = this->calculateRect();

            displaySprite->setTextureRect(newRect);

        }
    }

    void update() {
    }
};

#endif