#ifndef PLUGINMAIN
#define PLUGINMAIN

#include <obs-module.h>
#include <plugin-support.h>
#include <obs-frontend-api.h>

obs_hotkey_id CaptureReplayHotkey;

bool CurrentlySaving = false;
std::string LastForegroundWindowName = "Desktop";

void DelayedStartReplayBuffer(int miliseconds);
void StartReplayBuffer();
void StopReplayBuffer();

void RegisterHotkeys(obs_data_t *save_data, bool saving, void *private_data);
void SaveHotkeys(obs_data *save_data, bool saving, void *private_data);

void EventHandler(obs_frontend_event event, void *private_data);

void CaptureEvent(void *data, obs_hotkey_id id, obs_hotkey_t *hotkey, bool pressed);

void AttemptCaptureReplay();

void SendToastPowerShell(const char* message);

inline const char* GetForegroundWindowName();

#endif