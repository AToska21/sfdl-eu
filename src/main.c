#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#include <curl/curl.h>

#include <sysapp/launch.h>
#include <whb/proc.h>
#include <whb/log.h>
#include <whb/log_console.h>
#include <whb/gfx.h>
#include <coreinit/screen.h>
#include <coreinit/thread.h>
#include <vpad/input.h>
#include <nsysnet/socket.h>

#define URL "https://cdn.abmanagement.al/0000054d"
#define SAVE_DIR "fs:/vol/storage_mlc01/usr/boss/00050000/10176A00/user/common/data/optdat2"
#define SAVE_PATH SAVE_DIR "/0000054d"

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
    return fwrite(ptr, size, nmemb, (FILE *)stream);
}

int download_file(const char *url, const char *save_path) {
    CURL *curl;
    FILE *fp;
    CURLcode res;
    struct stat st = {0};

    // Create directory if it doesn't exist
    if (stat(SAVE_DIR, &st) == -1) {
        mkdir(SAVE_DIR, 0777);
    }

    fp = fopen(save_path, "wb");
    if (!fp) {
        WHBLogPrintf("Error opening file for writing: %s", strerror(errno));
        return -1;
    }
    curl = curl_easy_init();
    if (!curl) {
        WHBLogPrintf("Error initializing CURL");
        fclose(fp);
  //      WHBLogPrintf("Error detected! Closing app in 10 seconds.");
        sleep(10);
        return -1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

    // Ignore SSL certificate verification (since Wii U doesn't have updated CA certificates)
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    fclose(fp);

    if (res != CURLE_OK) {
//        WHBLogPrintf("CURL error: %s", curl_easy_strerror(res));
        return -1;
    }

    return 0;
}

int main(int argc, char **argv) {
    WHBLogConsoleInit();
    WHBProcInit();
    WHBGfxInit();

    WHBLogPrintf("Welcome to aromaSFDL for EU Splatoon!");
    WHBLogPrintf("Version 1.2a!");
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
    int vpad_error;
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

    if (socket_lib_init() < 0) {
        WHBLogPrintf("Failed to initialize network, did you check your network settings?");
        WHBLogConsoleDraw();
        goto exit;
    }

    WHBLogPrintf("Downloading file...");
    WHBLogConsoleDraw();

    if (download_file(URL, SAVE_PATH) != 0) {
//        WHBLogPrintf("[andreas cURL and directory handler] error! failed to download file. more info above");
        WHBLogConsoleDraw();
        goto cleanup_network;
    }

    WHBLogPrintf("File downloaded successfully.");
    WHBLogConsoleDraw();

cleanup_network:
    socket_lib_finish();

exit:
    WHBLogPrintf("Exiting in 10 seconds...");
    WHBLogConsoleDraw();
    sleep(10);

    WHBGfxShutdown();
//    WHBProcShutdown();
    WHBLogConsoleFree();
    SYSLaunchMenu();
    return 0;
}
