#include "game/pointers/Pointers.hpp"
#include "game/frontend/Menu.hpp"
#include "core/frontend/manager/UIManager.hpp"
#include "game/frontend/submenus/Settings/GUISettings.hpp"

namespace YimMenu
{
	void RenderClassicTheme()
	{
		YimMenu::SyncColorCommandsToStyle();

		float windowWidth = *YimMenu::Pointers.ScreenResX / 2.5f;
		float centerX = (*YimMenu::Pointers.ScreenResX - windowWidth) / 2.0f;
		float centerY = *YimMenu::Pointers.ScreenResY / 5.0f;
		ImVec2 windowSize(windowWidth, *YimMenu::Pointers.ScreenResY / 2.5f);

		ImGui::SetNextWindowSize(windowSize, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ImVec2(centerX, centerY), ImGuiCond_FirstUseEver);

		ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse;
		auto pos = ImGui::GetCursorPos();
		if(ImGui::Begin("##ClassicInputWindow", nullptr, flags))
		{
			if (ImGui::BeginChild("##submenus", ImVec2(120, ImGui::GetContentRegionAvail().y - 20), true, ImGuiWindowFlags_NoTitleBar))
			{
				const auto& submenus = YimMenu::UIManager::GetSubmenus();
				auto activeSubmenu = YimMenu::UIManager::GetActiveSubmenu();

				for (auto& submenu : submenus)
				{
					if (ImGui::Selectable(submenu->m_Name.data(), (submenu == activeSubmenu)))
					{
						YimMenu::UIManager::SetActiveSubmenu(submenu);
						YimMenu::UIManager::SetShowContentWindow(true);
					}
				}
			}
			ImGui::EndChild();

			ImGui::PushFont(Menu::Font::g_AwesomeFont);
			ImGui::TextColored(ImVec4(0.30f, 0.78f, 1.00f, 1.0f), "\xef\x8b\xb1");
			ImGui::PopFont();
			ImGui::SameLine();
			ImGui::PushFont(Menu::Font::g_DefaultFont);
			ImGui::TextColored(ImVec4(0.55f, 0.85f, 1.00f, 1.00f), "DONERIUM");
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(0.42f, 0.50f, 0.62f, 1.00f), "  |  GTA V ENHANCED");
			ImGui::PopFont();

			pos.y -= 28;
			ImGui::SetCursorPos(ImVec2(pos.x + 130, pos.y));

			if (ImGui::BeginChild("##minisubmenus", ImVec2(0, 50), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
			{
				auto activeSubmenu = YimMenu::UIManager::GetActiveSubmenu();
				if (activeSubmenu)
					activeSubmenu->DrawCategorySelectors();
			}
			ImGui::EndChild();

			ImGui::SetCursorPos(ImVec2(pos.x + 130, pos.y + 60));

			if (ImGui::BeginChild("##options", ImVec2(0, 0), true))
			{
				auto optionsFont = YimMenu::UIManager::GetOptionsFont();
				if (optionsFont)
					ImGui::PushFont(optionsFont);

				auto activeSubmenu = YimMenu::UIManager::GetActiveSubmenu();
				if (activeSubmenu)
					activeSubmenu->Draw();

				if (optionsFont)
					ImGui::PopFont();
			}
			ImGui::EndChild();

			// Donerium brand footer
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.28f, 0.47f, 0.66f, 1.00f));
			ImGui::Separator();
			ImGui::TextColored(ImVec4(0.35f, 0.56f, 0.78f, 1.00f), "DONERIUM %s", "v0.15.61");
			ImGui::PopStyleColor();
		}
		ImGui::End();
	}
}
