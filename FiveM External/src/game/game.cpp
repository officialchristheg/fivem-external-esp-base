// Credits to -> https://www.youtube.com/watch?v=1tAe66kxtXA
#include "game.h"
#include "../math/math.h"
#include "../window/window.hpp"
#include <iostream>  
#include <iomanip>  
#include <thread>
#include <mutex>
#include <algorithm>
#include <vector>
#include <string>
#include <cctype>
#include <unordered_map>  
#include <nlohmann/json.hpp>
#include <Windows.h>
#include <fstream>

// happy pasting
using json = nlohmann::json;
std::vector<uintptr_t> FiveM::peds;
std::vector<uintptr_t> FiveM::newPeds;
std::vector<uintptr_t> pedsBuffer;
std::mutex pedsMutex;

void FiveM::Setup() {
    auto process_name = mem.GetProcessName();
    using namespace offset;

    auto game_base = mem.GetBase(process_name);
    base = game_base;

    // 3258 for now only
    // if you want to make it for all gameversion you have to get the process name by searching for:
    // GTAProcess.exe then there is a offset for you to get the game version, then assign the offsets
    // do some research or reverse fivem its easy to find the offset
    
    if (process_name == "FiveM_GTAProcess.exe") {
        world = mem.Read<uintptr_t>(game_base + 0x25B14B0);
        viewport = mem.Read<uintptr_t>(game_base + 0x201DBA0);
        replay = mem.Read<uintptr_t>(game_base + 0x1FBD4F0);
        localplayer = mem.Read<uintptr_t>(world + 0x8);
        playerInfo = 0x10A8;
        visFlag = 0x145C;
        playerNetID = 0xE8;
        weaponManager = 0x10B8;
        playerArmor = 0x150C;
        playerHealth = 0x280;
        camera = 0x201E7D0;
    }
}

// Sorry for being so unorganized, I never planned to code here further, just learning a bit game hacking

void FiveM::Update() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        std::vector<uintptr_t> newPeds;

        // pedreplayinterface replay + 0x18
        uintptr_t ped_replay_interface = mem.Read<uintptr_t>(offset::replay + 0x18);
        if (!ped_replay_interface)
            continue;

        // interface + 100
        uintptr_t pedList = mem.Read<uintptr_t>(ped_replay_interface + 0x100);
        if (!pedList)
            continue;

        // Caching Peds
        for (int i = 0; i < 200; i++) {
            uintptr_t ped = mem.Read<uintptr_t>(pedList + (i * 0x10));
            if (!ped)
                continue;

            newPeds.push_back(ped);
        }

        std::lock_guard<std::mutex> lock(pedsMutex);
        peds.swap(newPeds);
    }
}

// todo: getbonepos

// saw this on UC
static Vec3 GetBonePosition(uintptr_t ped, int boneIndex)
{
    // 0x430 for older version :3
    Vec3 localPos = mem.Read<Vec3>(ped + 0x410 + (0x10 * boneIndex));
    Matrix boneMatrix = mem.Read<Matrix>(ped + 0x60);
    return localPos.Transform(&boneMatrix);
}

void FiveM::ESP::RunESP(Overlay& overlay)
{
    if (!overlay.drawESP)
        return;

    Matrix viewMatrix = mem.Read<Matrix>(offset::viewport + 0x24C);
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();

    std::vector<uintptr_t> currentPeds;
    {
        std::lock_guard<std::mutex> lock(pedsMutex);
        currentPeds = FiveM::peds;
    }

    // (meow) https://cdn.discordapp.com/attachments/1035339223051538486/1298713844558069830/togif.gif?ex=68492145&is=6847cfc5&hm=59998ee800636935b43f12b18be3b48c5893e0417197b3ea2fe60719d8d7888f&
    static std::vector<FiveM::BoneID> bones = {
        FiveM::HEAD,
        FiveM::NECK,
        FiveM::HIP,
        FiveM::LEFT_FOOT,
        FiveM::RIGHT_FOOT,
        FiveM::LEFT_HAND,
        FiveM::RIGHT_HAND
    };

    // https://media0.giphy.com/media/v1.Y2lkPTc5MGI3NjExdjJ1c3ByNDhoZnZjMzZmZjdvdWU0N2xxMWxnMm9zZ29pMHFsZDl2eCZlcD12MV9pbnRlcm5hbF9naWZfYnlfaWQmY3Q9Zw/Gd1Puu1Y4s5x9VZlLz/giphy.gif
    for (auto ped : currentPeds)
    {
        float health = mem.Read<float>(ped + offset::playerHealth);
        if (health <= 0.f) continue;

        std::array<Vec2, 7> boneScreen;
        std::array<bool, 7> boneVisible;

        for (int i = 0; i < (int)bones.size(); i++) {
            Vec3 bonePos = GetBonePosition(ped, (int)bones[i]);
            Vec2 screenPos;
            if (bonePos.world_to_screen(viewMatrix, screenPos)) {
                boneScreen[i] = screenPos;
                boneVisible[i] = true;
            }
            else {
                boneVisible[i] = false;
            }
        }

        // Drawing the root skeletons (mrrp :33)
        for (auto& pair : FiveM::skeletonPairs) {
            int idxA = -1, idxB = -1;
            for (int b = 0; b < (int)bones.size(); b++) {
                if (bones[b] == pair.first) idxA = b;
                if (bones[b] == pair.second) idxB = b;
            }
            if (idxA >= 0 && idxB >= 0 && boneVisible[idxA] && boneVisible[idxB]) {
                drawList->AddLine(
                    ImVec2(boneScreen[idxA].x, boneScreen[idxA].y),
                    ImVec2(boneScreen[idxB].x, boneScreen[idxB].y),
                    ImColor(255, 255, 255, 255), 1.2f
                );
            }
        }
    }
}

// https://open.spotify.com/episode/2AS22J6FVMo3PDlvXbVmUR?si=34c9519f863a4201