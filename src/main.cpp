#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>

using namespace geode::prelude;

struct FootstepData {
    int currentStep = 0;
    bool wasOnGround = false;
    bool wasJumping = false;
    float footstepTimer = 0.0f;
    CCPoint lastPos;
    float landingCooldown = 0.0f;
};

static FootstepData shit;

void playFootstepsLogic(PlayerObject* player, bool isPlatformer, float dt) {
    bool enabled = Mod::get()->getSettingValue<bool>("enabled");
    if (!enabled) return;
    
    float volume = Mod::get()->getSettingValue<double>("volume");
    
    if (!isPlatformer) return;
    
    if (player->m_yVelocity > 0.0f) return;
    
    bool onGround = player->m_isOnGround || player->m_isOnGround2 || player->m_isOnGround3;
    
    if (shit.landingCooldown > 0.0f) {
        shit.landingCooldown -= dt;
    }
    
    if (onGround && !shit.wasOnGround && shit.landingCooldown <= 0.0f) {
        int stepNum = (shit.currentStep % 14) + 1;
        std::string filename = fmt::format("amestep{}.mp3", stepNum);
        std::string path = geode::utils::string::pathToString(Mod::get()->getResourcesDir() / filename);
        
        auto* sound = FMODAudioEngine::sharedEngine();
        if (sound) {
            sound->playEffect(path.c_str(), 1.0f, 1.0f, volume * 1.5f);
        }
        
        shit.currentStep++;
        shit.landingCooldown = 0.5f;
    }
    
    if (onGround) {
        shit.footstepTimer -= dt;
        
        CCPoint currentPos = player->getPosition();
        float dist = ccpDistance(currentPos, shit.lastPos);
        bool moving = dist > 0.1f;
        
        if (moving && shit.footstepTimer <= 0.0f) {
            int stepNum = (shit.currentStep % 14) + 1;
            std::string filename = fmt::format("amestep{}.mp3", stepNum);
            std::string path = geode::utils::string::pathToString(Mod::get()->getResourcesDir() / filename);
            
            auto* sound = FMODAudioEngine::sharedEngine();
            if (sound) {
                sound->playEffect(path.c_str(), 1.0f, 1.0f, volume);
            }
            
            shit.currentStep++;
            shit.footstepTimer = 0.3f;
        }
    }
    
    shit.wasOnGround = onGround;
    shit.lastPos = player->getPosition();
}

class $modify(MyGJBaseGameLayer, GJBaseGameLayer) {
    bool init() {
        if (!GJBaseGameLayer::init()) return false;
        
        shit.currentStep = 0;
        shit.wasOnGround = false;
        shit.wasJumping = false;
        shit.footstepTimer = 0.0f;
        shit.lastPos = CCPointZero;
        shit.landingCooldown = 0.0f;
        
        return true;
    }
    
    void update(float dt) {
        GJBaseGameLayer::update(dt);
        
        auto* player = m_player1;
        if (!player) return;
        
        playFootstepsLogic(player, m_isPlatformer, dt);
    }
};

class $modify(MyLevelEditorLayer, LevelEditorLayer) {
    void update(float dt) {
        LevelEditorLayer::update(dt);
        
        auto* player = m_player1;
        if (!player) return;
        
        playFootstepsLogic(player, m_isPlatformer, dt);
    }
};