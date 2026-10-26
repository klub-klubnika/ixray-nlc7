#include "StdAfx.h"
#include "pch_script.h"
#include "../../Weapon.h"
#include "../../WeaponMagazined.h"
#include "../../WeaponMagazinedWGrenade.h"
#include "../../WeaponPistol.h"

using namespace luabind;

namespace WeaponScript
{
	int GetAmmoElapsed(CWeapon* Weapon) { return Weapon->GetAmmoElapsed(); }
	void SetAmmoElapsed(CWeapon* Weapon, int Count) { Weapon->SetAmmoElapsed(Count); }
	int GetMagSize(CWeapon* Weapon) { return Weapon->GetAmmoMagSize(); }
	void SetMagSize(CWeapon* Weapon, int Size) { Weapon->SetAmmoMagSize(Size); }
	float GetZoomFactor(CWeapon* Weapon) { return Weapon->GetZoomFactor(); }
	void SetZoomFactor(CWeapon* Weapon, float Factor) { Weapon->SetZoomFactor(Factor); }
	float GetScopeZoomFactor(CWeapon* Weapon) { return Weapon->GetScopeZoomFactor(); }
	void SetScopeZoomFactor(CWeapon* Weapon, float Factor) { Weapon->SetScopeZoomFactor(Factor); }
	float GetIronSightZoomFactor(CWeapon* Weapon) { return Weapon->GetIronSightZoomFactor(); }
	void SetIronSightZoomFactor(CWeapon* Weapon, float Factor) { Weapon->SetIronSightZoomFactor(Factor); }
	int GetScopeX(CWeapon* Weapon) { return Weapon->GetScopeX(); }
	int GetScopeY(CWeapon* Weapon) { return Weapon->GetScopeY(); }
	bool IsMisfire(CWeapon* Weapon) { return Weapon->IsMisfire(); }
	bool IsScopeAttached(CWeapon* Weapon) { return Weapon->IsScopeAttached(); }
	void StartFire(CWeapon* Weapon) { Weapon->ScriptFireStart(); }
	void StopFire(CWeapon* Weapon) { Weapon->ScriptFireEnd(); }
	void StopShooting(CWeapon* Weapon) { Weapon->StopShooting(); }
	Fvector GetFirePoint(CWeapon* Weapon) { return Weapon->get_CurrentFirePoint(); }
	Fvector GetFirePoint2(CWeapon* Weapon) { return Weapon->get_CurrentFirePoint2(); }
	Fmatrix GetParticlesXform(CWeapon* Weapon) { return Weapon->get_ParticlesXFORM(); }

	bool IsAmmoCompatible(CWeapon* Weapon, const char* Section)
	{
		for (const shared_str& AmmoType : Weapon->m_ammoTypes)
		{
			if (AmmoType == Section)
			{
				return true;
			}
		}
		return false;
	}

	const char* GetAmmoType(CWeapon* Weapon, int Index)
	{
		if (Index < 0)
		{
			Index = Weapon->GetAmmoType(false);
		}
		return Index >= 0 && Index < int(Weapon->m_ammoTypes.size()) ? Weapon->m_ammoTypes[Index].c_str() : "";
	}

	int GetFireMode(CWeaponMagazined* Weapon) { return Weapon->GetCurrentFireMode(); }
}

#pragma optimize("s",on)
void CWeapon::script_register(lua_State* L)
{
	module(L)
	[
		class_<CWeapon, bases<CGameObject, CInventoryItem>>("CWeapon")
			.property("ammo_elapsed", &WeaponScript::GetAmmoElapsed, &WeaponScript::SetAmmoElapsed)
			.property("ammo_mag_size", &WeaponScript::GetMagSize, &WeaponScript::SetMagSize)
			.property("zoom_factor", &WeaponScript::GetZoomFactor, &WeaponScript::SetZoomFactor)
			.property("scope_zoom_factor", &WeaponScript::GetScopeZoomFactor, &WeaponScript::SetScopeZoomFactor)
			.property("ironsight_zoom_factor", &WeaponScript::GetIronSightZoomFactor, &WeaponScript::SetIronSightZoomFactor)
			.property("scope_x", &WeaponScript::GetScopeX)
			.property("scope_y", &WeaponScript::GetScopeY)
			.property("misfire", &WeaponScript::IsMisfire)
			.property("scope_attached", &WeaponScript::IsScopeAttached)
			.def_readwrite("misfire_probability", &CWeapon::misfireProbability)
			.def_readwrite("misfire_condition_k", &CWeapon::misfireConditionK)
			.def_readwrite("condition_shot_dec", &CWeapon::conditionDecreasePerShot)
			.def_readwrite("fire_dispersion_condition_factor", &CWeapon::fireDispersionConditionFactor)
			.def("start_fire", &WeaponScript::StartFire)
			.def("stop_fire", &WeaponScript::StopFire)
			.def("stop_shoothing", &WeaponScript::StopShooting)
			.def("get_fire_point", &WeaponScript::GetFirePoint)
			.def("get_fire_point2", &WeaponScript::GetFirePoint2)
			.def("get_particles_xform", &WeaponScript::GetParticlesXform)
			.def("is_ammo_compat", &WeaponScript::IsAmmoCompatible)
			.def("get_ammo_type", &WeaponScript::GetAmmoType)
	];

	module(L)
	[
		class_<CWeaponMagazined, CWeapon>("CWeaponMagazined")
			.def(constructor<>())
			.property("fire_mode", &WeaponScript::GetFireMode)
	];

	module(L)
	[
		class_<CWeaponMagazinedWGrenade, CWeaponMagazined>("CWeaponMagazinedWGrenade")
			.def(constructor<>())
	];

	module(L)
	[
		class_<CWeaponPistol, CWeaponMagazined>("CWeaponPistol")
			.def(constructor<>())
	];

	luabind::object weapon_class = luabind::get_globals(L)["CWeaponMagazined"];

	auto MakeAliasLambda = [&weapon_class, L](const char* TypeID)
	{
		lua_pushstring(L, TypeID);
		weapon_class.pushvalue();
		lua_settable(L, LUA_GLOBALSINDEX);
	};

	MakeAliasLambda("CWeaponVintorez");
	MakeAliasLambda("CWeaponVal");
	MakeAliasLambda("CWeaponLR300");
	MakeAliasLambda("CWeaponFN2000");
	MakeAliasLambda("CWeaponCustomPistol");
	MakeAliasLambda("CWeaponSVU");

	weapon_class = luabind::get_globals(L)["CWeaponMagazinedWGrenade"];

	MakeAliasLambda("CWeaponAK74");
	MakeAliasLambda("CWeaponGroza");

	weapon_class = luabind::get_globals(L)["CWeaponPistol"];

	MakeAliasLambda("CWeaponPM");
	MakeAliasLambda("CWeaponHPSA");
	MakeAliasLambda("CWeaponFORT");
	MakeAliasLambda("CWeaponUSP45");
	MakeAliasLambda("CWeaponWalther");
}
