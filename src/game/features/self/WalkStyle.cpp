#include "core/commands/LoopedCommand.hpp"
#include "core/commands/ListCommand.hpp"
#include "game/backend/Self.hpp"
#include "game/gta/Natives.hpp"

namespace YimMenu::Features
{
	static const std::vector<std::pair<int, const char*>> g_WalkStyles = {
	    {0, "Çok Sarhoş"},
	    {1, "Hafif Sarhoş"},
	    {2, "Soylu"},
	    {3, "Gangster A"},
	    {4, "Gangster B"},
	    {5, "Gangster C"},
	    {6, "Hobo"},
	    {7, "Yaralı"},
	    {8, "Hızlı"},
	    {9, "Kelepçeli"},
	    {10, "Şişman"},
	    {11, "Sert Adam"},
	};

	static ListCommand _WalkStyleSelector{"walkstyleselector", "Stil Seç", "Yürüyüş stilini seç", g_WalkStyles};

	class WalkStyle : public LoopedCommand
	{
		using LoopedCommand::LoopedCommand;

	public:
		virtual void OnTick() override
		{
			auto ped = Self::GetPed();
			if (!ped)
				return;

			auto clipset = g_WalkStyles[_WalkStyleSelector.GetState()].second;
			STREAMING::REQUEST_ANIM_SET(clipset);
			if (STREAMING::HAS_ANIM_SET_LOADED(clipset))
				PED::SET_PED_MOVEMENT_CLIPSET(ped.GetHandle(), clipset, 0.5f);
		}

		virtual void OnDisable() override
		{
			if (auto ped = Self::GetPed())
				PED::RESET_PED_MOVEMENT_CLIPSET(ped.GetHandle(), 0.0f);
		}
	};

	static WalkStyle _WalkStyle{"walkstyle", "Yürüyüş Stili", "Özel bir yürüyüş stiliyle hareket et"};
}