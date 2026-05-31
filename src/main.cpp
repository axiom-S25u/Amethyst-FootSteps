#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>

using namespace geode::prelude;

struct PlayerStepState {
    int currentStep = 0;
    bool wasOnGround = false;
    float footstepTimer = 0.0f;
    CCPoint lastPos;
    float landingCooldown = 0.0f;
};

void playFootstepsLogic(PlayerObject* player, bool isPlatformer, float dt, PlayerStepState& s) {
    bool enabled = Mod::get()->getSettingValue<bool>("enabled");
    if (!enabled) return;

    float volume = Mod::get()->getSettingValue<double>("volume");

    if (!isPlatformer) return;

    if (player->m_yVelocity > 0.0f) return;

    bool onGround = player->m_isOnGround || player->m_isOnGround2 || player->m_isOnGround3;

    if (s.landingCooldown > 0.0f) {
        s.landingCooldown -= dt;
    }

    if (onGround && !s.wasOnGround && s.landingCooldown <= 0.0f) {
        int stepNum = (s.currentStep % 14) + 1;
        std::string filename = fmt::format("amestep{}.mp3", stepNum);
        std::string path = geode::utils::string::pathToString(Mod::get()->getResourcesDir() / filename);

        FMODAudioEngine::sharedEngine()->playEffect(path.c_str(), 1.0f, 1.0f, volume * 1.5f);

        s.currentStep++;
        s.landingCooldown = 0.35f;
    }

    if (onGround) {
        s.footstepTimer -= dt;

        CCPoint currentPos = player->getPosition();
        float dist = ccpDistance(currentPos, s.lastPos);
        bool moving = dist > 0.1f;

        if (moving && s.footstepTimer <= 0.0f) {
            int stepNum = (s.currentStep % 14) + 1;
            std::string filename = fmt::format("amestep{}.mp3", stepNum);
            std::string path = geode::utils::string::pathToString(Mod::get()->getResourcesDir() / filename);

            FMODAudioEngine::sharedEngine()->playEffect(path.c_str(), 1.0f, 1.0f, volume);

            s.currentStep++;
            s.footstepTimer = 0.22f; // idk it seems a little not lore accurate but works
        }
    }

    s.wasOnGround = onGround;
    s.lastPos = player->getPosition();
}

class $modify(MyGJBaseGameLayer, GJBaseGameLayer) {
    struct Fields {
        PlayerStepState p1;
        PlayerStepState p2;
    };

    bool init() {
        if (!GJBaseGameLayer::init()) return false;

        m_fields->p1 = PlayerStepState{};
        m_fields->p2 = PlayerStepState{};

        return true;
    }

    void update(float dt) {
        GJBaseGameLayer::update(dt);

        if (m_player1) playFootstepsLogic(m_player1, m_isPlatformer, dt, m_fields->p1);
        if (m_player2) playFootstepsLogic(m_player2, m_isPlatformer, dt, m_fields->p2);
    }
};