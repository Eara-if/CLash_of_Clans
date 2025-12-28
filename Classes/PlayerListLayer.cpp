#include "PlayerListLayer.h"
#include "json/document.h"
#include "json/writer.h"
#include "json/stringbuffer.h"

USING_NS_CC;
using namespace cocos2d::ui;
using namespace cocos2d::network; // 使用网络命名空间

// 引用全局变量获取当前用户名，用于在列表中排除自己或标记自己
extern std::string g_currentUsername;

//好友界面的初始化函数
bool PlayerListLayer::init()
{
    if (!Layer::init()) 
        return false;

    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 也开启一个黑色半透明遮罩
    auto bgLayer = LayerColor::create(Color4B(0, 0, 0, 150));
    this->addChild(bgLayer);

    // 也拦截点击
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);  //不将点击事件传给下面
    listener->onTouchBegan = [=](Touch* t, Event* e) {
        //把屏幕上的触摸点，转换成内部的坐标系
        Vec2 pos = sidebarNode->convertToNodeSpace(t->getLocation());
        //获取自己的大小
        Rect rect(0, 0, sidebarNode->getContentSize().width, sidebarNode->getContentSize().height);
        //如果不在这个侧边栏内点击，就关闭界面
        if (!rect.containsPoint(pos)) {
            this->hide();  //关闭这个函数
        }
        return true;   //表示接收了这个点击
        };
    //设置好友界面最先接收到一切点击，因为好友在最上面
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);  

    //拦截滚轮，不要在这个界面还可以对主界面进行放大缩小
    auto mouseListener = EventListenerMouse::create();
    mouseListener->onMouseScroll = [=](Event* event) {
        //强制转换，获取鼠标动作
        EventMouse* e = (EventMouse*)event;  
        // 获取滚轮方向：+1(向上推) 或 -1(向下滚)
        float scrollY = e->getScrollY(); 

        // 只有当列表存在
        if (listView) {
            auto inner = listView->getInnerContainer();
            float contentHeight = inner->getContentSize().height;
            float viewHeight = listView->getContentSize().height;
          //且内容高度超过显示区域时，才执行滚动
            if (contentHeight > viewHeight) {
                // 获取当前内部容器的位置 Y
                float currentY = inner->getPositionY();

                // 计算滚动速度
                // 滚轮向下滚 ，内容应该向下跑
                // 滚轮向上推 ，内容应该向上跑
                float step = 20.0f;
                float newY = currentY + (scrollY * step);

                // 边界限制 (防止滚出屏幕外)
                // Cocos ScrollView 的内部容器 Y 坐标范围通常是 [viewHeight - contentHeight, 0]
                float minY = viewHeight - contentHeight;
                float maxY = 0.0f;

                // 修正坐标，确保不越界
                if (newY < minY) 
                    newY = minY;
                if (newY > maxY) 
                    newY = maxY;
                // 应用新位置
                inner->setPositionY(newY);
            }
        }

        // 停止滚轮事件传播，防止底下的地图缩放
        event->stopPropagation();
        };
    //同样把侧边栏放到地图前方
    _eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);

    // 设置侧边栏背景
    float sidebarWidth = 400.0f;
    sidebarNode = LayerColor::create(Color4B(40, 44, 52, 255));
    sidebarNode->setContentSize(Size(sidebarWidth, visibleSize.height));
    sidebarNode->setAnchorPoint(Vec2(0, 0));
    sidebarNode->setPosition(Vec2(visibleSize.width, 0));
    this->addChild(sidebarNode);

    // 设置标题
    auto title = Label::createWithTTF("FRIENDS", "fonts/Marker Felt.ttf", 40);
    title->setPosition(Vec2(sidebarWidth / 2, visibleSize.height - 60));
    title->setTextColor(Color4B::YELLOW);
    sidebarNode->addChild(title);

    // 关闭按钮
    auto closeBtn = ui::Button::create();
    closeBtn->setTitleText("X");
    closeBtn->setTitleFontSize(40);
    closeBtn->setTitleColor(Color3B::RED);
    closeBtn->setPosition(Vec2(sidebarWidth - 40, visibleSize.height - 40));
    closeBtn->addClickEventListener([=](Ref*) { 
        this->hide(); 
        });
    sidebarNode->addChild(closeBtn);

    //创造一个可以滚动的容器
    listView = ListView::create();  
    //设置整体大小，然后居中放置
    listView->setContentSize(Size(sidebarWidth - 40, visibleSize.height - 120));  
    listView->setPosition(Vec2(20, 20));
    //设置为垂直滚动
    listView->setDirection(ui::ScrollView::Direction::VERTICAL);
    //拉到底部有一个弹动的效果
    listView->setBounceEnabled(true);
    //保持每个用户之间的距离
    listView->setItemsMargin(10);
    sidebarNode->addChild(listView);

    return true;
}
//保存用户数据，等待使用
void PlayerListLayer::setOnVisitCallback(VisitCallback callback) 
{
    onVisitCallback = callback;
}
//让侧边栏出现
void PlayerListLayer::show()
{
    //计算坐标，从而确定显示位置
    auto visibleSize = Director::getInstance()->getVisibleSize();
    float width = sidebarNode->getContentSize().width;

    //把侧边栏移动到位
    sidebarNode->runAction(EaseBackOut::create(
        MoveTo::create(0.3f, Vec2(visibleSize.width - width, 0))
    ));

    // 每次打开时，重新从服务器拉取数据更新
    this->loadData();
}
//把侧边栏收起来
void PlayerListLayer::hide()
{
    //规划撤退路线
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto slideOut = EaseBackIn::create(
        MoveTo::create(0.3f, Vec2(visibleSize.width, 0))  //0.3秒内移动到边缘之外
    );
    //把侧边栏彻底移除
    //CallFUnc把代码变成一个动作
    auto finishCallback = CallFunc::create([this]() {
        this->removeFromParent();
        }); 
    //整合成一个好的动画效果
    sidebarNode->runAction(Sequence::create(slideOut, finishCallback, nullptr));
}

void PlayerListLayer::loadData()
{
    //清空列表
    listView->removeAllItems();

    // 显示一个“加载中...”的提示，网络请求需要一定时间
    auto loadingLabel = Text::create("Loading...", "fonts/Marker Felt.ttf", 24);
    loadingLabel->setName("loading_label");
    loadingLabel->setPosition(Vec2(sidebarNode->getContentSize().width / 2, sidebarNode->getContentSize().height / 2));
    sidebarNode->addChild(loadingLabel);

    // 发起 HTTP GET 请求
    auto request = new HttpRequest();
    // 目标地址
    request->setUrl("http://100.80.248.229:5000/users"); // 你的 Python 服务器地址
    //GET是获取，POST是写
    request->setRequestType(HttpRequest::Type::GET);
    //当数据到了，执行onUserListRequestCompleted函数的内容
    request->setResponseCallback(CC_CALLBACK_2(PlayerListLayer::onUserListRequestCompleted, this));
    //把请求发给服务器
    HttpClient::getInstance()->send(request);
    //自动管理
    request->release();
}

void PlayerListLayer::onUserListRequestCompleted(HttpClient* client, HttpResponse* response)
{
    // 移除“加载中”提示
    sidebarNode->removeChildByName("loading_label");
    //调试
    if (!response || !response->isSucceed()) {
        log("Network Error in PlayerListLayer");
        return;
    }
    //拿到服务器数据
    std::vector<char>* buffer = response->getResponseData();
    //将01转换为字符串
    std::string resultStr(buffer->begin(), buffer->end());

    // 解析 JSON，翻译成程序可以执行操作的结构体
    rapidjson::Document doc;
    doc.Parse(resultStr.c_str());
    //检查JSON
    if (doc.HasParseError()) {
        log("JSON Parse Error");
        return;
    }

    // 服务器返回格式: { "users": ["Alice", "Bob", "Myself"] }
    if (doc.HasMember("users") && doc["users"].IsArray()) {
        auto users = doc["users"].GetArray();

        // 先添加一个自己家的按钮,点击不会切换场景
        auto myHomeItem = this->createPlayerItem(g_currentUsername, 9999, -1);
        //放到整体用户列表里面展示
        listView->pushBackCustomItem(myHomeItem);

        // 遍历服务器返回的用户
        for (int i = 0; i < users.Size(); i++) {
            std::string name = users[i].GetString();

            
            if (name == g_currentUsername)  //这里跳过自己家
                continue;
            auto item = this->createPlayerItem(name, 0, i);
            //放到整体用户列表里面展示
            listView->pushBackCustomItem(item);
        }
    }
}

Widget* PlayerListLayer::createPlayerItem(const std::string& name, int score, int index)
{
    // 设置每个人的背景底板
    auto layout = Layout::create();
    layout->setContentSize(Size(360, 80));
    layout->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
    layout->setBackGroundColor(Color3B(60, 60, 70));

    // 设置每个人的名字
    auto nameLabel = Text::create(name, "fonts/Marker Felt.ttf", 24);
    nameLabel->setAnchorPoint(Vec2(0, 0.5f));
    nameLabel->setPosition(Vec2(20, 55));
    layout->addChild(nameLabel);


    //设置攻击其他人的按钮
    auto btn = Button::create();
    btn->setTitleFontSize(20);
    btn->setScale9Enabled(true); //按钮自适应格式
    btn->setContentSize(Size(100, 40));

    // 如果是“自己”，按钮显示 HOME，否则显示 ATTACK
    std::string btnText = (name == g_currentUsername) ? "HOME" : "ATTACK";
    btn->setTitleText(btnText);
    btn->setTitleColor(Color3B::WHITE);
    btn->setPosition(Vec2(300, 40));
    //创建点击事件，点击按钮跳转到他人营地
    btn->addClickEventListener([=](Ref*) {
        log("Selected user: %s", name.c_str());
        // 调用回调函数，通知 GameScene 加载这个人的数据
        if (onVisitCallback) {
            onVisitCallback(name);
        }
        // 关闭列表
        this->hide();
        });
    //按钮挂在底板上
    layout->addChild(btn);

    return layout;
}