#include "StdAfx.h"
#include "pch_script.h"
#include "InventoryContainer.h"
#include "script_game_object.h"

using namespace luabind;

#pragma optimize("s",on)
void CInventoryContainer::script_register(lua_State *L)
{
	module(L)
		[
			class_<CInventoryContainer>("CInventoryContainer")
			.def("empty", &CInventoryContainer::IsEmpty)
			.def("object_count", &CInventoryContainer::ItemsCount)
			.def("object", &CInventoryContainer::ItemByIndex)
			.def("object", &CInventoryContainer::ItemByName)
			.def("open", &CInventoryContainer::Open)
			.def("close", &CInventoryContainer::Close)
			.property("is_opened", &CInventoryContainer::IsOpened)
			.property("cost", &CInventoryContainer::Cost)
			.property("weight", &CInventoryContainer::Weight)
		];
}
