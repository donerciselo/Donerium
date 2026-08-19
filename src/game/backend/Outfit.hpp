#pragma once
#include "core/filemgr/FileMgr.hpp"
#include <unordered_map>

namespace YimMenu
{
	namespace Outfit
	{
		struct ComponentData
		{
			std::string label;
			int drawable_id = 0;
			int texture_id = 0;
			int palette_var = -1;
			int drawable_id_max = 0;
			int texture_id_max = 0;
		};

		struct OutfitComponents
		{
			std::unordered_map<int, ComponentData> items = {
			    {1, {"Maske"}},
			    {2, {"Saç"}},
			    {3, {"Gövde"}},
			    {4, {"Bacak"}},
			    {5, {"Çanta"}},
			    {6, {"Ayakkabı"}},
			    {7, {"Aksesuar"}},
			    {8, {"Atlet"}},
			    {9, {"Kevlar/Zırh"}},
			    {10, {"Dekal"}},
			    {11, {"Üstler"}}};
		};

		struct OutfitProps
		{
			std::unordered_map<int, ComponentData> items = {
			    {0, {"Şapkalar"}},
			    {1, {"Gözlükler"}},
			    {2, {"Küpeler"}},
			    {6, {"Saatler"}},
			    {7, {"Bileklikler"}}};
		};

        class OutfitEditor {
            
            public: 

            static void SetSelfOutfit(OutfitComponents components, OutfitProps props, bool applyHair);

            // json
            static Folder CheckFolder(std::string folderName = "");
            static void ApplyOutfitFromJson(std::string folderName, std::string fileName, bool applyHair);
            static void SaveOutfit(std::string fileName, std::string folder);
			static void RefreshList(std::string folderName, std::vector<std::string>& folders, std::vector<std::string>& files);

            // check & fix bounds
            static void CheckBoundsDrawable(ComponentData& item, const int lower);
            static void CheckBoundsTexture(ComponentData& item, const int lower);
        };
	}
}