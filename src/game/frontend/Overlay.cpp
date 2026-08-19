#include "Overlay.hpp"
#include "Menu.hpp"
#include "core/commands/Commands.hpp"
#include "core/commands/BoolCommand.hpp"
#include "game/pointers/Pointers.hpp"
#include "game/gta/invoker/Invoker.hpp"
#include "game/gta/Natives.hpp"

#include <algorithm>
#include <string>

namespace YimMenu::Features
{
	BoolCommand _OverlayEnabled("overlay", "Overlay Enabled", "Show an info overlay at the top left corner of the screen");
	BoolCommand _OverlayShowFPS("overlayfps", "Overlay Show FPS", "Show frame rate in the info overlay");

	// state shared with Autopilot.cpp
	extern bool g_AutoPilotActive;
	extern float g_AutoPilotRemaining;
	extern int g_SpeedModeState;
	extern int g_DriveStyleState;
	extern bool g_WeatherSlowActive;
	extern bool g_NightModeActive;
	extern bool g_EscapeActive;
}

namespace YimMenu
{
	void Overlay::Draw()
	{
		if (!NativeInvoker::AreHandlersCached())
			return;

		ImGui::SetNextWindowSize(ImVec2(*Pointers.ScreenResX - 10.0f, *Pointers.ScreenResY - 10.0f), ImGuiCond_Always);
		ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f), ImGuiCond_Always);
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		ImGui::PushFont(Menu::Font::g_OverlayFont);

		ImGui::Begin("##overlay", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs);

		if (Features::_OverlayEnabled.GetState())
		{
			if (Features::_OverlayShowFPS.GetState())
				ImGui::Text("FPS: %d", (int)(ImGui::GetIO().Framerate));
		}

		// ------------------------------------------------------------
		// Autopilot HUD (independent of the "overlay" toggle)
		// ------------------------------------------------------------
		if (Commands::GetCommand<BoolCommand>("autopilothud"_J)->GetState())
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.29f, 0.87f, 1.0f, 1.0f));

			if (Features::g_AutoPilotActive)
			{
				const char* speedNames[] = {"Eco", "Comfort", "Sport"};
				const char* styleNames[] = {"Normal", "Işıkları Geç", "Agresif"};

				ImGui::Text("[Oto Pilot] Sürüyor");
				ImGui::Text("  Hız: %s  |  Stil: %s", speedNames[std::clamp(Features::g_SpeedModeState, 0, 2)], styleNames[std::clamp(Features::g_DriveStyleState, 0, 2)]);

				if (Features::g_AutoPilotRemaining >= 0.0f)
					ImGui::Text("  Mesafe: %.0f m", Features::g_AutoPilotRemaining);

				// active environment modes
				std::string activeModes;
				activeModes.reserve(24);
				if (Features::g_WeatherSlowActive)
					activeModes += "Hava ";
				if (Features::g_NightModeActive)
					activeModes += "Gece ";
				if (Features::g_EscapeActive)
					activeModes += "Kaçış ";

				if (!activeModes.empty())
					ImGui::Text("  Modlar: %s", activeModes.c_str());
			}

			ImGui::PopStyleColor();
		}

		ImGui::PopFont();
		ImGui::PopStyleColor();
		ImGui::End();
	}
}