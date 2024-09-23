#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <fstream>
#include <cerrno>
#include <cstring>
#include <chrono>
#include <thread>
#include <iostream>

#include <curl/curl.h>

#include <sysapp/launch.h>
#include <whb/proc.h>
#include <proc_ui/procui.h>
#include <whb/log.h>
#include <whb/log_console.h>
#include <whb/gfx.h>
#include <coreinit/screen.h>
#include <coreinit/thread.h>
#include <vpad/input.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>


#include <mocha/mocha.h>

#define URL "https://cdn.abmanagement.al/0000054d"
#define SAVE_DIR "/vol/storage_mlc01/usr/boss/00050000/10176a00/user/common/data/optdat2"
#define SAVE_PATH SAVE_DIR "/0000054d"

static bool running = true;

// Custom write function for libcurl to write directly to ofstream
static size_t write_data(void* ptr, size_t size, size_t nmemb, std::ofstream* stream) {
    stream->write(static_cast<char*>(ptr), size * nmemb);
    return size * nmemb;
}

int download_file(const std::string& url, const std::string& save_path) {
    CURL* curl;
    CURLcode res;
    struct stat st = {0};

    // Open file stream using C++ standard library
    std::ofstream ofs(save_path, std::ios::binary);
    if (!ofs) {
        WHBLogPrintf("Error opening file for writing: %s", std::strerror(errno));
        WHBLogConsoleDraw();
        return -1;
    }

    curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Error initializing CURL" << std::endl;
        ofs.close();
        std::this_thread::sleep_for(std::chrono::seconds(10)); // Replace sleep(10) with C++ equivalent
        return -1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ofs);

    // Ignore SSL certificate verification
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    ofs.close();

    if (res != CURLE_OK) {
        std::cerr << "CURL error: " << curl_easy_strerror(res) << std::endl;
        return -1;
    }

    return 0;
}

int client_handle = 0;

int main(int argc, char **argv) {
    ProcUIInit(NULL);

    while (running) {
        switch(ProcUIProcessMessages(true)) {
            case PROCUI_STATUS_EXITING:
                running = false;
                break;
            case PROCUI_STATUS_IN_FOREGROUND:
                WHBLogConsoleInit();
                WHBGfxInit();
                FSAInit();
                if (Mocha_InitLibrary() != MOCHA_RESULT_SUCCESS) {
                    WHBLogPrintf("Failed to init mocha!");
                    WHBLogConsoleDraw();
                    sleep(10);
                return -1;
                }
                client_handle = FSAAddClient(NULL);
                if (client_handle ==0) {
                    return -1;
                }

                Mocha_MountFS("storage_mlc", NULL, "/vol/sfdl_storage");
                if (Mocha_MountFS("storage_mlc", NULL, "/vol/sfdl_storage") == MOCHA_RESULT_ALREADY_EXISTS)
                {
                   WHBLogPrintf("MOCHA MOUNT ERROR: already mounted");
                }
                if (Mocha_MountFS("storage_mlc", NULL, "/vol/sfdl_storage") == MOCHA_RESULT_SUCCESS)
                {
                    WHBLogPrintf("mount success!");
                }
                else
                {
                    WHBLogPrintf("Failed to mount! Reason: %s", Mocha_GetStatusStr(Mocha_MountFS ("storage_mlc", NULL, "/vol/sfdl_storage")));
                }

                WHBLogPrintf("Welcome to aromaSFDL for EU Splatoon!");
                WHBLogPrintf("Version 1.3a!");
                WHBLogPrintf(" ");
                WHBLogPrintf("made by @ssdrive on discord");
                WHBLogPrintf("special thanks to: scraps for risking their wii u, fst purpletote for support,");
                WHBLogPrintf("d0mo for the testing files and the splatfestival staff team for being helpful");
                WHBLogPrintf("id put a heart emoji but this is written in C so");
                WHBLogPrintf(" ");
                WHBLogPrintf("Press A to install.");
                WHBLogPrintf("Press HOME to exit.");
                WHBLogConsoleDraw();

                VPADStatus vpad_data;
                VPADReadError vpad_error;
                bool confirmed = false;
                bool exiting = false;

                while (!confirmed) {
                    VPADRead(VPAD_CHAN_0, &vpad_data, 1, &vpad_error);
                    if (vpad_error == 0 && (vpad_data.trigger & VPAD_BUTTON_A)) {
                        confirmed = true;
                    }
                    OSSleepTicks(OSMillisecondsToTicks(50)); // Sleep for 50ms
                }

                WHBLogPrintf("Initializing network... if this crashes you have a skill issue");
                WHBLogConsoleDraw();

                WHBLogPrintf("Downloading file...");
                WHBLogConsoleDraw();

                if (download_file(URL, SAVE_PATH) != 0) {
                    WHBLogPrintf("Error in downloading");
                    WHBLogConsoleDraw();
                    sleep(999);
                }

                WHBLogPrintf("File downloaded successfully.");
                WHBLogConsoleDraw();


                WHBLogPrintf("Exiting in 10 seconds...");
                WHBLogConsoleDraw();
                sleep(10);

                FSAFlushVolume(client_handle, "/vol/sfdl_storage");
                Mocha_UnmountFS("sfdl_storage");
                Mocha_DeInitLibrary();
                WHBLogConsoleFree();
                WHBGfxShutdown();
                running = false;
                break;
        }
        usleep(10000);
    }
//    SYSLaunchMenu();
    ProcUIShutdown();
    return 0;
}
