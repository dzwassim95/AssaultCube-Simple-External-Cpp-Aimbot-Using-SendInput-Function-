#include "memory.h"
#include "math.h"
#include "offsets.h"
#include "aimbot.h"
#include <thread>
#include <iostream>
#include <iomanip>


// setting the width and the height for imgui

static int setImguiWinSize()
{
    HWND winH = FindWindow(NULL, "AssaultCube");
    
    if (winH == NULL)
    {
        std::cerr << "failed to get window handle!\n";
        return -1;
    }

    RECT rect{};

    if (GetClientRect(winH, &rect))
    {
        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(static_cast<float>(width), static_cast<float>(height));
        return 0;
    }
}



int main() {

    memory game("ac_client.exe");
    const auto client = game.GetModuleBaseAddr("ac_client.exe");
    
    if (client)
    {
        std::cout << "aimbot is running ...\n";
    }

    
    ////////////////////////

    vector3 headPos3D{};
    vector3 headPos2D {};

    ///////////////////////
    
    ImVec2 scn{};
    float ScreenCenterX{};
    float ScreenCenterY{};
    DWORD exitCode{};

    while (true)
    {
        GetExitCodeProcess(game.GetProcH(), &exitCode);
        if (exitCode != STILL_ACTIVE)
        {
            std::cerr << "game not found, exiting . . .\n";
            system("pause");
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        
        if (!(GetKeyState(VK_LSHIFT) & 0x8000))
        {
            continue;
        }

        setImguiWinSize();
        scn = ImGui::GetIO().DisplaySize;

        if (scn.x == 0 && scn.y == 0)
        {
            scn.x = 1920;
            scn.y = 1080;
        }

        const auto localPayer = game.read<uintptr_t>(client + offsets::iLocalPlayer);
        const auto vm = game.read<viewMatrix>(client + offsets::iViewMatrix);
        const auto entity = game.read<uintptr_t>(client + offsets::iEntityList);
        auto isTeam = game.read<int>(localPayer + offsets::iTeam);
        auto playerCount = game.read<int>(client + offsets::iEntityList + 0x8);
        float lowDist = 800.0f ;
        float aimPosX{};
        float aimPosY{};

        for (int i = 0; i < playerCount; ++i)
        {
            auto player = game.read<uintptr_t>(entity + 0x4 * i);
            
            if (game.read<int>(player + offsets::iTeam) == isTeam)
            {
                continue;
            }

            if (game.read<int>(player + offsets::isDead))
            {
                continue;
            }

            auto headPos3D = game.read<vector3>(player + offsets::vHead);

            if (w2scn(headPos3D, headPos2D, vm, scn))
            {
                float distance = dist2D(scn, headPos2D, ScreenCenterX, ScreenCenterY);

                if (distance < lowDist)
                {
                    lowDist = distance;
                    aimPosX = headPos2D.x;
                    aimPosY = headPos2D.y;
                }
            }
        }
        ///aimbot//
        
        aimbot(aimPosX , aimPosY, ScreenCenterX, ScreenCenterY);
        ImGui::DestroyContext();
    }
    
    ImGuiContext* context = ImGui::GetCurrentContext();
    if (context)
    {
        ImGui::DestroyContext();
    }
    
    if (game.GetProcH())
    {
        CloseHandle(game.GetProcH());
    }
    
    ExitProcess(0);
}
