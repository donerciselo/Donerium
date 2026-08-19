#include "core/commands/LoopedCommand.hpp"
#include "core/commands/FloatCommand.hpp"
#include "game/backend/Self.hpp"
#include "game/gta/Natives.hpp"

namespace YimMenu::Features
{
	static FloatCommand _VehicleDensity{"trafficdensity", "Araç Yoğunluğu", "Trafikteki araç yoğunluğunu ayarlar", 0.0f, 2.0f, 1.0f};
	static FloatCommand _PedDensity{"peddensity", "Yaya Yoğunluğu", "Sokaktaki yaya yoğunluğunu ayarlar", 0.0f, 2.0f, 1.0f};

	class TrafficDensityController : public LoopedCommand
	{
		using LoopedCommand::LoopedCommand;

	public:
		virtual void OnTick() override
		{
			VEHICLE::SET_VEHICLE_DENSITY_MULTIPLIER_THIS_FRAME(_VehicleDensity.GetState());
			VEHICLE::SET_RANDOM_VEHICLE_DENSITY_MULTIPLIER_THIS_FRAME(_VehicleDensity.GetState());
			PED::SET_PED_DENSITY_MULTIPLIER_THIS_FRAME(_PedDensity.GetState());
		}
	};

	static TrafficDensityController _TrafficDensity{"trafficcontroller", "Trafik Kontrolü", "Araç ve yaya yoğunluğunu her karede uygular"};
}