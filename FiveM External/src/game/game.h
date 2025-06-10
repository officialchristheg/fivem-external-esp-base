#pragma once
#include <cstdint>
#include "../mem/memify.h"

inline std::vector<std::string> processes = { "FiveM_b3095_GTAProcess.exe", "FiveM_GTAProcess.exe"};
inline memify mem(processes);
class Overlay;

namespace FiveM {
    namespace offset {
        inline uintptr_t world, replay, viewport, camera, localplayer;
        inline uintptr_t boneList, boneMatrix = 0x60;
        inline uintptr_t playerInfo, playerNetID, playerHealth, playerArmor, playerPosition = 0x90, visFlag, driver, maxHealth, configflags, weaponManager, isPlayerAiming, playerAimingAt, handleBullet, lastVehicle;
        inline uintptr_t base;
        inline uintptr_t weaponInfo;  
        inline uintptr_t weaponName;
    }

    extern std::vector<uintptr_t> peds;
    extern std::vector<uintptr_t> newPeds;

    namespace ESP {
        void RunESP(Overlay& overlay); 
    }

	void Setup();
	void Update();

    enum BoneID : int
    {
        HEAD = 0,
        LEFT_FOOT = 1,
        RIGHT_FOOT = 2,
        LEFT_ANKLE = 3,
        RIGHT_ANKLE = 4,
        LEFT_HAND = 5,
        RIGHT_HAND = 6,
        NECK = 7,
        HIP = 8
    };

    static const std::pair<BoneID, BoneID> skeletonPairs[] = {
        { HEAD, NECK },
        { NECK, HIP },
        { NECK, LEFT_HAND },
        { NECK, RIGHT_HAND },
        { HIP, LEFT_FOOT },
        { HIP, RIGHT_FOOT }
    };
}