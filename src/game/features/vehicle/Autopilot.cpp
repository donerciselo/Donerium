#include "core/commands/LoopedCommand.hpp"
#include "core/commands/ListCommand.hpp"
#include "core/commands/Commands.hpp"
#include "core/backend/FiberPool.hpp"
#include "core/util/Joaat.hpp"
#include "core/frontend/Notifications.hpp"
#include "game/backend/Self.hpp"
#include "game/gta/Natives.hpp"
#include "game/gta/Ped.hpp"
#include "game/gta/Vehicle.hpp"
#include "types/ped/PedCombatAttribute.hpp"

#include <cmath>
#include <string>
#include <vector>

namespace YimMenu::Features
{
	// HUD / other code can read our state via these globals
	bool g_AutoPilotActive = false;
	float g_AutoPilotRemaining = -1.0f;
	int g_SpeedModeState = 1; // default Comfort
	int g_DriveStyleState = 0; // default Normal
	bool g_WeatherSlowActive = false;
	bool g_NightModeActive = false;
	bool g_EscapeActive = false;

	class Autopilot : public LoopedCommand
	{
		using LoopedCommand::LoopedCommand;

	private:
		Ped m_Driver{nullptr};

		rage::fvector3 m_Target{0.0f, 0.0f, 0.0f};

		bool m_Moving = false;

		// Stuck / progress tracking
		int m_StuckTicks = 0;
		int m_ProgressTicks = 0;

		// Last drive mode issued (so we can re-issue on style change)
		int m_LastDriveMode = -1;

		rage::fvector3 m_LastPosition{0.0f, 0.0f, 0.0f};
		float m_LastDistance = -1.0f;

		// ------------------------------------------------------------
		// Driving modes & speed presets
		// ------------------------------------------------------------
		static constexpr int k_DriveModeNormal   = 786603;      // 0x000C000B "Normal"
		static constexpr int k_DriveModeNoLights = 2883621;     // "Ignore Lights"
		static constexpr int k_DriveModeAggro    = 1074528293;  // "Rushed"

		static constexpr float k_SpeedEco     = 27.78f; // m/s (~100 km/h)
		static constexpr float k_SpeedComfort = 41.67f; // m/s (~150 km/h)
		static constexpr float k_SpeedSport   = 250.0f; // m/s — effectively unlimited, let the vehicle reach its max

		static constexpr float k_StopRange = 5.0f;

		// Vehicle considered stuck if it barely moves for this many ticks.
		static constexpr int k_StuckThreshold = 180;

		// Minimum distance change required to count as progress.
		static constexpr float k_MinProgressDistance = 1.0f;

		// ------------------------------------------------------------
		// Current config (updated every tick from the global commands)
		// ------------------------------------------------------------
		int GetSelectedSpeedMode()
		{
			return g_SpeedModeState;
		}

		int GetSelectedDriveStyle()
		{
			return g_DriveStyleState;
		}

		int GetDriveMode(int style)
		{
			switch (style)
			{
			case 1: return k_DriveModeNoLights;
			case 2: return k_DriveModeAggro;
			default: return k_DriveModeNormal;
			}
		}

		float GetBaseSpeed(int mode)
		{
			switch (mode)
			{
			case 0: return k_SpeedEco;
			case 2: return k_SpeedSport;
			default: return k_SpeedComfort;
			}
		}

		// ------------------------------------------------------------
		// Environment-aware speed calculation
		// ------------------------------------------------------------
		bool IsRainingOrSnowing()
		{
			Hash weather = MISC::GET_PREV_WEATHER_TYPE_HASH_NAME();
			return weather == "RAIN"_J || weather == "THUNDER"_J || weather == "SNOW"_J || weather == "BLIZZARD"_J || weather == "SNOWLIGHT"_J;
		}

		bool IsNightTime()
		{
			int hour = CLOCK::GET_CLOCK_HOURS();
			return hour >= 20 || hour < 6;
		}

		int GetWantedLevel()
		{
			return PLAYER::GET_PLAYER_WANTED_LEVEL(PLAYER::PLAYER_ID());
		}

		// Returns the final cruise speed for this tick (m/s)
		float ComputeDriveSpeed(Vehicle veh)
		{
			const bool escaping = g_EscapeActive && GetWantedLevel() > 0;

			float speed = GetBaseSpeed(GetSelectedSpeedMode());

			// environment slowdowns do not apply while escaping — we want to get away fast
			if (g_WeatherSlowActive && IsRainingOrSnowing() && !escaping)
				speed *= 0.7f;

			if (g_NightModeActive && IsNightTime() && !escaping)
				speed *= 0.8f;

			// escape mode: go flat out
			if (escaping)
			{
				speed = std::max(speed, k_SpeedSport);
			}

			// never exceed sport top speed
			if (speed > k_SpeedSport)
				speed = k_SpeedSport;

			return speed;
		}

		// Aggressive driving mode is mandatory while escaping the police,
		// regardless of which style the player picked.
		int GetEffectiveDriveMode()
		{
			if (g_EscapeActive && GetWantedLevel() > 0)
				return k_DriveModeAggro;

			return GetDriveMode(GetSelectedDriveStyle());
		}

		// ------------------------------------------------------------
		// Resolve waypoint Z
		// ------------------------------------------------------------
		void ResolveZCoordinate(rage::fvector3& vec)
		{
			if (vec.z == 0.0f)
			{
				vec.z = PATH::GET_APPROX_HEIGHT_FOR_POINT(vec.x, vec.y);
			}
		}

		// ------------------------------------------------------------
		// Move player back into driver's seat
		// ------------------------------------------------------------
		void MovePlayerBackToDriverSeat()
		{
			if (auto veh = Self::GetVehicle())
			{
				Self::GetPed().SetInVehicle(veh, -1);
			}
		}

		// ------------------------------------------------------------
		// Reset progress tracking
		// ------------------------------------------------------------
		void ResetProgressTracking()
		{
			m_StuckTicks = 0;
			m_ProgressTicks = 0;
			m_LastDistance = -1.0f;
			m_LastPosition = rage::fvector3{0.0f, 0.0f, 0.0f};
			m_LastDriveMode = -1;
		}

		// ------------------------------------------------------------
		// Issue driving task
		// ------------------------------------------------------------
		void AssignDriveTask(Vehicle veh)
		{
			if (!m_Driver || !veh)
				return;

			// Keep the vehicle engine running
			VEHICLE::SET_VEHICLE_ENGINE_ON(
				veh.GetHandle(),
				true,
				true,
				false
			);

			// While escaping, protect the vehicle so we can actually get away
			if (g_EscapeActive && GetWantedLevel() > 0)
			{
				VEHICLE::SET_VEHICLE_TYRES_CAN_BURST(veh.GetHandle(), false);
				if (VEHICLE::GET_VEHICLE_ENGINE_HEALTH(veh.GetHandle()) < 1000.0f)
					VEHICLE::SET_VEHICLE_ENGINE_HEALTH(veh.GetHandle(), 1000.0f);
			}

			TASK::TASK_VEHICLE_DRIVE_TO_COORD_LONGRANGE(
				m_Driver.GetHandle(),
				veh.GetHandle(),
				m_Target.x,
				m_Target.y,
				m_Target.z,
				ComputeDriveSpeed(veh),
				GetEffectiveDriveMode(),
				k_StopRange
			);
		}

		// ------------------------------------------------------------
		// Cleanup
		// ------------------------------------------------------------
		void CleanupAndDisable(std::string_view message, NotificationType type)
		{
			FiberPool::Push([this, message = std::string(message), type] {
				if (m_Driver)
				{
					m_Driver.Delete();
					m_Driver = nullptr;
				}

				m_Moving = false;
				ResetProgressTracking();

				MovePlayerBackToDriverSeat();

				g_AutoPilotActive = false;

				Notifications::Show(
					"Oto Pilot",
					message,
					type
				);

				SetState(false);
			});
		}

		// ------------------------------------------------------------
		// Create invisible AI driver
		// ------------------------------------------------------------
		bool CreateDriver()
		{
			auto veh = Self::GetVehicle();
			if (!veh)
				return false;

			auto driver = Ped::Create(
				"a_m_m_beach_01"_J,
				veh.GetPosition()
			);

			if (!driver)
				return false;

			// Invisible locally
			driver.SetVisible(false);
			NETWORK::SET_ENTITY_LOCALLY_INVISIBLE(driver.GetHandle());

			// Keep entity alive / controlled by script
			ENTITY::SET_ENTITY_AS_MISSION_ENTITY(
				driver.GetHandle(),
				true,
				true
			);

			// --------------------------------------------------------
			// Prevent random AI events / fleeing / vehicle stealing
			// --------------------------------------------------------
			PED::SET_BLOCKING_OF_NON_TEMPORARY_EVENTS(
				driver.GetHandle(),
				true
			);

			PED::SET_PED_FLEE_ATTRIBUTES(
				driver.GetHandle(),
				0,
				false
			);

			PED::SET_PED_COMBAT_ATTRIBUTES(
				driver.GetHandle(),
				(int)PedCombatAttribute::AlwaysFlee,
				false
			);

			PED::SET_PED_COMBAT_ATTRIBUTES(
				driver.GetHandle(),
				(int)PedCombatAttribute::CanCommandeerVehicles,
				false
			);

			// Common vehicle-related config flag
			PED::SET_PED_CONFIG_FLAG(
				driver.GetHandle(),
				281,
				true
			);

			PED::SET_PED_CAN_BE_DRAGGED_OUT(
				driver.GetHandle(),
				false
			);

			PED::SET_PED_STAY_IN_VEHICLE_WHEN_JACKED(
				driver.GetHandle(),
				true
			);

			// --------------------------------------------------------
			// Driving behavior
			// --------------------------------------------------------
			PED::SET_DRIVER_ABILITY(
				driver.GetHandle(),
				1.0f
			);

			PED::SET_DRIVER_AGGRESSIVENESS(
				driver.GetHandle(),
				0.0f
			);

			driver.SetKeepTask(true);

			m_Driver = driver;

			return true;
		}

		// ------------------------------------------------------------
		// Start autopilot
		// ------------------------------------------------------------
		void StartAutopilot(const rage::fvector3& coords)
		{
			m_Target = coords;
			ResolveZCoordinate(m_Target);

			if (!CreateDriver())
			{
				m_Moving = false;

				Notifications::Show(
					"Oto Pilot",
					"Sürücü oluşturulamadı!",
					NotificationType::Error
				);

				SetState(false);
				return;
			}

			auto veh = Self::GetVehicle();
			if (!veh)
			{
				m_Driver.Delete();
				m_Driver = nullptr;
				m_Moving = false;
				SetState(false);
				return;
			}

			// --------------------------------------------------------
			// IMPORTANT:
			// Player first goes to passenger seat.
			// Then AI takes driver's seat.
			// --------------------------------------------------------
			Self::GetPed().SetInVehicle(
				veh,
				0
			);

			m_Driver.SetInVehicle(
				veh,
				-1
			);

			VEHICLE::SET_VEHICLE_ENGINE_ON(
				veh.GetHandle(),
				true,
				true,
				false
			);

			// Store initial movement state
			m_LastPosition = veh.GetPosition();

			m_LastDistance = MISC::GET_DISTANCE_BETWEEN_COORDS(
				m_LastPosition.x,
				m_LastPosition.y,
				m_LastPosition.z,
				m_Target.x,
				m_Target.y,
				m_Target.z,
				true
			);

			m_StuckTicks = 0;
			m_ProgressTicks = 0;

			g_AutoPilotActive = true;

			// Give AI the actual driving task.
			AssignDriveTask(veh);
		}

		// ------------------------------------------------------------
		// Check if vehicle is making meaningful progress
		// ------------------------------------------------------------
		void UpdateProgress(Vehicle veh)
		{
			if (!veh)
				return;

			const auto currentPos = veh.GetPosition();

			const float currentDistance =
				MISC::GET_DISTANCE_BETWEEN_COORDS(
					currentPos.x,
					currentPos.y,
					currentPos.z,
					m_Target.x,
					m_Target.y,
					m_Target.z,
					true
				);

			const float movementDistance =
				MISC::GET_DISTANCE_BETWEEN_COORDS(
					currentPos.x,
					currentPos.y,
					currentPos.z,
					m_LastPosition.x,
					m_LastPosition.y,
					m_LastPosition.z,
					true
				);

			// Vehicle actually moved
			if (movementDistance >= k_MinProgressDistance)
			{
				m_ProgressTicks++;
				m_StuckTicks = 0;
			}
			else
			{
				m_ProgressTicks = 0;

				// It might still be moving slowly,
				// so only count as stuck when speed is almost zero.
				if (veh.GetSpeed() < 0.5f)
					m_StuckTicks++;
				else
					m_StuckTicks = 0;
			}

			m_LastPosition = currentPos;

			// If distance meaningfully decreased, consider it progress too.
			if (m_LastDistance < 0.0f ||
				currentDistance + 1.0f < m_LastDistance)
			{
				m_StuckTicks = 0;
			}

			m_LastDistance = currentDistance;
		}

		// ------------------------------------------------------------
		// Re-assign driving task if truly stuck
		// ------------------------------------------------------------
		void RecoverIfStuck(Vehicle veh)
		{
			if (!m_Driver || !veh)
				return;

			if (m_StuckTicks < k_StuckThreshold)
				return;

			VEHICLE::SET_VEHICLE_ENGINE_ON(
				veh.GetHandle(),
				true,
				true,
				false
			);

			AssignDriveTask(veh);

			m_StuckTicks = 0;
			m_ProgressTicks = 0;
		}

	public:
		virtual void OnTick() override
		{
			auto veh = Self::GetVehicle();

			if (!veh || !Self::GetPed())
			{
				if (m_Driver || g_AutoPilotActive)
				{
					CleanupAndDisable(
						"Araçta değilsin!",
						NotificationType::Error
					);
				}

				return;
			}

			// --------------------------------------------------------
			// Waypoint exists
			// --------------------------------------------------------
			if (HUD::IS_WAYPOINT_ACTIVE())
			{
				// ----------------------------------------------------
				// Autopilot already running
				// ----------------------------------------------------
				if (m_Driver)
				{
					// Keep the cruise speed in sync with environment every tick
					TASK::SET_DRIVE_TASK_CRUISE_SPEED(
						m_Driver.GetHandle(),
						ComputeDriveSpeed(veh)
					);

					// Re-issue the task if the effective drive style changed
					// (e.g. police escape turned on/off mid-journey)
					const int currentMode = GetEffectiveDriveMode();
					if (m_LastDriveMode != currentMode)
					{
						AssignDriveTask(veh);
					}
					m_LastDriveMode = currentMode;

					// While escaping, keep the tyres intact and the engine healthy
					if (g_EscapeActive && GetWantedLevel() > 0)
					{
						VEHICLE::SET_VEHICLE_TYRES_CAN_BURST(veh.GetHandle(), false);
						if (VEHICLE::GET_VEHICLE_ENGINE_HEALTH(veh.GetHandle()) < 1000.0f)
							VEHICLE::SET_VEHICLE_ENGINE_HEALTH(veh.GetHandle(), 1000.0f);
					}

					// Night lights
					if (g_NightModeActive)
						VEHICLE::SET_VEHICLE_LIGHTS(veh.GetHandle(), 3);
					else
						VEHICLE::SET_VEHICLE_LIGHTS(veh.GetHandle(), 0);

					// ------------------------------------------------
					// Aggressive mode: signal before changing lanes /
					// overtaking (lateral velocity indicates a lane change)
					// ------------------------------------------------
					const int driveMode = GetEffectiveDriveMode();
					if (driveMode == k_DriveModeAggro && veh.GetSpeed() > 10.0f)
					{
						auto lateral = ENTITY::GET_ENTITY_SPEED_VECTOR(veh.GetHandle(), true);

						if (lateral.y > 1.2f)
							VEHICLE::SET_VEHICLE_INDICATOR_LIGHTS(veh.GetHandle(), 2, true); // right lane change
						else if (lateral.y < -1.2f)
							VEHICLE::SET_VEHICLE_INDICATOR_LIGHTS(veh.GetHandle(), 1, true); // left lane change
						else
						{
							VEHICLE::SET_VEHICLE_INDICATOR_LIGHTS(veh.GetHandle(), 1, false);
							VEHICLE::SET_VEHICLE_INDICATOR_LIGHTS(veh.GetHandle(), 2, false);
						}
					}
					else
					{
						VEHICLE::SET_VEHICLE_INDICATOR_LIGHTS(veh.GetHandle(), 1, false);
						VEHICLE::SET_VEHICLE_INDICATOR_LIGHTS(veh.GetHandle(), 2, false);
					}

					// Driver somehow left vehicle -> put driver back
					if (m_Driver.GetVehicle() != veh)
					{
						m_Driver.SetInVehicle(
							veh,
							-1
						);

						// Re-apply task after putting driver back
						AssignDriveTask(veh);
					}

					// ------------------------------------------------
					// Check distance to target
					// ------------------------------------------------
					const auto vehiclePos = veh.GetPosition();

					const float distance =
						MISC::GET_DISTANCE_BETWEEN_COORDS(
							vehiclePos.x,
							vehiclePos.y,
							vehiclePos.z,
							m_Target.x,
							m_Target.y,
							m_Target.z,
							true
						);

					g_AutoPilotRemaining = distance;

					if (distance <= 8.0f)
					{
						CleanupAndDisable(
							"Hedefe varıldı!",
							NotificationType::Success
						);

						return;
					}

					// ------------------------------------------------
					// Update movement / stuck state
					// ------------------------------------------------
					UpdateProgress(veh);

					// ------------------------------------------------
					// Recover only when genuinely stuck
					// ------------------------------------------------
					RecoverIfStuck(veh);
				}

				// ----------------------------------------------------
				// Start autopilot
				// ----------------------------------------------------
				else if (!m_Moving)
				{
					auto waypointBlip =
						HUD::GET_CLOSEST_BLIP_INFO_ID(
							HUD::GET_WAYPOINT_BLIP_ENUM_ID()
						);

					auto coords =
						HUD::GET_BLIP_COORDS(waypointBlip);

					m_Moving = true;

					rage::fvector3 target{coords.x, coords.y, coords.z};

					FiberPool::Push([this, target] {
						StartAutopilot(target);
					});
				}
			}

			// --------------------------------------------------------
			// Waypoint removed while autopilot is active
			// --------------------------------------------------------
			else if (m_Driver || g_AutoPilotActive)
			{
				CleanupAndDisable(
					"Hedef işareti kaldırıldı!",
					NotificationType::Error
				);
			}
		}

		virtual void OnDisable() override
		{
			m_Moving = false;
			ResetProgressTracking();

			g_AutoPilotActive = false;
			g_AutoPilotRemaining = -1.0f;

			if (m_Driver)
			{
				FiberPool::Push([this] {
					if (m_Driver)
					{
						if (auto veh = Self::GetVehicle())
						{
							VEHICLE::SET_VEHICLE_INDICATOR_LIGHTS(veh.GetHandle(), 1, false);
							VEHICLE::SET_VEHICLE_INDICATOR_LIGHTS(veh.GetHandle(), 2, false);
						}

						m_Driver.Delete();
						m_Driver = nullptr;
					}

					MovePlayerBackToDriverSeat();
				});
			}
		}
	};

	static Autopilot _Autopilot{
		"autopilot",
		"Oto Pilot",
		"İşaretlediğin noktaya otomatik sürüş yaptırır"
	};

	// ------------------------------------------------------------
	// Auto Park: pull the car over to the side of the road
	// ------------------------------------------------------------
	class AutoPark : public LoopedCommand
	{
		using LoopedCommand::LoopedCommand;

	private:
		Ped m_Driver{nullptr};
		rage::fvector3 m_ParkTarget{0.0f, 0.0f, 0.0f};
		bool m_Parking = false;
		int m_WaitTicks = 0;

		constexpr Ped GetPlayerPed()
		{
			return Self::GetPed();
		}

		void Cleanup()
		{
			if (m_Driver)
			{
				m_Driver.Delete();
				m_Driver = nullptr;
			}

			// Put the player back in the driver seat
			if (auto veh = Self::GetVehicle())
				GetPlayerPed().SetInVehicle(veh, -1);

			m_Parking = false;
			m_WaitTicks = 0;
			SetState(false);
		}

	public:
		virtual void OnTick() override
		{
			auto veh = Self::GetVehicle();
			if (!veh || !Self::GetPed())
			{
				Cleanup();
				return;
			}

			// If autopilot is running, stop it first
			if (g_AutoPilotActive)
			{
				Commands::GetCommand<BoolCommand>("autopilot"_J)->SetState(false);
			}

			if (!m_Parking)
			{
				// Find a spot on the right side of the road
				// (approx. 8 meters to the right of the vehicle)
				float heading = veh.GetHeading();
				float rad = (heading - 90.0f) * 3.14159265f / 180.0f;
				auto pos = veh.GetPosition();

				rage::fvector3 side{
				    pos.x + std::cos(rad) * 8.0f,
				    pos.y + std::sin(rad) * 8.0f,
				    pos.z
				};

				// Snap to the closest road node so the car actually parks on the road
				Vector3 nodePos{0.0f, 0.0f, 0.0f};
				if (PATH::GET_CLOSEST_VEHICLE_NODE(side.x, side.y, side.z, &nodePos, 2, 0.0f, 0))
				{
					m_ParkTarget.x = nodePos.x;
					m_ParkTarget.y = nodePos.y;
					m_ParkTarget.z = nodePos.z;
				}
				else
				{
					m_ParkTarget = side;
				}

				if (m_ParkTarget.z == 0.0f)
					m_ParkTarget.z = PATH::GET_APPROX_HEIGHT_FOR_POINT(m_ParkTarget.x, m_ParkTarget.y);

				// Create invisible driver
				auto driver = Ped::Create("a_m_m_beach_01"_J, veh.GetPosition());
				if (!driver)
				{
					Notifications::Show("Oto Park", "Sürücü oluşturulamadı!", NotificationType::Error);
					SetState(false);
					return;
				}

				driver.SetVisible(false);
				NETWORK::SET_ENTITY_LOCALLY_INVISIBLE(driver.GetHandle());
				ENTITY::SET_ENTITY_AS_MISSION_ENTITY(driver.GetHandle(), true, true);
				PED::SET_BLOCKING_OF_NON_TEMPORARY_EVENTS(driver.GetHandle(), true);
				PED::SET_PED_FLEE_ATTRIBUTES(driver.GetHandle(), 0, false);
				PED::SET_PED_COMBAT_ATTRIBUTES(driver.GetHandle(), (int)PedCombatAttribute::AlwaysFlee, false);
				PED::SET_PED_COMBAT_ATTRIBUTES(driver.GetHandle(), (int)PedCombatAttribute::CanCommandeerVehicles, false);
				PED::SET_PED_CONFIG_FLAG(driver.GetHandle(), 281, true);
				PED::SET_PED_CAN_BE_DRAGGED_OUT(driver.GetHandle(), false);
				PED::SET_PED_STAY_IN_VEHICLE_WHEN_JACKED(driver.GetHandle(), true);
				PED::SET_DRIVER_ABILITY(driver.GetHandle(), 1.0f);
				PED::SET_DRIVER_AGGRESSIVENESS(driver.GetHandle(), 0.0f);
				driver.SetKeepTask(true);

				m_Driver = driver;

				// Player to passenger seat, driver to driver seat
				GetPlayerPed().SetInVehicle(veh, 0);
				m_Driver.SetInVehicle(veh, -1);

				VEHICLE::SET_VEHICLE_ENGINE_ON(veh.GetHandle(), true, true, false);

				// Drive slowly to the target spot
				TASK::TASK_VEHICLE_DRIVE_TO_COORD_LONGRANGE(
					m_Driver.GetHandle(),
					veh.GetHandle(),
					m_ParkTarget.x,
					m_ParkTarget.y,
					m_ParkTarget.z,
					3.0f,             // slow crawl
					786603,           // normal mode
					1.0f              // stop near target
				);

				m_Parking = true;
				m_WaitTicks = 0;
			}
			else
			{
				// Check if we've reached the parking spot
				auto pos = veh.GetPosition();

				float dist = MISC::GET_DISTANCE_BETWEEN_COORDS(
					pos.x, pos.y, pos.z,
					m_ParkTarget.x, m_ParkTarget.y, m_ParkTarget.z,
					true
				);

				if (dist < 2.0f || veh.GetSpeed() < 0.2f)
					m_WaitTicks++;
				else
					m_WaitTicks = 0;

				if (m_WaitTicks > 180) // vehicle has stopped for ~3s
				{
					VEHICLE::SET_VEHICLE_ENGINE_ON(veh.GetHandle(), false, true, false);
					Notifications::Show("Oto Park", "Araç park edildi!", NotificationType::Success);
					Cleanup();
				}
			}
		}

		virtual void OnDisable() override
		{
			Cleanup();
		}
	};

	static AutoPark _AutoPark{
		"autopark",
		"Oto Park",
		"Aracı sağdaki boşluğa yavaşça park eder"
	};

	// ------------------------------------------------------------
	// Speed mode selection
	// ------------------------------------------------------------
	static std::vector<std::pair<int, const char*>> g_SpeedModes = {
	    {0, "Eco (100 km/s)"},
	    {1, "Comfort (150 km/s)"},
	    {2, "Sport (Son Hız)"},
	};

	class SpeedMode : public ListCommand
	{
		using ListCommand::ListCommand;

		virtual void OnChange() override
		{
			g_SpeedModeState = GetState();
		}
	};

	static SpeedMode _SpeedMode{
		"autopilotspeedmode",
		"Hız Modu",
		"Oto pilotun hız modunu seç",
		g_SpeedModes,
		1 // Comfort default
	};

	// ------------------------------------------------------------
	// Drive style selection
	// ------------------------------------------------------------
	static std::vector<std::pair<int, const char*>> g_DriveStyles = {
	    {0, "Normal"},          // 786603
	    {1, "Işıkları Geç"},    // 2883621
	    {2, "Agresif"},         // 1074528293
	};

	class DriveStyle : public ListCommand
	{
		using ListCommand::ListCommand;

		virtual void OnChange() override
		{
			g_DriveStyleState = GetState();
		}
	};

	static DriveStyle _DriveStyle{
		"autopilotdrivestyle",
		"Sürüş Stili",
		"Oto pilotun sürüş stilini seç",
		g_DriveStyles,
		0 // Normal default
	};

	// ------------------------------------------------------------
	// Environment toggles
	// ------------------------------------------------------------
	static BoolCommand _WeatherSlow{
		"autopilotweather",
		"Hava Modu",
		"Yağmurda ve karda otomatik yavaşla",
		false
	};

	static BoolCommand _NightMode{
		"autopilotnight",
		"Gece Modu",
		"Gece yavaşla ve farları aç (20:00 - 06:00)",
		false
	};

	static BoolCommand _EscapeMode{
		"autopilotescape",
		"Polis Kaçış",
		"Aranırken otopilot hızlı ve kaçış odaklı sürsün",
		false
	};

	// ------------------------------------------------------------
	// HUD toggle (read by Overlay.cpp)
	// ------------------------------------------------------------
	static BoolCommand _AutoPilotHUD{
		"autopilothud",
		"Oto Pilot HUD",
		"Ekranda oto pilot bilgilerini göster (hız modu, stil, mesafe, çevre modları)",
		false
	};
}