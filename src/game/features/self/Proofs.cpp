#include "core/commands/LoopedCommand.hpp"
#include "core/commands/BoolCommand.hpp"
#include "game/backend/Self.hpp"
#include "game/gta/Natives.hpp"

namespace YimMenu::Features
{
	static BoolCommand _BulletProof{"bulletproof", "Kurşun Direnci", "Kurşun hasarını engeller"};
	static BoolCommand _FireProof{"fireproof", "Ateş Direnci", "Ateş hasarını engeller"};
	static BoolCommand _ExplosionProof{"explosionproof", "Patlama Direnci", "Patlama hasarını engeller"};
	static BoolCommand _CollisionProof{"collisionproof", "Çarpışma Direnci", "Çarpışma hasarını engeller"};
	static BoolCommand _MeleeProof{"meleeproof", "Yakın Dövüş Direnci", "Yakın dövüş hasarını engeller"};
	static BoolCommand _WaterProof{"waterproof", "Su Direnci", "Su ile hasar almayı engeller"};

	class Proofs : public LoopedCommand
	{
		using LoopedCommand::LoopedCommand;

	public:
		virtual void OnTick() override
		{
			auto ped = Self::GetPed();
			if (!ped)
				return;

			ENTITY::SET_ENTITY_PROOFS(ped.GetHandle(), _BulletProof.GetState(), _FireProof.GetState(), _ExplosionProof.GetState(), _CollisionProof.GetState(),
			    _MeleeProof.GetState(), false, true, _WaterProof.GetState());
		}

		virtual void OnDisable() override
		{
			if (auto ped = Self::GetPed())
				ENTITY::SET_ENTITY_PROOFS(ped.GetHandle(), false, false, false, false, false, false, true, false);
		}
	};

	static Proofs _Proofs{"proofs", "Hasar Dirençleri", "Tüm hasar dirençlerini yönetir"};
}