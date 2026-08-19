#include "Onboarding.hpp"
#include "GUI.hpp"
#include "core/commands/Commands.hpp"
#include "core/commands/BoolCommand.hpp"
#include "game/backend/AnticheatBypass.hpp"
#include "game/pointers/Pointers.hpp"

namespace YimMenu
{
	static BoolCommand _OnboardingComplete{"$onboardingcomplete", "", ""};

	void ProcessOnboarding()
	{
		if (_OnboardingComplete.GetState())
			return;

		static bool ensure_popup_open = [] {
			ImGui::OpenPopup("ÖNEMLİ! LÜTFEN OKU!");
			GUI::SetOnboarding(true);
			return true;
		}();

		const auto window_size = ImVec2{700, 500};
		const auto window_position = ImVec2{(*Pointers.ScreenResX - window_size.x) / 2, (*Pointers.ScreenResY - window_size.y) / 2};

		ImGui::SetNextWindowSize(window_size, ImGuiCond_Once);
		ImGui::SetNextWindowPos(window_position, ImGuiCond_Once);

		if (ImGui::BeginPopupModal("ÖNEMLİ! LÜTFEN OKU!", nullptr, ImGuiWindowFlags_NoSavedSettings))
		{
			ImGui::TextWrapped("%s",
			    "Donerium'a hoş geldin! Menüyü açmak için INSERT veya Ctrl+\\ tuşlarına basabilirsin. BattlEye'nin gelmesiyle birlikte herkese açık oturumlara "
			    "katılıp orada kalma yeteneği ciddi şekilde kısıtlandı. "
			    "İstersen yalnızca kendi topluluğumuzla oynamayı seçebilir, istersen de normal BattlEye korumalı oturumlara bağlanmayı deneyebilirsin. "
			    "Normal oturumlardan üç dakikadan kısa sürede otomatik olarak atılacaksın ve BattlEye'yi yeniden açsan bile "
			    "iki güne kadar oturumlara katılmaktan geçici olarak yasaklanabilirsin");
			static int value = 0;
			ImGui::RadioButton("Kendi topluluğumuzla oyna", &value, 0);
			ImGui::SameLine();
			ImGui::RadioButton("Herkesle oyna (Bozuk!)", &value, 1);
			ImGui::TextWrapped("%s",
			    "Seçimini her zaman Ağ > Sahtecilik > Sadece Kendi Topluluğumuzdan Oturumlara Katıl seçeneğini değiştirerek güncelleyebilirsin. "
			    "Etrafta oyna ve Donerium ile eğlen!");
			if (ImGui::Button("Başla"))
			{
				Commands::GetCommand<BoolCommand>("cheaterpool"_J)->SetState(!value);
				_OnboardingComplete.SetState(true);
				GUI::SetOnboarding(false);
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}
}