/*
Plugin Name
Copyright (C) <Year> <Developer> <Email Address>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

#include <chrono>
#include <thread>

#include "plugin-main.h"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

bool obs_module_load(void)
{
	obs_log(LOG_INFO, "plugin loaded successfully (version %s)",
		PLUGIN_VERSION);

    obs_frontend_add_preload_callback(RegisterHotkeys, NULL);
    obs_frontend_add_save_callback(SaveHotkeys, NULL);
    obs_frontend_add_event_callback(EventHandler, NULL);
    
	return true;
}

void obs_module_unload(void)
{
	obs_log(LOG_INFO, "plugin unloaded");
}

// Start the replay buffer if it is not already active.
void StartReplayBuffer() {
    obs_log(LOG_INFO, "Starting Replay Buffer");

    // Start the replay buffer
    bool CurrentlyActive = obs_frontend_replay_buffer_active();

    if (!CurrentlyActive) {
        obs_frontend_replay_buffer_start();
    }
}

// Stops the replay buffer if it is active.
void StopReplayBuffer() {
    obs_log(LOG_INFO, "Stopping Replay Buffer");

    // Stop the replay buffer
    bool CurrentlyActive = obs_frontend_replay_buffer_active();

    if (CurrentlyActive) {
        obs_frontend_replay_buffer_stop();
    }
}

// Handles when hotkeys are pressed.
void CaptureEvent(void *data, obs_hotkey_id id, obs_hotkey_t *hotkey, bool pressed) {
    if (id == CaptureReplayHotkey && pressed) {
        AttemptCaptureReplay();
    }
}

// Handles when the replay buffer is completely stopped, or if it is saved.
void EventHandler(obs_frontend_event event, void *private_data) {
    if (event == OBS_FRONTEND_EVENT_REPLAY_BUFFER_STOPPED) {
        obs_log(LOG_INFO, "Restarting Replay Buffer");
        
        // We only want to restart the buffer when we are saving a replay
        if (CurrentlySaving) {
            CurrentlySaving = false;

            // This is awful. Truly awful. I'm not good enough to figure out a better way right now.
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            StartReplayBuffer();
        }
    }

    // Handles when the replay buffer is saved.
    if (event == OBS_FRONTEND_EVENT_REPLAY_BUFFER_SAVED) {
        if (CurrentlySaving) {
            obs_log(LOG_INFO, "Processing Output Data");

            // https://github.com/redraskal/obs-replay-folders/blob/main/OBSReplayFolders.lua#L31
            // This is how we get the path of the replay file.
            obs_output *BufferOutput = obs_frontend_get_replay_buffer_output();
            calldata *CallData = calldata_create();
            proc_handler_t *ProcHandler = obs_output_get_proc_handler(BufferOutput);

            proc_handler_call(ProcHandler, "get_last_replay", CallData);

            const char *ReplayPath = calldata_string(CallData, "path");

            calldata_destroy(CallData);
            obs_output_release(BufferOutput);

            // TODO: Do something with the replay file.
            obs_log(LOG_INFO, ReplayPath);
            obs_log(LOG_INFO, "Output Data Processed");

            // We're done with the replay, so we can "restart" the buffer by force stopping it.
            StopReplayBuffer();
        }
    }

    if (event == OBS_FRONTEND_EVENT_REPLAY_BUFFER_STARTED) {
        obs_log(LOG_INFO, "Resetting Saving State");
        CurrentlySaving = false;
    }
}

// This function is called when the hotkey is pressed.
void AttemptCaptureReplay() {
    if (CurrentlySaving) {
        return;
    }

    CurrentlySaving = true;

    obs_log(LOG_INFO, "Capturing Replay");

    obs_frontend_replay_buffer_save();
}

// Register hotkeys
void RegisterHotkeys(obs_data_t *save_data, bool saving, void *private_data) {
    CaptureReplayHotkey = obs_hotkey_register_frontend("ShadowReplaySim.CaptureReplay", "Capture Replay", CaptureEvent, NULL);
    obs_data_array_t *CaptureHotkeyArray = obs_data_get_array(save_data, "ShadowReplaySim.CaptureReplay");
    obs_hotkey_load(CaptureReplayHotkey, CaptureHotkeyArray);
}

// Save hotkeys
void SaveHotkeys(obs_data *save_data, bool saving, void *private_data) {
    obs_data_array_t *CaptureHotkeyArray = obs_hotkey_save(CaptureReplayHotkey);
    obs_data_set_array(save_data, "ShadowReplaySim.CaptureReplay", CaptureHotkeyArray);
    obs_data_array_release(CaptureHotkeyArray);
}