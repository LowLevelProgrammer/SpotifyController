#include <iostream>
#include <windows.h>
#include <psapi.h>
#include <vector>

bool IsSpotifyProcess(DWORD pid);

struct WindowsInfo {
    HWND hwnd;
    DWORD pid;
};

std::vector<WindowsInfo> g_windows;

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);

    if (!IsWindowVisible(hwnd)) {
        return TRUE;
    }
    if (!IsSpotifyProcess(pid)) {
        return TRUE;
    }

    char title[256];
    GetWindowTextA(hwnd, title, sizeof(title));

    printf("HWND: %p | PID: %lu | Title: \"%s\"\n", hwnd, pid, title);

    g_windows.emplace_back(hwnd, pid);
    return TRUE;
}

bool IsSpotifyProcess(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);

    if (!hProcess)
        return false;

    char exeName[MAX_PATH];

    if (GetModuleBaseNameA(hProcess, nullptr, exeName, MAX_PATH)) {
        CloseHandle(hProcess);
        return _stricmp(exeName, "spotify.exe") == 0;
    }

    CloseHandle(hProcess);
    return false;
}

void SendHarmlessMessage(HWND hwnd) {
    if (!hwnd) return;

    BOOL result = PostMessage(hwnd, WM_NULL, 0, 0);

    if (result) {
        printf("WM_NULL successfully posted message to spotify\n");
    } else {
        printf("WM_NULL failure\n");
    }
}

void SendPlayPause(HWND hwnd) {
    if (!hwnd) return;

    LPARAM lParam = APPCOMMAND_MEDIA_PLAY_PAUSE << 16;

    BOOL result = PostMessage(hwnd, WM_APPCOMMAND, (WPARAM)hwnd, lParam);

    if (result) {
        printf("WM_APPCOMMAND Play/Pause sent\n");
    } else {
        printf("Failed to send\n");
    }
}

void SendVolumeUp(HWND hwnd) {
    if (!hwnd) return;

    LPARAM lParam = APPCOMMAND_VOLUME_UP << 16;
    BOOL result = PostMessage(hwnd, WM_APPCOMMAND, (WPARAM)hwnd, lParam);

    if (result) {
        printf("WM_APPCOMMAND VolumeUp sent\n");
    } else {
        printf("Failed to send VolumeUp\n");
    }
}

void SendVolumeDown(HWND hwnd) {
    if (!hwnd) return;

    LPARAM lParam = APPCOMMAND_VOLUME_DOWN << 16;
    BOOL result = PostMessage(hwnd, WM_APPCOMMAND, (WPARAM)hwnd, lParam);

    if (result) {
        printf("WM_APPCOMMAND VolumeDown sent\n");
    } else {
        printf("Failed to send VolumeDown\n");
    }
}
void SendKey(HWND hwnd, WPARAM vk, bool keyDown) {
    LPARAM lParam = 0;

    if (!keyDown) {
        lParam |= (1 << 30); // previous state
        lParam |= (1 << 31); // key released
    }

    PostMessage(hwnd,
                keyDown ? WM_KEYDOWN : WM_KEYUP,
                vk,
                lParam);
}
void SendCtrlDown(HWND hwnd) {
    if (!hwnd) return;

    SendKey(hwnd, VK_CONTROL, true); // Ctrl down
    SendKey(hwnd, VK_DOWN, true);    // Down down
    SendKey(hwnd, VK_DOWN, false);   // Down up
    SendKey(hwnd, VK_CONTROL, false);// Ctrl up

    printf("Sent Ctrl + Down to Spotify\n");
}


int main() {
    EnumWindows(EnumWindowsProc, NULL);

    if (!g_windows.empty()) {
        HWND spotifyHwnd = g_windows.front().hwnd;
        SendPlayPause(spotifyHwnd);
    } else {
        printf("No spotify found\n");
    }

    return 0;
}