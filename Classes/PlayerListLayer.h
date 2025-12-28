#ifndef PLAYER_LIST_LAYER_H_
#define PLAYER_LIST_LAYER_H_

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "network/HttpClient.h" // 1. 引入网络库

class PlayerListLayer : public cocos2d::Layer
{
public:
    virtual bool init();

    void show();
    void hide();

    // 定义一个回调函数,当点击列表中的某个玩家时调用
    // 参数是 string (目标玩家的名字)
    typedef std::function<void(std::string)> VisitCallback;

    // 设置回调函数的方法
    void setOnVisitCallback(VisitCallback callback);

    CREATE_FUNC(PlayerListLayer);

private:
    cocos2d::Node* sidebarNode;
    cocos2d::ui::ListView* listView;
    VisitCallback onVisitCallback; // 保存回调函数

    cocos2d::ui::Widget* createPlayerItem(const std::string& name, int score, int index);

    // 3. 将加载数据改为网络请求
    void loadData();
    void onUserListRequestCompleted(cocos2d::network::HttpClient* client, cocos2d::network::HttpResponse* response);
};

#endif
