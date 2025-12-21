#include "EnemyBuilding.h"
// ����������������� Soldier.h
#include "Soldier.h"

USING_NS_CC;


// 1. ʵ�� create
EnemyBuilding* EnemyBuilding::create(const std::string& filename, const std::string& hpBarFilename, int totalHp, int damagePerNotch, int attack, float range)
{
    EnemyBuilding* pRet = new(std::nothrow) EnemyBuilding();
    // ������������
    if (pRet && pRet->init(filename, hpBarFilename, totalHp, damagePerNotch, attack, range))
    {
        pRet->autorelease();
        return pRet;
    }
    else
    {
        delete pRet;
        pRet = nullptr;
        return nullptr;
    }
}

bool EnemyBuilding::init(const std::string& filename, const std::string& hpBarFilename, int totalHp, int damagePerNotch, int attack, float range)
{
    // 1. ��ʼ�����������ͼƬ 
    if (!Sprite::initWithFile(filename)) return false;

    _maxHp = totalHp;
    _currentHp = totalHp;
    _damagePerNotch = damagePerNotch;

    // ����������ʼ���ݻ�״̬Ϊ false
    _isDestroyed = false;

    // 2. ����Ѫ��
    _healthBar = Sprite::create(hpBarFilename);
    if (_healthBar)
    {
        // �������߼���Ѫ���ĵȷֳ�ʼ��
        // ��ȡ����Ѫ��ͼƬ�ĳߴ�
        float textureWidth = _healthBar->getContentSize().width;
        float textureHeight = _healthBar->getContentSize().height;

        // ����ÿһ��Ŀ�� (һ��4��״̬)
        float frameWidth = textureWidth / 4.0f;

        // ��ʼ״̬����ʾ�� 4 �� (��Ѫ)��x ����ӦΪ 3 * frameWidth
        float startX = 3.0f * frameWidth;
        _healthBar->setTextureRect(Rect(startX, 0, frameWidth, textureHeight));

        // ��Ѫ�����ڽ���ͷ��
        // Vec2(���ݿ�ȵ�һ��, ���ݸ߶� + һ��ƫ��)
        _healthBar->setPosition(Vec2(this->getContentSize().width / 2, this->getContentSize().height + 20));
        _healthBar->setScale(3.0f);

        this->addChild(_healthBar,100);
    }
    else
    {
        // ��������ǿ��ַ�����ֱ����Ϊ��ָ��
        _healthBar = nullptr;
    }

    // ����������ʼ����������
    _attackPower = attack;
    _attackRange = range;
    _attackCooldown = 1.0f; // ����1�빥��һ�Σ�Ҳ�������ɲ�������
    _attackTimer = 0.0f;    // ��ʱ������

    return true;
}

void EnemyBuilding::updateTowerLogic(float dt, const cocos2d::Vector<Soldier*>& soldiers)
{
    if (_isDestroyed || _attackPower <= 0) return;

    _attackTimer += dt;
    if (_attackTimer < _attackCooldown) {
        return;
    }

    Soldier* target = nullptr;
    float minDistance = _attackRange;

    // 根据建筑类型决定寻找逻辑
    if (_type == EnemyType::CANNON) {
        // === 加农炮逻辑：先找空军，再找陆军 ===

        // 1. 先遍历找最近的空军
        for (auto soldier : soldiers) {
            if (soldier && soldier->getParent() && soldier->getCurrentHp() > 0) {
                // 必须是空军
                if (soldier->isFlying()) {
                    float dist = this->getPosition().distance(soldier->getPosition());
                    if (dist < minDistance) {
                        minDistance = dist;
                        target = soldier;
                    }
                }
            }
        }

        // 2. 如果没找到空军，再找最近的陆军
        if (!target) {
            minDistance = _attackRange; // 重置距离范围
            for (auto soldier : soldiers) {
                if (soldier && soldier->getParent() && soldier->getCurrentHp() > 0) {
                    // 必须不是空军
                    if (!soldier->isFlying()) {
                        float dist = this->getPosition().distance(soldier->getPosition());
                        if (dist < minDistance) {
                            minDistance = dist;
                            target = soldier;
                        }
                    }
                }
            }
        }
    }
    else if (_type == EnemyType::TOWER) {
        // === 普通塔逻辑：只打陆军 ===
        for (auto soldier : soldiers) {
            if (soldier && soldier->getParent() && soldier->getCurrentHp() > 0) {
                // 忽略飞行单位
                if (!soldier->isFlying()) {
                    float dist = this->getPosition().distance(soldier->getPosition());
                    if (dist < minDistance) {
                        minDistance = dist;
                        target = soldier;
                    }
                }
            }
        }
    }
    else {
        // Base 或其他：原样逻辑（攻击最近的任意敌人）
        for (auto soldier : soldiers) {
            if (soldier && soldier->getParent() && soldier->getCurrentHp() > 0) {
                float dist = this->getPosition().distance(soldier->getPosition());
                if (dist < minDistance) {
                    minDistance = dist;
                    target = soldier;
                }
            }
        }
    }

    if (target) {
        fireMissile(target);
        _attackTimer = 0.0f;
    }
}
// EnemyBuilding.cpp

void EnemyBuilding::fireMissile(Soldier* target)
{
    // �������
    if (_isDestroyed || !target) return;

    // 1. ��������
    auto missile = Sprite::create("weapon/cannonball.png"); // ȷ��·����
    if (!missile) return;

    missile->setPosition(this->getContentSize().width / 2, this->getContentSize().height / 2 + 50);
    missile->setScale(0.8f);
    this->addChild(missile, 10);

    // ���������ʱ��
    Vec2 targetPosInTower = this->convertToNodeSpace(target->getParent()->convertToWorldSpace(target->getPosition()));
    float distance = missile->getPosition().distance(targetPosInTower);
    float speed = 400.0f;
    float duration = distance / speed;

    // �����Ƕ�
    Vec2 direction = targetPosInTower - missile->getPosition();
    float angle = CC_RADIANS_TO_DEGREES(atan2(direction.y, direction.x));
    missile->setRotation(-angle);

    // ============================================================
    // �������޸���ʼ��
    // ============================================================

    // A. ��ǰ��ȡ������ (ֵ����)
    // ���� Lambda ��Ͳ���Ҫ���� 'this' �ˣ���ֹ�������±���
    int damage = this->_attackPower;

    // B. ����Ŀ�� (Retain)
    // ǿ�Ƹ� target �����ü��� +1����ʹʿ���ڱ𴦱�����(removeFromParent)��
    // �����ڴ�Ҳ���ᱻ�����ͷţ�ֱ�����Ǻ� release��
    target->retain();

    auto seq = Sequence::create(
        MoveTo::create(duration, targetPosInTower),
        CallFunc::create([target, missile, damage]() { // ���޸ġ����� damage�������� this

            // Lambda �ص�ִ��ʱ��

            // C. ���Ŀ���Ƿ��߼���
            // ��Ȼ retain ��֤��ָ�벻������ʿ�������Ѿ����ˣ����ڳ������ˣ���
            // ��� getParent() Ϊ�գ�˵��ʿ���Ѿ����Ƴ���Ϸ�����ǾͲ�����˺��ˡ�
            if (target->getParent()) {
                target->takeDamage(damage);
            }

            // D. �Ƴ�����
            missile->removeFromParent();

            // E. �ͷ�Ŀ�� (Release)
            // ��Ӧǰ��� retain�����ʿ��֮ǰ�Ѿ����Ƴ������������ release �������ü������㣬������������
            target->release();
            }),
        nullptr
    );

    missile->runAction(seq);

    // ============================================================
    // �������޸�������
    // ============================================================
}



void EnemyBuilding::takeDamage(int damage)
{
    // ����Ѿ����ݻ٣����ٴ����˺�
    if (_isDestroyed) return;

    _currentHp -= damage;
    if (_currentHp < 0) _currentHp = 0;

    // ��Ѫ��ˢ��Ѫ��
    this->updateHealthBar();

    if (_currentHp == 0) {
        // ���Ϊ�Ѵݻ�
        _isDestroyed = true;

        // �����������ű�ը����
        this->playExplosionEffect();

        // �����߼�
        log("Building Destroyed!");

        // ��ұ�ʾ��� (����Ա��������Ҳ����ȥ�������ը���Ч��)
        this->setColor(Color3B::GRAY);

        // ������������Ϊ0
        _attackPower = 0;

        // ����Ѫ������ѡ��ͨ������ը��Ѫ��ҲӦ����ʧ��
        if (_healthBar) {
            _healthBar->setVisible(false);
        }
    }
}


void EnemyBuilding::updateHealthBar()
{
    if (!_healthBar) return;

    // �������߼�������Ѫ��������ʾ�ڼ���ͼ
    // ����ͼƬ˳���ǣ�[��Ѫ] [��Ѫ] [��Ѫ] [��Ѫ] (���� 0, 1, 2, 3)

    // ���㵱ǰ��Ѫ���˼��� Notch (��)
    int lostHp = _maxHp - _currentHp;
    int lostNotches = lostHp / _damagePerNotch;

    // ���������� 0 �� 3 ֮��
    if (lostNotches > 3) lostNotches = 3;

    // ������ͼ����
    float textureWidth = _healthBar->getTexture()->getContentSize().width; // ԭͼ�ܿ�
    float textureHeight = _healthBar->getTexture()->getContentSize().height;
    float frameWidth = textureWidth / 4.0f; // �����

    // ������ʾ������
    // x ���� = ���� * �����
    _healthBar->setTextureRect(Rect((3 - lostNotches) * frameWidth, 0, frameWidth, textureHeight));
}

void EnemyBuilding::playExplosionEffect()
{
    // 1. ������ը Sprite
    auto explosionSprite = Sprite::create();

    // ���޸ĵ�1����������
    // ��Ϊ����Ҫ�����ӵ���ͼ�ϣ�����ͬ����������λ��ֱ�ӵ������ڵ�ͼ�ϵ�λ��
    explosionSprite->setPosition(this->getPosition());

    explosionSprite->setScale(5.0f); // ����������õĴ�С

    // ���޸ĵ�2����ӵ����ڵ㣨��ͼ�������� this
    // this->getLocalZOrder() + 10 ȷ������Զ������this���� 10 �㣬��ʾ����ǰ��
    if (this->getParent()) {
        this->getParent()->addChild(explosionSprite, this->getLocalZOrder() + 10);
    }
    else {
        //�Դ˷�����һ����û�и��ڵ���������Ȼ��̫���ܣ�
        this->addChild(explosionSprite, 20);
        explosionSprite->setPosition(Vec2(this->getContentSize().width / 2, this->getContentSize().height / 2));
    }

    // 2. ���ض���֡ (����ԭ��)
    Vector<SpriteFrame*> animFrames;
    char str[100] = { 0 };

    for (int i = 1; i <= 9; i++)
    {
        sprintf(str, "soldiers/Explosion%d.png", i);
        auto sprite = Sprite::create(str);
        if (sprite) {
            animFrames.pushBack(sprite->getSpriteFrame());
        }
    }

    // 3. �������� (����ԭ��)
    auto animation = Animation::createWithSpriteFrames(animFrames, 0.1f);

    // 4. �������� (����ԭ��)
    auto seq = Sequence::create(
        Animate::create(animation),
        RemoveSelf::create(),
        nullptr
    );

    explosionSprite->runAction(seq);
}