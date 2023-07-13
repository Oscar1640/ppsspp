#include <cstring>

#include "Common/System/OSD.h"
#include "Common/TimeUtil.h"
#include "Common/Log.h"

OnScreenDisplay g_OSD;

void OnScreenDisplay::Update() {
	std::lock_guard<std::mutex> guard(mutex_);

	double now = time_now_d();
	for (auto iter = entries_.begin(); iter != entries_.end(); ) {
		if (now >= iter->endTime) {
			iter = entries_.erase(iter);
		} else {
			iter++;
		}
	}

	for (auto iter = sideEntries_.begin(); iter != sideEntries_.end(); ) {
		if (now >= iter->endTime) {
			iter = sideEntries_.erase(iter);
		} else {
			iter++;
		}
	}

	for (auto iter = bars_.begin(); iter != bars_.end(); ) {
		if (now >= iter->endTime) {
			iter = bars_.erase(iter);
		} else {
			iter++;
		}
	}
}

std::vector<OnScreenDisplay::Entry> OnScreenDisplay::Entries() {
	std::lock_guard<std::mutex> guard(mutex_);
	return entries_;  // makes a copy.
}

std::vector<OnScreenDisplay::Entry> OnScreenDisplay::SideEntries() {
	std::lock_guard<std::mutex> guard(mutex_);
	return sideEntries_;  // makes a copy.
}

std::vector<OnScreenDisplay::ProgressBar> OnScreenDisplay::ProgressBars() {
	std::lock_guard<std::mutex> guard(mutex_);
	return bars_;  // makes a copy.
}

void OnScreenDisplay::Show(OSDType type, const std::string &text, const std::string &text2, const std::string &icon, float duration_s, const char *id) {
	// Automatic duration based on type.
	if (duration_s <= 0.0f) {
		switch (type) {
		case OSDType::MESSAGE_ERROR:
		case OSDType::MESSAGE_WARNING:
			duration_s = 4.0f;
			break;
		case OSDType::MESSAGE_FILE_LINK:
			duration_s = 5.0f;
			break;
		case OSDType::MESSAGE_SUCCESS:
			duration_s = 2.0f;
			break;
		default:
			duration_s = 1.5f;
			break;
		}
	}

	double now = time_now_d();
	std::lock_guard<std::mutex> guard(mutex_);
	if (id) {
		for (auto iter = entries_.begin(); iter != entries_.end(); ++iter) {
			if (iter->id && !strcmp(iter->id, id)) {
				Entry msg = *iter;
				msg.endTime = now + duration_s;
				msg.text = text;
				msg.text2 = text2;
				msg.type = type;
				msg.iconName = icon;
				// Move to top (should we? maybe not?)
				entries_.erase(iter);
				entries_.insert(entries_.begin(), msg);
				return;
			}
		}
	}

	Entry msg;
	msg.text = text;
	msg.text2 = text2;
	msg.iconName = icon;
	msg.startTime = now;
	msg.endTime = now + duration_s;
	msg.type = type;
	msg.id = id;
	entries_.insert(entries_.begin(), msg);
}

void OnScreenDisplay::ShowAchievementUnlocked(int achievementID) {
	double now = time_now_d();

	double duration_s = 5.0;

	Entry msg;
	msg.numericID = achievementID;
	msg.type = OSDType::ACHIEVEMENT_UNLOCKED;
	msg.startTime = now;
	msg.endTime = now + duration_s;
	entries_.insert(entries_.begin(), msg);
}

void OnScreenDisplay::ShowAchievementProgress(int achievementID, float duration_s) {
	double now = time_now_d();

	for (auto &entry : sideEntries_) {
		if (entry.numericID == achievementID && entry.type == OSDType::ACHIEVEMENT_PROGRESS) {
			// Duplicate, let's just bump the timer.
			entry.startTime = now;
			entry.endTime = now + (double)duration_s;
			// We're done.
			return;
		}
	}

	// OK, let's make a new side-entry.
	Entry entry;
	entry.numericID = achievementID;
	entry.type = OSDType::ACHIEVEMENT_PROGRESS;
	entry.startTime = now;
	entry.endTime = now + (double)duration_s;
	sideEntries_.insert(sideEntries_.begin(), entry);
}

void OnScreenDisplay::ShowChallengeIndicator(int achievementID, bool show) {
	double now = time_now_d();

	for (auto &entry : sideEntries_) {
		if (entry.numericID == achievementID && entry.type == OSDType::ACHIEVEMENT_CHALLENGE_INDICATOR && !show) {
			// Hide and eventually delete it.
			entry.endTime = now + (double)FadeoutTime();
			// Found it, we're done.
			return;
		}
	}

	if (!show) {
		// Sanity check
		return;
	}

	// OK, let's make a new side-entry.
	Entry entry;
	entry.numericID = achievementID;
	entry.type = OSDType::ACHIEVEMENT_CHALLENGE_INDICATOR;
	entry.startTime = now;
	entry.endTime = now + 10000000.0;  // Don't auto-fadeout.
	sideEntries_.insert(sideEntries_.begin(), entry);
}

void OnScreenDisplay::ShowLeaderboardTracker(int leaderboardTrackerID, const char *trackerText, bool show) {   // show=true is used both for create and update.
	double now = time_now_d();

	for (auto &entry : sideEntries_) {
		if (entry.numericID == leaderboardTrackerID && entry.type == OSDType::LEADERBOARD_TRACKER) {
			if (show) {
				// Just an update.
				entry.text = trackerText ? trackerText : "";
			} else {
				// Keep the current text, hide and eventually delete it.
				entry.endTime = now + (double)FadeoutTime();
			}
			// Found it, we're done.
			return;
		}
	}

	if (!show) {
		// Sanity check
		return;
	}

	// OK, let's make a new side-entry.
	Entry entry;
	entry.numericID = leaderboardTrackerID;
	entry.type = OSDType::LEADERBOARD_TRACKER;
	entry.startTime = now;
	entry.endTime = now + 10000000.0;  // Don't auto-fadeout
	if (trackerText) {
		entry.text = trackerText;
	}
	sideEntries_.insert(sideEntries_.begin(), entry);
}

void OnScreenDisplay::ShowOnOff(const std::string &message, bool on, float duration_s) {
	// TODO: translate "on" and "off"? Or just get rid of this whole thing?
	Show(OSDType::MESSAGE_INFO, message + ": " + (on ? "on" : "off"), duration_s);
}

void OnScreenDisplay::SetProgressBar(std::string id, std::string &&message, int minValue, int maxValue, int progress) {
	std::lock_guard<std::mutex> guard(mutex_);
	double now = time_now_d();
	bool found = false;
	for (auto &bar : bars_) {
		if (bar.id == id) {
			bar.minValue = minValue;
			bar.maxValue = maxValue;
			bar.progress = progress;
			bar.message = message;
			bar.endTime = now + 60.0;  // Nudge the progress bar to keep it shown.
			return;
		}
	}

	ProgressBar bar;
	bar.id = id;
	bar.message = std::move(message);
	bar.minValue = minValue;
	bar.maxValue = maxValue;
	bar.progress = progress;
	bar.endTime = now + 60.0;  // Show the progress bar for 60 seconds, then fade it out.
	bars_.push_back(bar);
}

void OnScreenDisplay::RemoveProgressBar(std::string id) {
	std::lock_guard<std::mutex> guard(mutex_);
	for (auto iter = bars_.begin(); iter != bars_.end(); iter++) {
		if (iter->id == id) {
			iter->progress = iter->maxValue;
			iter->endTime = time_now_d() + FadeoutTime();
			break;
		}
	}
}

// Fades out everything related to achievements. Should be used on game shutdown.
void OnScreenDisplay::ClearAchievementStuff() {
	double now = time_now_d();
	for (auto &iter : entries_) {
		switch (iter.type) {
		case OSDType::ACHIEVEMENT_CHALLENGE_INDICATOR:
		case OSDType::ACHIEVEMENT_UNLOCKED:
		case OSDType::ACHIEVEMENT_PROGRESS:
		case OSDType::LEADERBOARD_TRACKER:
			iter.endTime = now;
			break;
		default:
			break;
		}
	}
	for (auto &iter : sideEntries_) {
		switch (iter.type) {
		case OSDType::ACHIEVEMENT_CHALLENGE_INDICATOR:
		case OSDType::ACHIEVEMENT_UNLOCKED:
		case OSDType::ACHIEVEMENT_PROGRESS:
		case OSDType::LEADERBOARD_TRACKER:
			iter.endTime = now;
			break;
		default:
			break;
		}
	}
}
