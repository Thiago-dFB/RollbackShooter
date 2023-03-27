//ugliness number 1, to solve conflicts between Raylib and windows.h
//from https://github.com/raysan5/raylib/issues/1217

#if defined(_WIN32)
// To avoid conflicting windows.h symbols with raylib, some flags are defined
// WARNING: Those flags avoid inclusion of some Win32 headers that could be required
// by user at some point and won't be included...
//-------------------------------------------------------------------------------------

// If defined, the following flags inhibit definition of the indicated items.
#define NOGDICAPMASKS     // CC_*, LC_*, PC_*, CP_*, TC_*, RC_
#define NOVIRTUALKEYCODES // VK_*
#define NOWINMESSAGES     // WM_*, EM_*, LB_*, CB_*
#define NOWINSTYLES       // WS_*, CS_*, ES_*, LBS_*, SBS_*, CBS_*
#define NOSYSMETRICS      // SM_*
#define NOMENUS           // MF_*
#define NOICONS           // IDI_*
#define NOKEYSTATES       // MK_*
#define NOSYSCOMMANDS     // SC_*
#define NORASTEROPS       // Binary and Tertiary raster ops
#define NOSHOWWINDOW      // SW_*
#define OEMRESOURCE       // OEM Resource values
#define NOATOM            // Atom Manager routines
#define NOCLIPBOARD       // Clipboard routines
#define NOCOLOR           // Screen colors
#define NOCTLMGR          // Control and Dialog routines
#define NODRAWTEXT        // DrawText() and DT_*
#define NOGDI             // All GDI defines and routines
#define NOKERNEL          // All KERNEL defines and routines
#define NOUSER            // All USER defines and routines
//#define NONLS             // All NLS defines and routines
#define NOMB              // MB_* and MessageBox()
#define NOMEMMGR          // GMEM_*, LMEM_*, GHND, LHND, associated routines
#define NOMETAFILE        // typedef METAFILEPICT
#define NOMINMAX          // Macros min(a,b) and max(a,b)
#define NOMSG             // typedef MSG and associated routines
#define NOOPENFILE        // OpenFile(), OemToAnsi, AnsiToOem, and OF_*
#define NOSCROLL          // SB_* and scrolling routines
#define NOSERVICE         // All Service Controller routines, SERVICE_ equates, etc.
#define NOSOUND           // Sound driver routines
#define NOTEXTMETRIC      // typedef TEXTMETRIC and associated routines
#define NOWH              // SetWindowsHook and WH_*
#define NOWINOFFSETS      // GWL_*, GCL_*, associated routines
#define NOCOMM            // COMM driver routines
#define NOKANJI           // Kanji support stuff.
#define NOHELP            // Help engine interface.
#define NOPROFILER        // Profiler interface.
#define NODEFERWINDOWPOS  // DeferWindowPos routines
#define NOMCX             // Modem Configuration Extensions

// Type required before windows.h inclusion
typedef struct tagMSG* LPMSG;

#include <windows.h>
#include <winsock.h>

// Type required by some unused function...
typedef struct tagBITMAPINFOHEADER {
    DWORD biSize;
    LONG  biWidth;
    LONG  biHeight;
    WORD  biPlanes;
    WORD  biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG  biXPelsPerMeter;
    LONG  biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} BITMAPINFOHEADER, * PBITMAPINFOHEADER;

#include <objbase.h>
#include <mmreg.h>
#include <mmsystem.h>

// Some required types defined for MSVC/TinyC compiler
#if defined(_MSC_VER) || defined(__TINYC__)
#include "propidl.h"
#endif
#endif

#ifndef RBST_GGPO_HPP
#define RBST_GGPO_HPP

//std
#include <istream>
#include <ostream>
#include <stdio.h>
#include <algorithm>
//GGPO
#include <ggponet.h>

#include "Input.hpp"
#include "Config.hpp"
#include "GameState.hpp"
#include "Presentation.hpp"

GameState ggState;
GGPOSession* ggpo = NULL;
GGPOPlayer ggP1, ggP2;
GGPOPlayerHandle ggHandle1, ggHandle2, localHandle;
bool connected = false;
int framesAheadPenalty = -1;
std::string connectionString = "";
long confirmFrame = 0;

//lifted from GGPO example
//(itself lifted from a Wikipedia article about the algorithm? lul)
int fletcher32_checksum(short* data, size_t len)
{
    int sum1 = 0xffff, sum2 = 0xffff;

    while (len) {
        size_t tlen = len > 360 ? 360 : len;
        len -= tlen;
        do {
            sum1 += *data++;
            sum2 += sum1;
        } while (--tlen);
        sum1 = (sum1 & 0xffff) + (sum1 >> 16);
        sum2 = (sum2 & 0xffff) + (sum2 >> 16);
    }

    /* Second reduction step to reduce sums to 16 bits */
    sum1 = (sum1 & 0xffff) + (sum1 >> 16);
    sum2 = (sum2 & 0xffff) + (sum2 >> 16);
    return sum2 << 16 | sum1;
}

//GGPO deprecated callback
bool __cdecl rbst_begin_game_callback(const char*)
{
    return true;
}

//unused parameter: flags
bool __cdecl rbst_advance_frame_callback(int)
{
    PlayerInputZip zips[2] = { 0 };
    int disconnect_flags;
    //get synced inputs
    ggpo_synchronize_input(ggpo, (void*)zips, sizeof(PlayerInputZip)*2, &disconnect_flags);
    PlayerInput p1 = unzipInput(zips[0]);
    PlayerInput p2 = unzipInput(zips[1]);
    InputData input { p1,p2 };
    //simulate one step
    ggState = simulate(ggState, &cfg, input);
    //this wasn't on vector war but GGPO does expect me to advance frames in this callback or it will fail some assertion
    ggpo_advance_frame(ggpo);
    
    return true;
}

//the 3 following callbacks are largely the same as the example
//C memory management scares the ever living shit out of me so I'm gonna trust it
//I could just assign active state to confirm state and vice versa but GGPO stores the confirm state within the session
//that's for midgame join I guess? still looking at what GGPO can do

bool __cdecl rbst_load_game_state_callback(unsigned char* buffer, int len)
{
    memcpy(&ggState, buffer, len);
    confirmFrame = ggState.frame;
    return true;
}

//unused parameter: frame
bool __cdecl rbst_save_game_state_callback(unsigned char** buffer, int* len, int* checksum, int)
{
    *len = sizeof(ggState);
    *buffer = (unsigned char*)malloc(*len);
    if (!*buffer) {
        return false;
    }
    memcpy(*buffer, &ggState, *len);
    *checksum = fletcher32_checksum((short*)*buffer, *len / 2);
    return true;
}

void __cdecl rbst_free_buffer(void* buffer)
{
    free(buffer);
}

//not using this
bool __cdecl rbst_log_game_state(char* filename, unsigned char* buffer, int)
{
    return true;
}

//event management
bool __cdecl rbst_on_event_callback(GGPOEvent* info)
{
    int progress;
    switch (info->code) {
    case GGPO_EVENTCODE_CONNECTED_TO_PEER:
        connectionString.insert(0, "[NET]Succesfully connected!\n");
        break;
    case GGPO_EVENTCODE_SYNCHRONIZING_WITH_PEER:
        progress = info->u.synchronizing.count;
        char txt[32];
        sprintf_s(txt, "[NET]Synchronizing... %d%% \n", progress);
        connectionString.insert(0, txt);
        break;
    case GGPO_EVENTCODE_SYNCHRONIZED_WITH_PEER:
        connectionString.insert(0, "[NET]Succesfully synchronized!\n");
        break;
    case GGPO_EVENTCODE_RUNNING:
        connectionString = "";
        break;
    case GGPO_EVENTCODE_CONNECTION_INTERRUPTED:
        break;
    case GGPO_EVENTCODE_CONNECTION_RESUMED:
        break;
    case GGPO_EVENTCODE_DISCONNECTED_FROM_PEER:
        connected = false;
        break;
    case GGPO_EVENTCODE_TIMESYNC:
        //this game is ahead by n frames
        //and as such will be penalized down by n*5 frames at 50FPS
        framesAheadPenalty = 5 * info->u.timesync.frames_ahead;
        SetTargetFPS(50);
        break;
    }
    return true;
}

void NewNetworkedSession(std::string remoteAddress, unsigned short port, playerid localPlayer)
{
    GGPOErrorCode ggRes;
    GGPOSessionCallbacks ggCallbacks;
    ggCallbacks.begin_game = rbst_begin_game_callback;
    ggCallbacks.advance_frame = rbst_advance_frame_callback;
    ggCallbacks.load_game_state = rbst_load_game_state_callback;
    ggCallbacks.save_game_state = rbst_save_game_state_callback;
    ggCallbacks.free_buffer = rbst_free_buffer;
    ggCallbacks.log_game_state = rbst_log_game_state;
    ggCallbacks.on_event = rbst_on_event_callback;

    //ugliness number 2, from this open PR: https://github.com/pond3r/ggpo/pull/69/commits/01243225d407eafb1c2e7aa4cefca8f11a107476
    {
#ifdef WIN32
        WORD wVersionRequested = MAKEWORD(2, 2);
        WSADATA wsaData;
        int err = WSAStartup(wVersionRequested, &wsaData);

        if (err != 0) {
            // Can comment out following lines for debugging purposes
            //printf("WSAStartup failed with error: %d\n", err);
            //DWORD lastError = WSAGetLastError();
            //printf("last error code: %d\n", lastError);
            assert(FALSE && "Error initializing winsockets");
        }
#endif
    }

    ggRes = ggpo_start_session(&ggpo, &ggCallbacks, "RBST", 2, sizeof(PlayerInputZip), port);

    //Automatically disconnect at
    ggpo_set_disconnect_timeout(ggpo, 3000);
    //Start disconnect timer at
    ggpo_set_disconnect_notify_start(ggpo, 1000);

    ggP1 = ggP2 = { 0 };
    ggP1.size = ggP2.size = sizeof(GGPOPlayer);
    ggP1.player_num = 1;
    ggP2.player_num = 2;

    switch (localPlayer)
    {
    case 1:
        ggP1.type = GGPO_PLAYERTYPE_LOCAL;
        ggP2.type = GGPO_PLAYERTYPE_REMOTE;
        strcpy_s(ggP2.u.remote.ip_address, remoteAddress.c_str());
        ggP2.u.remote.port = port;
        ggRes = ggpo_add_player(ggpo, &ggP1, &ggHandle1);
        ggRes = ggpo_add_player(ggpo, &ggP2, &ggHandle2);
        ggpo_set_frame_delay(ggpo, ggHandle1, 0);
        localHandle = ggHandle1;
        break;
    case 2:
        ggP2.type = GGPO_PLAYERTYPE_LOCAL;
        ggP1.type = GGPO_PLAYERTYPE_REMOTE;
        strcpy_s(ggP1.u.remote.ip_address, remoteAddress.c_str());
        ggP1.u.remote.port = port;
        ggRes = ggpo_add_player(ggpo, &ggP2, &ggHandle2);
        ggRes = ggpo_add_player(ggpo, &ggP1, &ggHandle1);
        ggpo_set_frame_delay(ggpo, ggHandle2, 0);
        localHandle = ggHandle2;
        break;
    }

    connected = true;
}

void NetworkedMain(const Sprites* sprs, std::string remoteAddress, unsigned short port, playerid localPlayer)
{
    Camera3D cam = initialCamera();
    ggState = initialState(&cfg);
    std::ostringstream gameInfoOSS;
    double semaphoreIdleTime = 0;
    long latestConfFrame = 0;
    long prevConfFrame = 0;

    NewNetworkedSession(remoteAddress, port, localPlayer);
    while (connected && !WindowShouldClose() && !endCondition(&ggState, &cfg))
    {
        //restore framerate to 60FPS after time sync penalty
        framesAheadPenalty = std::max(-1, framesAheadPenalty - 1);
        if (framesAheadPenalty == 0)
        {
            SetTargetFPS(60);
        }

        //check if rollback ever comes back to a frame before the latest confirm one
        if (confirmFrame < latestConfFrame)
        {
            prevConfFrame = confirmFrame;
        }
        else
        {
            latestConfFrame = confirmFrame;
        }
        
        if (IsKeyPressed(KEY_F10))
        {
            GGPOErrorCode ggRes = ggpo_disconnect_player(ggpo, localHandle);
            connected = false;
        }

        //GGPO needs this time to execute rollbacks and send packets
        //try to give as much as you can without lagging the main loop
        int timeGivenToIdle = static_cast<int>(floor(semaphoreIdleTime * 1000)) - 1;
        ggpo_idle(ggpo, std::max(0, timeGivenToIdle));
        
        //input processing
        GGPOErrorCode ggRes = GGPO_OK;
        int disconnect_flags;
        PlayerInputZip zips[2] = { 0 };

        if (localHandle != GGPO_INVALID_HANDLE)
        {
            PlayerInput localInput = processInput(&inputBind);
            PlayerInputZip inputZip = zipInput(localInput);
            ggRes = ggpo_add_local_input(ggpo, localHandle, &inputZip, sizeof(inputZip));
        }

        //input syncing (might have to do with input delay if it's set)
        if (GGPO_SUCCEEDED(ggRes))
        {
            ggRes = ggpo_synchronize_input(ggpo, (void*)zips, sizeof(PlayerInputZip) * 2, &disconnect_flags);
            if (GGPO_SUCCEEDED(ggRes))
            {
                
                PlayerInput p1 = unzipInput(zips[0]);
                PlayerInput p2 = unzipInput(zips[1]);
                InputData input{ p1,p2 };
                ggState = simulate(ggState, &cfg, input);
                //Notify GGPO that a frame has passed;
                ggpo_advance_frame(ggpo);
            }
        }
        POV pov;
        if (localHandle == ggHandle1)
            pov = Player1;
        else
            pov = Player2;

        int currentFps = GetFPS();
        gameInfoOSS.str("");
        gameInfoOSS << "FPS: " << currentFps << std::endl;
        gameInfoOSS << "Semaphore idle time: " << semaphoreIdleTime * 1000 << " ms" << std::endl;
        gameInfoOSS << "Rollbacked frames:" << (ggState.frame - confirmFrame) << std::endl;
        gameInfoOSS << "Latest confirm frame: " << latestConfFrame << std::endl;
        if(prevConfFrame > 0)
        {
            gameInfoOSS << "ROLLBACK TO CONFIRM FRAME BEFORE LATEST DETECTED: " << prevConfFrame << std::endl;
        }
        gameInfoOSS << "P1 HP: " << ggState.health1 << "; ";
        gameInfoOSS << "P2 HP: " << ggState.health2 << std::endl;
        gameInfoOSS << "Round Phase: " << std::to_string(ggState.phase) << "; ";
        gameInfoOSS << "Round Countdown : " << (ggState.roundCountdown / 60) << "." << (ggState.roundCountdown % 60) << std::endl;
        gameInfoOSS << connectionString;

        semaphoreIdleTime = present(pov, &ggState, &cfg, &cam, sprs, &gameInfoOSS);
    }
    //exit session
    if (ggpo)
    {
        ggpo_close_session(ggpo);
        ggpo = NULL;
    }
    connected = false;
    connectionString = "";
    framesAheadPenalty = -1;
    confirmFrame = 0;

    //ugliness number 3, also from the PR
    {
#ifdef WIN32
        // https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-wsacleanup
        int result = WSACleanup();
        if (result != 0) {
            assert(FALSE && "Error de-initializing winsockets");
        }
#endif
    }
}

#endif