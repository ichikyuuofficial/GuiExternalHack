#pragma once

#include <cstdint>
#include <cstddef>


namespace globals {

	inline std::uintptr_t clientAddr = 0;
	inline std::uintptr_t engineAddr = 0;

	/* Feature */
	inline bool isRadar = false;
	inline bool isAimbot = false;
	inline bool isTriggerbot = false;
	inline bool isBunnyHop = false;
	inline bool isRecoil = false;
	inline bool isEsp = false;
	inline bool isEspGlow = false;
	inline float glowColor[] = { 1.f, 0.f, 0.f, 1.f };
	
}