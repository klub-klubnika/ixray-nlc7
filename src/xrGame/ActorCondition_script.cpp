#include "StdAfx.h"
#include "pch_script.h"
#include "ActorCondition.h"
#include "EntityCondition.h"
#include "Wound.h"

using namespace luabind;

class CActorCondition;

void BoosterForEach(CActorCondition* conditions, const luabind::functor<bool> &funct)
{
	CEntityCondition::BOOSTER_MAP& cur_booster_influences = conditions->GetCurBoosterInfluences();
	CEntityCondition::BOOSTER_MAP::const_iterator it = cur_booster_influences.begin();
	CEntityCondition::BOOSTER_MAP::const_iterator it_e = cur_booster_influences.end();
	for (; it != it_e; ++it)
	{
		if (funct((*it).first, (*it).second.fBoostTime, (*it).second.fBoostValue) == true)
			break;
	}
}

bool ApplyBooster_script(CActorCondition* cond, const SBooster& B, const char* sect)
{
	return cond->ApplyBooster(B, sect);
}

void ClearAllBoosters(CActorCondition* conditions)
{
	CEntityCondition::BOOSTER_MAP& cur_booster_influences = conditions->GetCurBoosterInfluences();
	CEntityCondition::BOOSTER_MAP::const_iterator it = cur_booster_influences.begin();
	CEntityCondition::BOOSTER_MAP::const_iterator it_e = cur_booster_influences.end();
	for (; it != it_e; ++it)
	{
		conditions->DisableBoostParameters((*it).second);
	}
	cur_booster_influences.clear();
}

void WoundForEach(CActorCondition* conditions, const luabind::functor<bool> &funct)
{
	auto const& cur_wounds = conditions->wounds();
	auto it = conditions->wounds().begin();
	auto it_e = conditions->wounds().end();
	for (; it != it_e ; ++it)
	{
		if (funct((*it)) == true)
			break;
	}
}

namespace NlcConditionFields
{
	using ChangeValues = std::remove_reference_t<decltype(std::declval<CEntityCondition&>().change_v())>;
	using ChangeField = float ChangeValues::*;

	template <ChangeField Field>
	float GetChange(CEntityCondition* Condition) { return Condition->change_v().*Field; }

	template <ChangeField Field>
	void SetChange(CEntityCondition* Condition, float Value) { Condition->change_v().*Field = Value; }

	float GetHealth(CEntityCondition* Condition) { return Condition->GetHealth(); }
	void SetHealth(CEntityCondition* Condition, float Value) { Condition->SetHealth(Value); }
	float GetMaxHealth(CEntityCondition* Condition) { return Condition->GetMaxHealth(); }
	void SetMaxHealth(CEntityCondition* Condition, float Value) { Condition->max_health() = Value; }
	float GetPower(CEntityCondition* Condition) { return Condition->GetPower(); }
	void SetPower(CEntityCondition* Condition, float Value) { Condition->SetPower(Value); }
	float GetMaxPower(CEntityCondition* Condition) { return Condition->GetMaxPower(); }
	void SetMaxPower(CEntityCondition* Condition, float Value) { Condition->SetMaxPower(Value); }
	float GetRadiation(CEntityCondition* Condition) { return Condition->GetRadiation(); }
	void SetRadiation(CEntityCondition* Condition, float Value) { Condition->radiation() = Value; }
	float GetPsyHealth(CEntityCondition* Condition) { return Condition->GetPsyHealth(); }
	void SetPsyHealth(CEntityCondition* Condition, float Value) { Condition->ChangePsyHealth(Value - Condition->GetPsyHealth()); }

	float GetSatiety(CActorCondition* Condition) { return Condition->Satiety.Current; }
	void SetSatiety(CActorCondition* Condition, float Value) { Condition->Satiety.Current = Value; }
	float GetSatietyV(CActorCondition* Condition) { return Condition->Satiety.Variability; }
	void SetSatietyV(CActorCondition* Condition, float Value) { Condition->Satiety.Variability = Value; }
	float GetSatietyHealthV(CActorCondition* Condition) { return Condition->Satiety.HealthBoost; }
	void SetSatietyHealthV(CActorCondition* Condition, float Value) { Condition->Satiety.HealthBoost = Value; }
	float GetAlcohol(CActorCondition* Condition) { return Condition->Alcohol.Current; }
	void SetAlcohol(CActorCondition* Condition, float Value) { Condition->Alcohol.Current = Value; }
	float GetAlcoholV(CActorCondition* Condition) { return Condition->Alcohol.Variability; }
	void SetAlcoholV(CActorCondition* Condition, float Value) { Condition->Alcohol.Variability = Value; }
	template <ALife::EHitType HitType>
	float GetImmunity(CHitImmunity* Immunity) { return Immunity->GetHitImmunity(HitType); }

	template <ALife::EHitType HitType>
	void SetImmunity(CHitImmunity* Immunity, float Value) { Immunity->SetHitImmunity(HitType, Value); }

	float GetWoundSize(CEntityCondition* Condition, u32 Bone, u32 HitType)
	{
		const auto& Wounds = Condition->wounds();
		return Bone < Wounds.size() ? Wounds[Bone]->TypeSize(ALife::EHitType(HitType)) : 0.0f;
	}

	float GetWoundTotalSize(CEntityCondition* Condition, u32 Bone)
	{
		const auto& Wounds = Condition->wounds();
		return Bone < Wounds.size() ? Wounds[Bone]->TotalSize() : 0.0f;
	}

	bool IsLimping(CActorCondition* Condition) { return Condition->IsLimping(); }
	bool IsCantWalk(CActorCondition* Condition) { return Condition->IsCantWalk(); }
	bool IsCantSprint(CActorCondition* Condition) { return Condition->IsCantSprint(); }
}

#pragma optimize("s",on)
void CActorCondition::script_register(lua_State *L)
{
	module(L)
		[
			class_<SBooster>("SBooster")
			.def(constructor<>())
			.def_readwrite("fBoostTime", &SBooster::fBoostTime)
			.def_readwrite("fBoostValue", &SBooster::fBoostValue)
			.def_readwrite("m_type", &SBooster::m_type)
			,
			class_<CWound>("CWound")
			//.def(constructor<>())
			.def("TypeSize", &CWound::TypeSize)
			.def("BloodSize", &CWound::BloodSize)
			.def("AddHit", &CWound::AddHit)
			.def("Incarnation", &CWound::Incarnation)
			.def("TotalSize", &CWound::TotalSize)
			.def("SetBoneNum", &CWound::SetBoneNum)
			.def("GetBoneNum", &CWound::GetBoneNum)
			.def("GetParticleBoneNum", &CWound::GetParticleBoneNum)
			.def("SetParticleBoneNum", &CWound::SetParticleBoneNum)
			.def("SetDestroy", &CWound::SetDestroy)
			.def("GetDestroy", &CWound::GetDestroy)
			,
			class_<CHitImmunity>("CHitImmunity")
			.property("burn_immunity", &NlcConditionFields::GetImmunity<ALife::eHitTypeBurn>, &NlcConditionFields::SetImmunity<ALife::eHitTypeBurn>)
			.property("shock_immunity", &NlcConditionFields::GetImmunity<ALife::eHitTypeShock>, &NlcConditionFields::SetImmunity<ALife::eHitTypeShock>)
			.property("chemical_burn_immunity", &NlcConditionFields::GetImmunity<ALife::eHitTypeChemicalBurn>, &NlcConditionFields::SetImmunity<ALife::eHitTypeChemicalBurn>)
			.property("radiation_immunity", &NlcConditionFields::GetImmunity<ALife::eHitTypeRadiation>, &NlcConditionFields::SetImmunity<ALife::eHitTypeRadiation>)
			.property("telepatic_immunity", &NlcConditionFields::GetImmunity<ALife::eHitTypeTelepatic>, &NlcConditionFields::SetImmunity<ALife::eHitTypeTelepatic>)
			.property("wound_immunity", &NlcConditionFields::GetImmunity<ALife::eHitTypeWound>, &NlcConditionFields::SetImmunity<ALife::eHitTypeWound>)
			.property("fire_wound_immunity", &NlcConditionFields::GetImmunity<ALife::eHitTypeFireWound>, &NlcConditionFields::SetImmunity<ALife::eHitTypeFireWound>)
			.property("strike_immunity", &NlcConditionFields::GetImmunity<ALife::eHitTypeStrike>, &NlcConditionFields::SetImmunity<ALife::eHitTypeStrike>)
			.property("explosion_immunity", &NlcConditionFields::GetImmunity<ALife::eHitTypeExplosion>, &NlcConditionFields::SetImmunity<ALife::eHitTypeExplosion>)
			.property("wound_2_immunity", &NlcConditionFields::GetImmunity<ALife::eHitTypeWound_2>, &NlcConditionFields::SetImmunity<ALife::eHitTypeWound_2>)
			.property("physic_strike_immunity", &NlcConditionFields::GetImmunity<ALife::eHitTypePhysicStrike>, &NlcConditionFields::SetImmunity<ALife::eHitTypePhysicStrike>)
			,
			class_<CEntityCondition, CHitImmunity>("CEntityCondition")
			//.def(constructor<>())
			.def("get_wound_size", &NlcConditionFields::GetWoundSize)
			.def("get_wound_total_size", &NlcConditionFields::GetWoundTotalSize)
			.def("AddWound", &CEntityCondition::AddWound)
			.def("ClearWounds", &CEntityCondition::ClearWounds)
			.def("GetWhoHitLastTimeID", &CEntityCondition::GetWhoHitLastTimeID)
			.def("GetPower", &CEntityCondition::GetPower)
			.def("SetPower", &CEntityCondition::SetPower)
			.def("GetRadiation", &CEntityCondition::GetRadiation)
			.def("GetPsyHealth", &CEntityCondition::GetPsyHealth)
			.def("GetSatiety", &CEntityCondition::GetSatiety)
			.def("GetThirst", &CEntityCondition::GetThirst)
			.def("GetEntityMorale", &CEntityCondition::GetEntityMorale)
			.def("GetHealthLost", &CEntityCondition::GetHealthLost)
			.def("IsLimping", &CEntityCondition::IsLimping)
			.def("ChangeSatiety", &CEntityCondition::ChangeSatiety)
			.def("ChangeThirst", &CEntityCondition::ChangeThirst)
			.def("ChangeHealth", &CEntityCondition::ChangeHealth)
			.def("ChangePower", &CEntityCondition::ChangePower)
			.def("ChangeRadiation", &CEntityCondition::ChangeRadiation)
			.def("ChangePsyHealth", &CEntityCondition::ChangePsyHealth)
			.def("ChangeAlcohol", &CEntityCondition::ChangeAlcohol)
			.def("ChangeIntoxication", &CEntityCondition::ChangeIntoxication)
			.def("GetIntoxication", &CEntityCondition::GetIntoxication)
			.def("SetMaxPower", &CEntityCondition::SetMaxPower)
			.def("GetMaxPower", &CEntityCondition::GetMaxPower)
			.def("ChangeEntityMorale", &CEntityCondition::ChangeEntityMorale)
			.def("ChangeBleeding", &CEntityCondition::ChangeBleeding)
			.def("BleedingSpeed", &CEntityCondition::BleedingSpeed)
			.property("health", &NlcConditionFields::GetHealth, &NlcConditionFields::SetHealth)
			.property("health_max", &NlcConditionFields::GetMaxHealth, &NlcConditionFields::SetMaxHealth)
			.property("max_health", &NlcConditionFields::GetMaxHealth, &NlcConditionFields::SetMaxHealth)
			.property("power", &NlcConditionFields::GetPower, &NlcConditionFields::SetPower)
			.property("power_max", &NlcConditionFields::GetMaxPower, &NlcConditionFields::SetMaxPower)
			.property("radiation", &NlcConditionFields::GetRadiation, &NlcConditionFields::SetRadiation)
			.property("psy_health", &NlcConditionFields::GetPsyHealth, &NlcConditionFields::SetPsyHealth)
			.property("radiation_v", &NlcConditionFields::GetChange<&SConditionChangeV::m_fV_Radiation>, &NlcConditionFields::SetChange<&SConditionChangeV::m_fV_Radiation>)
			.property("psy_health_v", &NlcConditionFields::GetChange<&SConditionChangeV::m_fV_PsyHealth>, &NlcConditionFields::SetChange<&SConditionChangeV::m_fV_PsyHealth>)
			.property("circumspection_v", &NlcConditionFields::GetChange<&SConditionChangeV::m_fV_Circumspection>, &NlcConditionFields::SetChange<&SConditionChangeV::m_fV_Circumspection>)
			.property("morale_v", &NlcConditionFields::GetChange<&SConditionChangeV::m_fV_EntityMorale>, &NlcConditionFields::SetChange<&SConditionChangeV::m_fV_EntityMorale>)
			.property("radiation_health_v", &NlcConditionFields::GetChange<&SConditionChangeV::m_fV_RadiationHealth>, &NlcConditionFields::SetChange<&SConditionChangeV::m_fV_RadiationHealth>)
			.property("bleeding_v", &NlcConditionFields::GetChange<&SConditionChangeV::m_fV_Bleeding>, &NlcConditionFields::SetChange<&SConditionChangeV::m_fV_Bleeding>)
			.property("wound_incarnation_v", &NlcConditionFields::GetChange<&SConditionChangeV::m_fV_WoundIncarnation>, &NlcConditionFields::SetChange<&SConditionChangeV::m_fV_WoundIncarnation>)
			.property("health_restore_v", &NlcConditionFields::GetChange<&SConditionChangeV::m_fV_HealthRestore>, &NlcConditionFields::SetChange<&SConditionChangeV::m_fV_HealthRestore>)
			.enum_("EBoostParams")
			[
				value("eBoostHpRestore", int(EBoostParams::eBoostHpRestore)),
				value("eBoostPowerRestore", int(EBoostParams::eBoostPowerRestore)),
				value("eBoostRadiationRestore", int(EBoostParams::eBoostRadiationRestore)),
				value("eBoostBleedingRestore", int(EBoostParams::eBoostBleedingRestore)),
				value("eBoostMaxWeight", int(EBoostParams::eBoostMaxWeight)),
				value("eBoostRadiationProtection", int(EBoostParams::eBoostRadiationProtection)),
				value("eBoostTelepaticProtection", int(EBoostParams::eBoostTelepaticProtection)),
				value("eBoostChemicalBurnProtection", int(EBoostParams::eBoostChemicalBurnProtection)),
				value("eBoostBurnImmunity", int(EBoostParams::eBoostBurnImmunity)),
				value("eBoostShockImmunity", int(EBoostParams::eBoostShockImmunity)),
				value("eBoostRadiationImmunity", int(EBoostParams::eBoostRadiationImmunity)),
				value("eBoostTelepaticImmunity", int(EBoostParams::eBoostTelepaticImmunity)),
				value("eBoostChemicalBurnImmunity", int(EBoostParams::eBoostChemicalBurnImmunity)),
				value("eBoostExplImmunity", int(EBoostParams::eBoostExplImmunity)),
				value("eBoostStrikeImmunity", int(EBoostParams::eBoostStrikeImmunity)),
				value("eBoostFireWoundImmunity", int(EBoostParams::eBoostFireWoundImmunity)),
				value("eBoostWoundImmunity", int(EBoostParams::eBoostWoundImmunity))
			],

			class_<CActorCondition, CEntityCondition>("CActorCondition")
			//.def(constructor<>())
			.def("ClearAllBoosters", &ClearAllBoosters)
			.def("ApplyBooster", &ApplyBooster_script)
			.def("BoosterForEach", &BoosterForEach)
			.def("WoundForEach", &WoundForEach)
			.def("V_Satiety", &CActorCondition::V_Satiety)
			.def("V_SatietyPower", &CActorCondition::V_SatietyPower)
			.def("V_SatietyHealth", &CActorCondition::V_SatietyHealth)
			.def("SatietyCritical", &CActorCondition::SatietyCritical)
			.def("GetSatiety", &CActorCondition::GetSatiety)
			.def("V_Thirst", &CActorCondition::V_Thirst)
			.def("V_ThirstPower", &CActorCondition::V_ThirstPower)
			.def("V_ThirstHealth", &CActorCondition::V_ThirstHealth)
			.def("ThirstCritical", &CActorCondition::ThirstCritical)
			.def("GetThirst", &CActorCondition::GetThirst)
			.def("IsLimping", &CActorCondition::IsLimping)
			.def("IsCantWalk", &CActorCondition::IsCantWalk)
			.def("IsCantWalkWeight", &CActorCondition::IsCantWalkWeight)
			.def("IsCantSprint", &CActorCondition::IsCantSprint)
			.def_readwrite("m_MaxWalkWeight", &CActorCondition::m_MaxWalkWeight)
			.def_readwrite("m_fJumpPower", &CActorCondition::m_fJumpPower)
			.def_readwrite("m_fStandPower", &CActorCondition::m_fStandPower)
			.def_readwrite("m_fJumpWeightPower", &CActorCondition::m_fJumpWeightPower)
			.def_readwrite("m_fWalkWeightPower", &CActorCondition::m_fWalkWeightPower)
			.def_readwrite("m_fOverweightWalkK", &CActorCondition::m_fOverweightWalkK)
			.def_readwrite("m_fOverweightJumpK", &CActorCondition::m_fOverweightJumpK)
			.def_readwrite("m_fAccelK", &CActorCondition::m_fAccelK)
			.def_readwrite("m_fSprintK", &CActorCondition::m_fSprintK)
			.def_readwrite("m_condition_flags", &CActorCondition::m_condition_flags)
			.property("satiety", &NlcConditionFields::GetSatiety, &NlcConditionFields::SetSatiety)
			.property("satiety_v", &NlcConditionFields::GetSatietyV, &NlcConditionFields::SetSatietyV)
			.property("satiety_health_v", &NlcConditionFields::GetSatietyHealthV, &NlcConditionFields::SetSatietyHealthV)
			.property("alcohol_health", &NlcConditionFields::GetAlcohol, &NlcConditionFields::SetAlcohol)
			.property("alcohol_v", &NlcConditionFields::GetAlcoholV, &NlcConditionFields::SetAlcoholV)
			.def_readwrite("max_power_leak_speed", &CActorCondition::m_fPowerLeakSpeed)
			.def_readwrite("jump_power", &CActorCondition::m_fJumpPower)
			.def_readwrite("stand_power", &CActorCondition::m_fStandPower)
			.def_readwrite("walk_power", &CActorCondition::m_fWalkPower)
			.def_readwrite("jump_weight_power", &CActorCondition::m_fJumpWeightPower)
			.def_readwrite("walk_weight_power", &CActorCondition::m_fWalkWeightPower)
			.def_readwrite("overweight_walk_k", &CActorCondition::m_fOverweightWalkK)
			.def_readwrite("overweight_jump_k", &CActorCondition::m_fOverweightJumpK)
			.def_readwrite("accel_k", &CActorCondition::m_fAccelK)
			.def_readwrite("sprint_k", &CActorCondition::m_fSprintK)
			.def_readwrite("max_walk_weight", &CActorCondition::m_MaxWalkWeight)
			.property("limping", &NlcConditionFields::IsLimping)
			.property("cant_walk", &NlcConditionFields::IsCantWalk)
			.property("cant_sprint", &NlcConditionFields::IsCantSprint)
			.enum_("condition_flags")
			[
				value("eCriticalPowerReached", int(CActorCondition::eCriticalPowerReached)),
				value("eCriticalBleedingSpeed", int(CActorCondition::eCriticalBleedingSpeed)),
				value("eCriticalSatietyReached", int(CActorCondition::eCriticalSatietyReached)),
				value("eCriticalThirstReached", int(CActorCondition::eCriticalThirstReached)),
				value("eCriticalRadiationReached", int(CActorCondition::eCriticalRadiationReached)),
				value("eWeaponJammedReached", int(CActorCondition::eWeaponJammedReached)),
				value("ePhyHealthMinReached", int(CActorCondition::ePhyHealthMinReached)),
				value("ePhyHealthMinReached", int(CActorCondition::ePhyHealthMinReached)),
				value("eCantWalkWeight", int(CActorCondition::eCantWalkWeight)),
				value("eCantWalkWeightReached", int(CActorCondition::eCantWalkWeightReached))
			]
		];
}