#include "StdAfx.h"
#include "Level.h"
#include "GameObject.h"
#include "ai_object_location.h"
#include "../../xrServerEntities/xrMessages.h"
#include "../../xrServerEntities/xrServer_Objects_ALife.h"
#include "script_game_object.h"

#include "Actor.h"
#include "WeaponMagazinedWGrenade.h"
#include "WeaponAmmo.h"
#include "Artefact.h"
#include "CustomOutfit.h"
#include "eatable_item.h"
#include "InventoryContainer.h"
#include "ui/UIGameCustom.h"
#include "ui/UIMainIngameWnd.h"
#include "../../xrEngine/EngineApp.h"
#include "ai_space.h"
#include "../../xrScripts/script_engine.h"

#include <luabind/luabind.hpp>

luabind::object ToEngineObject(CGameObject* Object)
{
	lua_State* L = ai().script_engine().lua();
	if (Object == nullptr)
	{
		return luabind::object();
	}

	if (CActor* Actor = smart_cast<CActor*>(Object))
		return luabind::object(L, Actor);
	if (CWeaponMagazinedWGrenade* Weapon = smart_cast<CWeaponMagazinedWGrenade*>(Object))
		return luabind::object(L, Weapon);
	if (CWeaponMagazined* Weapon = smart_cast<CWeaponMagazined*>(Object))
		return luabind::object(L, Weapon);
	if (CWeapon* Weapon = smart_cast<CWeapon*>(Object))
		return luabind::object(L, Weapon);
	if (CWeaponAmmo* Ammo = smart_cast<CWeaponAmmo*>(Object))
		return luabind::object(L, Ammo);
	if (CArtefact* Artefact = smart_cast<CArtefact*>(Object))
		return luabind::object(L, Artefact);
	if (CCustomOutfit* Outfit = smart_cast<CCustomOutfit*>(Object))
		return luabind::object(L, Outfit);
	if (CInventoryContainer* Container = smart_cast<CInventoryContainer*>(Object))
		return luabind::object(L, Container);
	if (CEatableItem* Eatable = smart_cast<CEatableItem*>(Object))
		return luabind::object(L, Eatable);
	if (CInventoryItem* Item = smart_cast<CInventoryItem*>(Object))
		return luabind::object(L, Item);

	return luabind::object(L, Object);
}

static bool SetupGameIcon(u32 Icon, u32 Color, float Width, float Height)
{
	CUIGameCustom* GameUI = CurrentGameUI();
	if (GameUI == nullptr || GameUI->UIMainIngameWnd == nullptr)
	{
		return false;
	}
	return GameUI->UIMainIngameWnd->SetupGameIcon(Icon, Color, Width, Height);
}

static CEngineApp* GetApplication() { return pApp; }
static void SetLoadTexture(CEngineApp* App, const char* Texture) { App->SetScriptLoadTexture(Texture); }

static luabind::object EngineObject(luabind::object Target)
{
	if (Target.type() == LUA_TNUMBER)
	{
		const u16 ID = luabind::object_cast<u16>(Target);
		return ToEngineObject(smart_cast<CGameObject*>(Level().Objects.net_Find(ID)));
	}

	if (CScriptGameObject* ScriptObject = luabind::object_cast_nothrow<CScriptGameObject*>(Target).value_or(nullptr))
	{
		return ToEngineObject(&ScriptObject->object());
	}

	return luabind::object();
}

float SimDistTo(luabind::object Left, luabind::object Right)
{
	CGameObject* ClientObject1 = luabind::object_cast_nothrow<CGameObject*>(Left).value_or(nullptr);
	CGameObject* ClientObject2 = luabind::object_cast_nothrow<CGameObject*>(Right).value_or(nullptr);

	CSE_ALifeObject* ServerObject1 = ClientObject1 != nullptr ? nullptr : luabind::object_cast_nothrow<CSE_ALifeObject*>(Left).value_or(nullptr);
	CSE_ALifeObject* ServerObject2 = ClientObject2 != nullptr ? nullptr : luabind::object_cast_nothrow<CSE_ALifeObject*>(Right).value_or(nullptr);

	if ((ServerObject1 == nullptr && ClientObject1 == nullptr) || (ServerObject2 == nullptr && ClientObject2 == nullptr))
	{
		return FLT_MAX;
	}

	u32 VertexIDLeft = u32(-1);
	u32 VertexIDRight = u32(-1);
	
	if (ClientObject1 != nullptr)
	{
		VertexIDLeft = ClientObject1->ai_location().game_vertex_id();
	}
	else if (ServerObject1 != nullptr)
	{
		const auto& cross_table = ai().cross_table();
		const u32 level_vertex_id = ServerObject1->m_tNodeID;
		if (level_vertex_id >= cross_table.header().level_vertex_count())
		{
			return FLT_MAX;
		}

		VertexIDLeft = cross_table.vertex(level_vertex_id).game_vertex_id();
	}

	if (ClientObject2 != nullptr)
	{
		VertexIDRight = ClientObject2->ai_location().game_vertex_id();
	}
	else if (ServerObject2 != nullptr)
	{
		const auto& cross_table = ai().cross_table();
		const u32 level_vertex_id = ServerObject2->m_tNodeID;
		if (level_vertex_id >= cross_table.header().level_vertex_count())
		{
			return FLT_MAX;
		}

		VertexIDRight = cross_table.vertex(level_vertex_id).game_vertex_id();
	}

	if (!ai().game_graph().valid_vertex_id(VertexIDLeft) || !ai().game_graph().valid_vertex_id(VertexIDRight))
	{
		return FLT_MAX;
	}

	const CGameGraph::CVertex* v1 = ai().game_graph().vertex(VertexIDLeft);
	const CGameGraph::CVertex* v2 = ai().game_graph().vertex(VertexIDRight);

	if (v1 == nullptr || v2 == nullptr)
	{
		return FLT_MAX;
	}

	return v1->game_point().distance_to(v2->game_point());
}


CScriptGameObject* net_find(ALife::_OBJECT_ID object_id)
{
	if (CObject* obj = Level().Objects.net_Find(object_id))
	{
		if (CGameObject* gObj = obj->cast_game_object())
		{
			return gObj->lua_game_object();
		}
	}

	return 0;
}

u32 GameVertexByLevelVertex(u32 LevelVertexID)
{
	const auto& CrossTable = ai().cross_table();
	if (LevelVertexID >= CrossTable.header().level_vertex_count())
	{
		return u32(-1);
	}

	return CrossTable.vertex(LevelVertexID).game_vertex_id();
}

#pragma optimize("s",on)
void CompatibilityBringeExport(lua_State* L)
{
	luabind::module(L, "sim")
	[
		luabind::def("dist_to", SimDistTo),
		luabind::def("net_find", net_find)
	];

	luabind::module(L)
	[
		luabind::def("game_vertex_by_level_vertex", GameVertexByLevelVertex),
		luabind::def("engine_object", EngineObject),
		luabind::def("setup_game_icon", SetupGameIcon),
		luabind::class_<CEngineApp>("CApplication")
			.def("set_load_texture", &SetLoadTexture),
		luabind::def("get_application", GetApplication)
	];

	luabind::get_globals(L)["SRotation"] = luabind::get_globals(L)["rotation"];

	luabind::object script_events = luabind::get_globals(L)["script_events"];
	script_events["M_SCRIPT_EVENT"] = M_SCRIPT_EVENT;
}