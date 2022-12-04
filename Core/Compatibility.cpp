// Copyright (c) 2013- PPSSPP Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0 or later versions.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official git repository and contact information can be found at
// https://github.com/hrydgard/ppsspp and http://www.ppsspp.org/.

#include <cstring>

#include "Common/Log.h"
#include "Common/Data/Format/IniFile.h"
#include "Common/StringUtils.h"
#include "Core/Compatibility.h"
#include "Core/Config.h"
#include "Core/System.h"

void Compatibility::Load(const std::string &gameID) {
	Clear();

	// Allow ignoring compat settings by name (regardless of game ID.)
	std::vector<std::string> ignored;
	SplitString(g_Config.sIgnoreCompatSettings, ',', ignored);
	ignored_ = std::set<std::string>(ignored.begin(), ignored.end());
	// If ALL, don't process any compat flags.
	if (ignored_.find("ALL") != ignored_.end())
		return;

	{
		IniFile compat;
		// This loads from assets.
		if (compat.LoadFromVFS("compat.ini")) {
			CheckSettings(compat, gameID);
		}
	}

	{
		IniFile compat2;
		// This one is user-editable. Need to load it after the system one.
		Path path = GetSysDirectory(DIRECTORY_SYSTEM) / "compat.ini";
		if (compat2.Load(path.ToString())) {
			CheckSettings(compat2, gameID);
		}
	}

	{
		IniFile compat;
		// This loads from assets.
		if (compat.LoadFromVFS("compatvr.ini")) {
			CheckVRSettings(compat, gameID);
		}
	}

	{
		IniFile compat2;
		// This one is user-editable. Need to load it after the system one.
		Path path = GetSysDirectory(DIRECTORY_SYSTEM) / "compatvr.ini";
		if (compat2.Load(path.ToString())) {
			CheckVRSettings(compat2, gameID);
		}
	}
}

void Compatibility::Clear() {
	memset(&flags_, 0, sizeof(flags_));
	memset(&vrCompat_, 0, sizeof(vrCompat_));
}

void Compatibility::CheckSettings(IniFile &iniFile, const std::string &gameID) {
	CheckSetting(iniFile, gameID, "VertexDepthRounding", &flags_.VertexDepthRounding);
	CheckSetting(iniFile, gameID, "PixelDepthRounding", &flags_.PixelDepthRounding);
	CheckSetting(iniFile, gameID, "DepthRangeHack", &flags_.DepthRangeHack);
	CheckSetting(iniFile, gameID, "ClearToRAM", &flags_.ClearToRAM);
	CheckSetting(iniFile, gameID, "Force04154000Download", &flags_.Force04154000Download);
	CheckSetting(iniFile, gameID, "DrawSyncEatCycles", &flags_.DrawSyncEatCycles);
	CheckSetting(iniFile, gameID, "DelayDelayThreadHack", &flags_.DelayDelayThreadHack);
	CheckSetting(iniFile, gameID, "FakeMipmapChange", &flags_.FakeMipmapChange);
	CheckSetting(iniFile, gameID, "HackFixVideo", &flags_.HackFixVideo);
	CheckSetting(iniFile, gameID, "HackFixHangs", &flags_.HackFixHangs);
	CheckSetting(iniFile, gameID, "HackLatchedFramebuffer", &flags_.HackLatchedFramebuffer);
	CheckSetting(iniFile, gameID, "DecreaseVertexDepthAccuracy", &flags_.DecreaseVertexDepthAccuracy);
	CheckSetting(iniFile, gameID, "DecreaseFragmentDepthAccuracy", &flags_.DecreaseFragmentDepthAccuracy);
	CheckSetting(iniFile, gameID, "ForceRangeDownload", &flags_.ForceRangeDownload);
	CheckSetting(iniFile, gameID, "FlushAtStall", &flags_.FlushAtStall);
	CheckSetting(iniFile, gameID, "Force04Download", &flags_.Force04Download);
	CheckSetting(iniFile, gameID, "RequireBufferedRendering", &flags_.RequireBufferedRendering);
	CheckSetting(iniFile, gameID, "RequireBlockTransfer", &flags_.RequireBlockTransfer);
	CheckSetting(iniFile, gameID, "RequireDefaultCPUClock", &flags_.RequireDefaultCPUClock);
	CheckSetting(iniFile, gameID, "DisableAccurateDepth", &flags_.DisableAccurateDepth);
	CheckSetting(iniFile, gameID, "MGS2AcidHack", &flags_.MGS2AcidHack);
	CheckSetting(iniFile, gameID, "SonicRivalsHack", &flags_.SonicRivalsHack);
	CheckSetting(iniFile, gameID, "AdhocHackDisableNBSwitch", &flags_.AdhocHackDisableNBSwitch);
	CheckSetting(iniFile, gameID, "AdhocHackStopFriendClear", &flags_.AdhocHackStopFriendClear);
	CheckSetting(iniFile, gameID, "AdhocHackPhantasyStarOffset", &flags_.AdhocHackPhantasyStarOffset);
	CheckSetting(iniFile, gameID, "BlockTransferAllowCreateFB", &flags_.BlockTransferAllowCreateFB);
	CheckSetting(iniFile, gameID, "IntraVRAMBlockTransferAllowCreateFB", &flags_.IntraVRAMBlockTransferAllowCreateFB);
	CheckSetting(iniFile, gameID, "YugiohSaveFix", &flags_.YugiohSaveFix);
	CheckSetting(iniFile, gameID, "ForceUMDDelay", &flags_.ForceUMDDelay);
	CheckSetting(iniFile, gameID, "ForceMax60FPS", &flags_.ForceMax60FPS);
	CheckSetting(iniFile, gameID, "GoWFramerateHack60", &flags_.GoWFramerateHack60);
	CheckSetting(iniFile, gameID, "GoWFramerateHack30", &flags_.GoWFramerateHack30);
	CheckSetting(iniFile, gameID, "JitInvalidationHack", &flags_.JitInvalidationHack);
	CheckSetting(iniFile, gameID, "HideISOFiles", &flags_.HideISOFiles);
	CheckSetting(iniFile, gameID, "MoreAccurateVMMUL", &flags_.MoreAccurateVMMUL);
	CheckSetting(iniFile, gameID, "ForceSoftwareRenderer", &flags_.ForceSoftwareRenderer);
	CheckSetting(iniFile, gameID, "DarkStalkersPresentHack", &flags_.DarkStalkersPresentHack);
	CheckSetting(iniFile, gameID, "ReportSmallMemstick", &flags_.ReportSmallMemstick);
	CheckSetting(iniFile, gameID, "MemstickFixedFree", &flags_.MemstickFixedFree);
	CheckSetting(iniFile, gameID, "DateLimited", &flags_.DateLimited);
	CheckSetting(iniFile, gameID, "ShaderColorBitmask", &flags_.ShaderColorBitmask);
	CheckSetting(iniFile, gameID, "DisableFirstFrameReadback", &flags_.DisableFirstFrameReadback);
	CheckSetting(iniFile, gameID, "MpegAvcWarmUp", &flags_.MpegAvcWarmUp);
	CheckSetting(iniFile, gameID, "BlueToAlpha", &flags_.BlueToAlpha);
	CheckSetting(iniFile, gameID, "CenteredLines", &flags_.CenteredLines);
	CheckSetting(iniFile, gameID, "MaliDepthStencilBugWorkaround", &flags_.MaliDepthStencilBugWorkaround);
	CheckSetting(iniFile, gameID, "ZZT3SelectHack", &flags_.ZZT3SelectHack);
	CheckSetting(iniFile, gameID, "AllowLargeFBTextureOffsets", &flags_.AllowLargeFBTextureOffsets);
	CheckSetting(iniFile, gameID, "AtracLoopHack", &flags_.AtracLoopHack);
	CheckSetting(iniFile, gameID, "DeswizzleDepth", &flags_.DeswizzleDepth);
	CheckSetting(iniFile, gameID, "SplitFramebufferMargin", &flags_.SplitFramebufferMargin);
	CheckSetting(iniFile, gameID, "ForceLowerResolutionForEffectsOn", &flags_.ForceLowerResolutionForEffectsOn);
	CheckSetting(iniFile, gameID, "ForceLowerResolutionForEffectsOff", &flags_.ForceLowerResolutionForEffectsOff);
	CheckSetting(iniFile, gameID, "AllowDownloadCLUT", &flags_.AllowDownloadCLUT);
	CheckSetting(iniFile, gameID, "NearestFilteringOnFramebufferCreate", &flags_.NearestFilteringOnFramebufferCreate);
	CheckSetting(iniFile, gameID, "SecondaryTextureCache", &flags_.SecondaryTextureCache);
	CheckSetting(iniFile, gameID, "EnglishOrJapaneseOnly", &flags_.EnglishOrJapaneseOnly);
	CheckSetting(iniFile, gameID, "OldAdrenoPixelDepthRoundingGL", &flags_.OldAdrenoPixelDepthRoundingGL);
	CheckSetting(iniFile, gameID, "ForceCircleButtonConfirm", &flags_.ForceCircleButtonConfirm);
}

void Compatibility::CheckVRSettings(IniFile &iniFile, const std::string &gameID) {
	CheckSetting(iniFile, gameID, "IdentityViewHack", &vrCompat_.IdentityViewHack);
	CheckSetting(iniFile, gameID, "Skyplane", &vrCompat_.Skyplane);
	CheckSetting(iniFile, gameID, "UnitsPerMeter", &vrCompat_.UnitsPerMeter);

	NOTICE_LOG(G3D, "UnitsPerMeter for %s: %f", gameID.c_str(), vrCompat_.UnitsPerMeter);
}

void Compatibility::CheckSetting(IniFile &iniFile, const std::string &gameID, const char *option, bool *flag) {
	if (ignored_.find(option) == ignored_.end()) {
		iniFile.Get(option, gameID.c_str(), flag, *flag);

		// Shortcut for debugging, sometimes useful to globally enable compat flags.
		bool all = false;
		iniFile.Get(option, "ALL", &all, false);
		*flag |= all;
	}
}

void Compatibility::CheckSetting(IniFile &iniFile, const std::string &gameID, const char *option, float *flag) {
	std::string value;
	iniFile.Get(option, gameID.c_str(), &value, "0");
	*flag = stof(value);
}
