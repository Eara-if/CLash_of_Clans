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
    std::string filename = "";
    int count = 0;

    // ????????? 1??????????????????????????????????
    // ??????????????????????????????????? Node*
    virtual void print(Node* parentNode) {}; // ?�D????????????��
    virtual void modify() {};
    resource(int data) :count(data) {};
    virtual ~resource() {}; // ?????��??????????????????�D??
};

class goldcoin : public resource
{
private:
    cocos2d::Sprite* _displaySprite = nullptr;
    Rect calculateRect() {
        float start_x = 0;
        float start_y = 63;
        float tileWidth = 43;
        float tileHeight = 10;

        // ?????????????
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

    goldcoin() : resource(5000) {};
    ~goldcoin() {};

    void modify() override { // ?????��???? override ???????????????????
        // ??????
    }

    // ?????????????????????????��????????????? Node*
    void print(Node* parentNode) override {

        // ?????? 2???????????????????
        auto visibleSize = Director::getInstance()->getVisibleSize();
        Vec2 origin = Director::getInstance()->getVisibleOrigin();

        filename = "ui/06.png";


        Rect tileRect = this->calculateRect();

        _displaySprite = Sprite::create(filename, tileRect);

        // ??????
        if (_displaySprite) {
            _displaySprite->getTexture()->setAliasTexParameters();
            _displaySprite->setScale(6.0f); // ??? scaleAmount ???????????

            // ????��??
            _displaySprite->setPosition(origin.x + visibleSize.width - 150, origin.y + visibleSize.height - 50);

            // ??????????
            parentNode->addChild(_displaySprite, 2);
        }
    }
    void refresh() {
        if (_displaySprite != nullptr) {
            // 1. ??????? Rect (??? coin_count ???????)
            Rect newRect = this->calculateRect();

            // 2. ???????????????????????????????????
            _displaySprite->setTextureRect(newRect);

            // log("Coin icon refreshed!"); 
        }
    }
    void update() {
    }
};
class water : public resource
{
private:
    cocos2d::Sprite* _displaySprite = nullptr;
    Rect calculateRect() {
        float start_x = 0;
        float start_y = 19;
        float tileWidth = 43;
        float tileHeight = 10;

        // ?????????????
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
    water() : resource(5000) {};
    ~water() {};

    void modify() override { // ?????��???? override ???????????????????
        // ??????
    }

    void print(Node* parentNode) override {

        // ?????? 2???????????????????
        auto visibleSize = Director::getInstance()->getVisibleSize();
        Vec2 origin = Director::getInstance()->getVisibleOrigin();

        filename = "ui/06.png";


        Rect tileRect = this->calculateRect();

        _displaySprite = Sprite::create(filename, tileRect);

        // ??????
        if (_displaySprite) {
            _displaySprite->getTexture()->setAliasTexParameters();
            _displaySprite->setScale(6.0f); // ??? scaleAmount ???????????

            // ????��??
            _displaySprite->setPosition(origin.x + visibleSize.width - 150, origin.y + visibleSize.height - 120);

            // ??????????
            parentNode->addChild(_displaySprite, 2);
        }
    }
    void refresh() {
        if (_displaySprite != nullptr) {
            // 1. ??????? Rect (??? coin_count ???????)
            Rect newRect = this->calculateRect();

            // 2. ???????????????????????????????????
            _displaySprite->setTextureRect(newRect);

            // log("Coin icon refreshed!"); 
        }
    }

    void update() {
    }
};
class Gem : public resource
{
private:
    cocos2d::Sprite* _displaySprite = nullptr;
    Rect calculateRect() {
        float start_x = 0;
        float start_y = 34;
        float tileWidth = 43;
        float tileHeight = 10;

        // ?????????????
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
    Gem() : resource(5000) {};
    ~Gem() {};

    void modify() override {
    }


    void print(Node* parentNode) override {

        // ?????? 2???????????????????
        auto visibleSize = Director::getInstance()->getVisibleSize();
        Vec2 origin = Director::getInstance()->getVisibleOrigin();

        filename = "ui/06.png";


        Rect tileRect = this->calculateRect();

        _displaySprite = Sprite::create(filename, tileRect);

        // ??????
        if (_displaySprite) {
            _displaySprite->getTexture()->setAliasTexParameters();
            _displaySprite->setScale(6.0f); // ??? scaleAmount ???????????

            // ????��??
            _displaySprite->setPosition(origin.x + visibleSize.width - 150, origin.y + visibleSize.height - 182);

            // ??????????
            parentNode->addChild(_displaySprite, 2);
        }
    }
    void refresh() {
        if (_displaySprite != nullptr) {
            // 1. ??????? Rect (??? coin_count ???????)
            Rect newRect = this->calculateRect();

            // 2. ???????????????????????????????????
            _displaySprite->setTextureRect(newRect);

            // log("Coin icon refreshed!"); 
        }
    }

    void update() {
    }
};

#endif