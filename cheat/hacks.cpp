#include "hacks.h"
#include "globals.h"
#include "gui.h"
#include "offset.h"
#include "vector.h"

#include <thread>

constexpr Vector3 CalculateAngle(
	const Vector3& localPosition,
	const Vector3& enemyPosition,
	const Vector3& viewAngles) noexcept
{
	return ((enemyPosition - localPosition).ToAngle() - viewAngles);
}

void hacks::VisualThread(const Memory& mem) noexcept
{
	/* Global used */
	const auto& localPlayer = mem.Read<std::uintptr_t>(globals::clientAddr + offsets::dwLocalPlayer);
	const auto& teamNum = mem.Read<std::uintptr_t>(globals::clientAddr + offsets::m_iTeamNum);
	const auto& clientState = mem.Read<std::uintptr_t>(globals::engineAddr + offsets::dwClientState);

	/* Vector for recoil */
	auto oldpunch = Vector2 { };

	while (gui::isRunning) {

		/* No Recoil */
		if (globals::isRecoil) {
			
			const auto& shotfired = mem.Read<std::int32_t>(localPlayer + offsets::m_iShotsFired);

			if (shotfired) {

				const auto& recoilClientstateviewangles = mem.Read<Vector2>(clientState + offsets::dwClientState_ViewAngles);
				const auto& punch = mem.Read<Vector2>(localPlayer + offsets::m_aimPunchAngle);

				auto newAngle = Vector2
				{
					recoilClientstateviewangles.x + oldpunch.x - punch.x * 2.f,
					recoilClientstateviewangles.y + oldpunch.y - punch.y * 2.f
				};

				if (newAngle.x > 89.f)
					newAngle.x = 89.f;

				if (newAngle.x < -89.f)
					newAngle.x = -89.f;

				while (newAngle.y > 180.f)
					newAngle.y -= 360.f;

				while (newAngle.y < -180.f)
					newAngle.y += 360.f;

				mem.Write<Vector2>(clientState + offsets::dwClientState_ViewAngles, newAngle);

				oldpunch.x = punch.x * 2.f;
				oldpunch.y = punch.y * 2.f;
			}
			else {
				oldpunch.x = oldpunch.y = 0.f;
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(1));

		}
		

		if (globals::isRadar) {
			
			/* Radar Hacks */
			for (auto i = 1; i <= 64; ++i)
			{

				const auto& radarEntity = mem.Read<std::uintptr_t>(globals::clientAddr + offsets::dwEntityList + (i * 0x10));

				if (mem.Read<std::uintptr_t>(radarEntity + offsets::m_iTeamNum) == teamNum)
					continue;

				mem.Write<bool>(radarEntity + offsets::m_bSpotted, true);

			}

			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}


		if (globals::isAimbot) {
			
			/* Aimbot */
			if (WM_MOUSEMOVE)
			{
				
				if (!localPlayer)
					continue;
				
				/* Eyes position */
				const auto& localEyePosition = mem.Read<Vector3>(localPlayer + offsets::m_vecOrigin) +
					mem.Read<Vector3>(localPlayer + offsets::m_vecViewOffset);

				const auto localPlayerId =
					mem.Read<std::int32_t>(clientState + offsets::dwClientState_GetLocalPlayer);

				const auto& viewAngles = mem.Read<Vector3>(clientState + offsets::dwClientState_ViewAngles);
				const auto& aimPunch = mem.Read<Vector3>(clientState + offsets::m_aimPunchAngle) * 2;

				/* AIMBOT FOV */
				auto bestFov = 5.f;
				auto bestAngle = Vector3{ };

				for (auto i = 1; i <= 32; ++i)
				{
					const auto player = mem.Read<std::uintptr_t>(globals::clientAddr + offsets::dwEntityList + i * 0x10);

					if (mem.Read<std::int32_t>(player + offsets::m_iTeamNum) == teamNum)
						continue;

					if (mem.Read<bool>(player + offsets::m_bDormant))
						continue;

					if (mem.Read<std::int32_t>(player + offsets::m_lifeState))
						continue;

					if (mem.Read<std::int32_t>(player + offsets::m_bSpottedByMask) & (1 << localPlayerId))
					{
						const auto boneMatrix = mem.Read<std::uintptr_t>(player + offsets::m_dwBoneMatrix);

						const auto playerHeadPosition = Vector3{
							mem.Read<float>(boneMatrix + 0x30 * 8 + 0x0C),
							mem.Read<float>(boneMatrix + 0x30 * 8 + 0x1C),
							mem.Read<float>(boneMatrix + 0x30 * 8 + 0x2C)
						};

						const auto angle = CalculateAngle(
							localEyePosition,
							playerHeadPosition,
							viewAngles + aimPunch
						);

						const auto fov = std::hypot(angle.x, angle.y);

						if (fov < bestFov)
						{
							bestFov = fov;
							bestAngle = angle;
						}

						if (globals::isTriggerbot) {
							
							/* Triggerbot */
							const auto& localhealth = mem.Read<std::int32_t>(localPlayer + offsets::m_iHealth);

							if (!localhealth)
								continue;

							const auto& crosshairid = mem.Read<std::int32_t>(localPlayer + offsets::m_iCrosshairId);

							if (!crosshairid || crosshairid > 64)
								continue;

							const auto& player = mem.Read<std::uintptr_t>(globals::clientAddr + offsets::dwEntityList + (crosshairid - 1) * 0x10);

							// skip aim for dead players
							if (!mem.Read<std::int32_t>(player + offsets::m_iHealth))
								continue;

							// skip aim for teammates
							if (mem.Read<std::int32_t>(player + offsets::m_iTeamNum) == mem.Read<std::int32_t>(localPlayer + offsets::m_iTeamNum))
								continue;

							mem.Write<std::uintptr_t>(globals::clientAddr + offsets::dwForceAttack, 6);
							std::this_thread::sleep_for(std::chrono::milliseconds(400));
							mem.Write<std::uintptr_t>(globals::clientAddr + offsets::dwForceAttack, 4);
							
						}

					}
				}

				if (!bestAngle.IsZero())
					mem.Write<Vector3>(clientState + offsets::dwClientState_ViewAngles, viewAngles + bestAngle / 3.f);

			}

			std::this_thread::sleep_for(std::chrono::milliseconds(1));

		}


		/* ESP GLOW */
		if (globals::isEspGlow) {

			const auto& glowObj = mem.Read<std::uintptr_t>(globals::clientAddr + offsets::dwGlowObjectManager);

			for (auto i = 0; i < 64; ++i)
			{
				const auto& EspEntity = mem.Read<std::uintptr_t>(globals::clientAddr + offsets::dwEntityList + (i * 0x10));

				if (mem.Read<std::uintptr_t>(EspEntity + offsets::m_iTeamNum) == mem.Read<std::uintptr_t>(localPlayer + offsets::m_iTeamNum))
					continue;

				const auto glowIndex = mem.Read<std::int32_t>(EspEntity + offsets::m_iGlowIndex);

				mem.Write<float>(glowObj + (glowIndex * 0x38) + 0x8, globals::glowColor[0]);
				mem.Write<float>(glowObj + (glowIndex * 0x38) + 0xC, globals::glowColor[1]);
				mem.Write<float>(glowObj + (glowIndex * 0x38) + 0x10, globals::glowColor[2]);
				mem.Write<float>(glowObj + (glowIndex * 0x38) + 0x14, globals::glowColor[3]);

				mem.Write<bool>(glowObj + (glowIndex * 0x38) + 0x28, true);
				mem.Write<bool>(glowObj + (glowIndex * 0x38) + 0x29, false);

			}

			std::this_thread::sleep_for(std::chrono::milliseconds(1));

		}

		/* Bunnyhop */
		if (globals::isBunnyHop) {
			
			const auto localplayerflags = mem.Read<std::int32_t>(localPlayer + offsets::m_fFlags);
			if (globals::isBunnyHop) {
				if (GetAsyncKeyState(VK_SPACE))
					(localplayerflags & (1 << 0)) ?
					mem.Write<std::uintptr_t>(globals::clientAddr + offsets::dwForceJump, 6) :
					mem.Write<std::uintptr_t>(globals::clientAddr + offsets::dwForceJump, 4);
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			
		}
		
	}
	
}