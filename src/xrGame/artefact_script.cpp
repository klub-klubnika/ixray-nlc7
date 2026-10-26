#include "StdAfx.h"
#include "pch_script.h"
#include "Artefact.h"
#include "MercuryBall.h"
#include "GraviArtifact.h"
#include "BastArtifact.h"
#include "BlackGraviArtifact.h"
#include "ElectricBall.h"
#include "cta_game_artefact.h"

using namespace luabind;

namespace ArtefactScript
{
	float GetHealth(CArtefact* Artefact) { return Artefact->GetHealthPower(); }
	void SetHealth(CArtefact* Artefact, float Value) { Artefact->SetHealthPower(Value); }
	float GetRadiation(CArtefact* Artefact) { return Artefact->GetRadiationPower(); }
	void SetRadiation(CArtefact* Artefact, float Value) { Artefact->SetRadiationPower(Value); }
	float GetSatiety(CArtefact* Artefact) { return Artefact->GetSatietyPower(); }
	void SetSatiety(CArtefact* Artefact, float Value) { Artefact->SetSatietyPower(Value); }
	float GetPower(CArtefact* Artefact) { return Artefact->GetPowerPower(); }
	void SetPower(CArtefact* Artefact, float Value) { Artefact->SetPowerPower(Value); }
	float GetBleeding(CArtefact* Artefact) { return Artefact->GetBleedingPower(); }
	void SetBleeding(CArtefact* Artefact, float Value) { Artefact->SetBleedingPower(Value); }
	float GetAdditionalWeight(CArtefact* Artefact) { return Artefact->AdditionalInventoryWeight(); }
}

#pragma optimize("s",on)
void CArtefact::script_register(lua_State *L)
{
	module(L)
	[
		class_<CArtefact, bases<CGameObject, CInventoryItem>>("CArtefact")
			.def(constructor<>())
			.property("health_restore_speed", &ArtefactScript::GetHealth, &ArtefactScript::SetHealth)
			.property("radiation_restore_speed", &ArtefactScript::GetRadiation, &ArtefactScript::SetRadiation)
			.property("satiety_restore_speed", &ArtefactScript::GetSatiety, &ArtefactScript::SetSatiety)
			.property("power_restore_speed", &ArtefactScript::GetPower, &ArtefactScript::SetPower)
			.property("bleeding_restore_speed", &ArtefactScript::GetBleeding, &ArtefactScript::SetBleeding)
			.property("additional_inventory_weight", &ArtefactScript::GetAdditionalWeight)
			.def("FollowByPath", &CArtefact::FollowByPath)
			.def("SwitchVisibility", &CArtefact::SwitchVisibility)
			.def("GetAfRank", &CArtefact::GetAfRank)
			.def("GetJumpHeightModifier", &CArtefact::GetJumpHeightModifier)
			.def("SetJumpHeightModifier", &CArtefact::SetJumpHeightModifier)
			.def("GetMovementSpeedModifier", &CArtefact::GetMovementSpeedModifier)
			.def("SetMovementSpeedModifier", &CArtefact::SetMovementSpeedModifier)
			.def("GetEquipmentDurabilityModifier", &CArtefact::GetEquipmentDurabilityModifier)
			.def("GetInventoryWeightModifier", &CArtefact::GetInventoryWeightModifier)
			.def("SetInventoryWeightModifier", &CArtefact::SetInventoryWeightModifier)
			.def("SetEquipmentDurabilityModifier", &CArtefact::SetEquipmentDurabilityModifier),

		class_<CMercuryBall			,CArtefact>("CMercuryBall").def(constructor<>()),
		class_<CBlackGraviArtefact	,CArtefact>("CBlackGraviArtefact").def(constructor<>()),
		class_<CBastArtefact		,CArtefact>("CBastArtefact").def(constructor<>()),
		class_<CElectricBall		,CArtefact>("CElectricBall").def(constructor<>()),
		class_<CGraviArtefact		,CArtefact>("CGraviArtefact").def(constructor<>())
	];

	luabind::object artefact_class = luabind::get_globals(L)["CArtefact"];

	auto MakeAliasLambda = [&artefact_class, L](const char* TypeID)
	{
		lua_pushstring(L, TypeID);
		artefact_class.pushvalue();
		lua_settable(L, LUA_GLOBALSINDEX);
	};

	MakeAliasLambda("CDummyArtefact");
	MakeAliasLambda("CZudaArtefact");
	MakeAliasLambda("CThornArtefact");
	MakeAliasLambda("CRustyHairArtefact");
	MakeAliasLambda("CFadedBall");
	MakeAliasLambda("CBlackDrops");
	MakeAliasLambda("CGalantineArtefact");
}
