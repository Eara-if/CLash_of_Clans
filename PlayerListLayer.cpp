#include "PlayerListLayer.h"
#include "json/document.h"
#include "json/writer.h"
#include "json/stringbuffer.h"

USING_NS_CC;
using namespace cocos2d::ui;
using namespace cocos2d::network; // 使用网络命名空间

// 引用全局变量获取当前用户名，用于在列表中排除自己或标记自己
extern std::string g_currentUsername;

bool PlayerListLayer::init()
{
    if (!Layer::init()) return false;

    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 1. 半透明遮罩
    auto bgLayer = LayerColor::create(Color4B(0, 0, 0, 150));
    this->addChild(bgLayer);

    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [=](Touch* t, Event* e) {
        Vec2 pos = _sidebarNode->convertToNodeSpace(t->getLocation());
        Rect rect(0, 0, _sidebarNode->getContentSize().width, _sidebarNode->getContentSize().height);
        if (!rect.containsPoint(pos)) {
            this->hide();
        }
        return true;
        };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    // 2. 侧边栏背景
    float sidebarWidth = 400.0f;
    _sidebarNode = LayerColor::create(Color4B(40, 44, 52, 255));
    _sidebarNode->setContentSize(Size(sidebarWidth, visibleSize.height));
    _sidebarNode->setAnchorPoint(Vec2(0, 0));
    _sidebarNode->setPosition(Vec2(visibleSize.width, 0));
    this->addChild(_sidebarNode);

    // 3. 标题
    auto title = Label::createWithTTF("NEIGHBORS", "fonts/Marker Felt.ttf", 40);
    title->setPosition(Vec2(sidebarWidth / 2, visibleSize.height - 60));
    title->setTextColor(Color4B::YELLOW);
    _sidebarNode->addChild(title);

    // 关闭按钮
    auto closeBtn = ui::Button::create("ui/close_btn.png"); // 假设你有图片，没有就用文字
    if (closeBtn->getContentSize().width <= 0) {
        closeBtn = ui::Button::create();
        closeBtn->setTitleText("X");
        closeBtn->setTitleFontSize(40);
        closeBtn->setTitleColor(Color3B::RED);
    }
    closeBtn->setPosition(Vec2(sidebarWidth - 40, visibleSize.height - 40));
    closeBtn->addClickEventListener([=](Ref*) { this->hide(); });
    _sidebarNode->addChild(closeBtn);

    // 4. ListView
    _listView = ListView::create();
    _listView->setContentSize(Size(sidebarWidth - 40, visibleSize.height - 120));
    _listView->setPosition(Vec2(20, 20));
    _listView->setDirection(ui::ScrollView::Direction::VERTICAL);
    _listView->setBounceEnabled(true);
    _listView->setItemsMargin(10);
    _sidebarNode->addChild(_listView);

    return true;
}

void PlayerListLayer::setOnVisitCallback(VisitCallback callback) {
    _onVisitCallback = callback;
}

void PlayerListLayer::show()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    float width = _sidebarNode->getContentSize().width;

    _sidebarNode->runAction(EaseBackOut::create(
        MoveTo::create(0.3f, Vec2(visibleSize.width - width, 0))
    ));

    // 每次打开时，重新从服务器拉取数据
    this->loadData();
}

void PlayerListLayer::hide()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto slideOut = EaseBackIn::create(MoveTo::create(0.3f, Vec2(visibleSize.width, 0)));
    auto finishCallback = CallFunc::create([this]() {
        this->removeFromParent();
        });
    _sidebarNode->runAction(Sequence::create(slideOut, finishCallback, nullptr));
}

// =============================================================
//  网络请求核心逻辑
// =============================================================

void PlayerListLayer::loadData()
{
    _listView->removeAllItems();

    // 显示一个“加载中...”的提示
    auto loadingLabel = Text::create("Loading...", "fonts/Marker Felt.ttf", 24);
    loadingLabel->setName("loading_label");
    loadingLabel->setPosition(Vec2(_sidebarNode->getContentSize().width / 2, _sidebarNode->getContentSize().height / 2));
    _sidebarNode->addChild(loadingLabel);

    // 发起 HTTP GET 请求
    auto request = new HttpRequest();
    request->setUrl("http://100.80.248.229:5000/users"); // 你的 Python 服务器地址
    request->setRequestType(HttpRequest::Type::GET);
    request->setResponseCallback(CC_CALLBACK_2(PlayerListLayer::onUserListRequestCompleted, this));
    HttpClient::getInstance()->send(request);
    request->release();
}

void PlayerListLayer::onUserListRequestCompleted(HttpClient* client, HttpResponse* response)
{
    // 移除“加载中”提示
    _sidebarNode->removeChildByName("loading_label");

    if (!response || !response->isSucceed()) {
        log("Network Error in PlayerListLayer");
        return;
    }

    std::vector<char>* buffer = response->getResponseData();
    std::string resultStr(buffer->begin(), buffer->end());

    // 解析 JSON
    rapidjson::Document doc;
    doc.Parse(resultStr.c_str());

    if (doc.HasParseError()) {
        log("JSON Parse Error");
        return;
    }

    // 假设服务器返回格式: { "users": ["Alice", "Bob", "Myself"] }
    if (doc.HasMember("users") && doc["users"].IsArray()) {
        auto users = doc["users"].GetArray();

        // 1. 先添加一个 "MY HOME" 按钮方便回去
        auto myHomeItem = this->createPlayerItem(g_currentUsername, 9999, -1);
        _listView->pushBackCustomItem(myHomeItem);

        // 2. 遍历服务器返回的用户
        for (int i = 0; i < users.Size(); i++) {
            std::string name = users[i].GetString();

            // 如果是自己，已经在上面添加了 My Home，这里就跳过，或者你可以选择保留
            if (name == g_currentUsername) continue;

            // 目前服务器没返回分数，我们随机生成一个模拟分数 (1000 - 5000)
            int fakeScore = 1000 + (rand() % 4000);

            auto item = this->createPlayerItem(name, fakeScore, i);
            _listView->pushBackCustomItem(item);
        }
    }
}

Widget* PlayerListLayer::createPlayerItem(const std::string& name, int score, int index)
{
    // 1. 条目背景
    auto layout = Layout::create();
    layout->setContentSize(Size(360, 80));
    layout->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
    layout->setBackGroundColor(Color3B(60, 60, 70));

    // 2. 名字
    auto nameLabel = Text::create(name, "fonts/Marker Felt.ttf", 24);
    nameLabel->setAnchorPoint(Vec2(0, 0.5f));
    nameLabel->setPosition(Vec2(20, 55));
    layout->addChild(nameLabel);


    // 4. 按钮 (Visit / Attack)
    auto btn = Button::create("ui/button_normal.png"); // 确保你有这个图，或者用 create()
    if (btn->getContentSize().width <= 0) {
        btn = Button::create();
        btn->setTitleFontSize(20);
        btn->setScale9Enabled(true);
        btn->setContentSize(Size(100, 40));
    }

    // 如果是“自己”，按钮显示 HOME，否则显示 VISIT
    std::string btnText = (name == g_currentUsername) ? "HOME" : "VISIT";
    btn->setTitleText(btnText);
    btn->setTitleColor(Color3B::WHITE);
    btn->setPosition(Vec2(300, 40));

    btn->addClickEventListener([=](Ref*) {
        log("Selected user: %s", name.c_str());

        // 核心：调用回调函数，通知 GameScene 加载这个人的数据
        if (_onVisitCallback) {
            _onVisitCallback(name);
        }

        // 关闭列表
        this->hide();
        });

    layout->addChild(btn);

    return layout;
}