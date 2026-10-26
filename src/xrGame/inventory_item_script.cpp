#include "StdAfx.h"
#include "pch_script.h"
#include "inventory_item.h"
#include "inventory_space.h"
#include "ai_space.h"
#include "../xrScripts/script_engine.h"

using namespace luabind;

namespace InventoryItemScript
{
	int GetSlot(CInventoryItem* Item) { return int(Item->BaseSlot()) - 1; }
	void SetSlot(CInventoryItem* Item, int Slot) { Item->m_ItemCurrPlace.base_slot_id = u16(Slot + 1); }

	luabind::object GetSlots(CInventoryItem* Item)
	{
		luabind::object Slots = luabind::newtable(ai().script_engine().lua());
		int Index = 1;
		if (Item->BaseSlot() != NO_ACTIVE_SLOT)
		{
			Slots[Index++] = GetSlot(Item);
		}
		for (u16 Slot : Item->AllowedSlots())
		{
			if (Slot != Item->BaseSlot())
			{
				Slots[Index++] = int(Slot) - 1;
			}
		}
		return Slots;
	}

	u32 GetItemPlace(CInventoryItem* Item) { return Item->CurrPlace(); }
	u32 GetCost(CInventoryItem* Item) { return Item->Cost(); }
	void SetCost(CInventoryItem* Item, u32 Cost) { Item->SetCost(Cost); }
	float GetWeight(CInventoryItem* Item) { return Item->Weight(); }
	void SetWeight(CInventoryItem* Item, float Weight) { Item->setWeight(Weight); }
	float GetCondition(CInventoryItem* Item) { return Item->GetCondition(); }
	void SetCondition(CInventoryItem* Item, float Condition) { Item->SetCondition(Condition); }
	u32 GetFlags(CInventoryItem* Item) { return Item->GetItemFlags(); }
	void SetFlags(CInventoryItem* Item, u32 Flags) { Item->SetItemFlags(u16(Flags)); }
	const char* GetName(CInventoryItem* Item) { return Item->NameItem(); }
	const char* GetShortName(CInventoryItem* Item) { return Item->NameShort(); }
	bool IsQuestItem(CInventoryItem* Item) { return Item->IsQuestItem(); }
	void SetQuestItem(CInventoryItem* Item, bool Quest) { Item->SetQuestItem(Quest); }
	void SetVisibleInSlot(CInventoryItem* Item, int, bool Visible) { Item->SetVisibleInInventory(Visible); }
	bool VisibleInSlot(CInventoryItem* Item, int) { return Item->VisibleInInventory(); }
}

#pragma optimize("s",on)
void CInventoryItem::script_register(lua_State *L)
{
	module(L)
		[
			class_<CInventoryItem>("CInventoryItem")
			//.def(constructor<>())
			.enum_("EIIFlags")
			[
				value("FdropManual", int(EIIFlags::FdropManual)),
				value("FCanTake", int(EIIFlags::FCanTake)),
				value("FCanTrade", int(EIIFlags::FCanTrade)),
				value("Fbelt", int(EIIFlags::Fbelt)),
				value("Fruck", int(EIIFlags::Fruck)),
				value("FRuckDefault", int(EIIFlags::FRuckDefault)),
				value("FUsingCondition", int(EIIFlags::FUsingCondition)),
				value("FAllowSprint", int(EIIFlags::FAllowSprint)),
				value("Fuseful_for_NPC", int(EIIFlags::Fuseful_for_NPC)),
				value("FInInterpolation", int(EIIFlags::FInInterpolation)),
				value("FInInterpolate", int(EIIFlags::FInInterpolate)),
				value("FIsQuestItem", int(EIIFlags::FIsQuestItem)),
				value("FIsHelperItem", int(EIIFlags::FIsHelperItem)),
				value("FCanStack", int(EIIFlags::FCanStack))
					// TODO: St4lker0k765: Port this?
					//				value("FWeaponDisabled", int(EIIFlags::FWeaponDisabled))
			]
			.def("NameItem", &CInventoryItem::NameItem)
			.def("NameShort", &CInventoryItem::NameShort)
			.def("ItemDescription", &CInventoryItem::ItemDescription)
			.def("Useful", &CInventoryItem::Useful)
			.def("IsUsingCondition", &CInventoryItem::IsUsingCondition)
			.def("CanStack", &CInventoryItem::CanStack)
			.def("HandDependence", &CInventoryItem::HandDependence)
			.def("ActivateItem", &CInventoryItem::ActivateItem)
			.def("DeactivateItem", &CInventoryItem::DeactivateItem)
			.def("GetDropManual", &CInventoryItem::GetDropManual)
			.def("SetDropManual", &CInventoryItem::SetDropManual)
			.def("IsQuestItem", &CInventoryItem::IsQuestItem)
			.def("Cost", &CInventoryItem::Cost)
			.def("Weight", &CInventoryItem::Weight)
			.def("SetWeight", &CInventoryItem::setWeight)
			.def("GetIconName", &CInventoryItem::GetIconName)
			.def("GetCondition", &CInventoryItem::GetCondition)
			.def("SetCondition", &CInventoryItem::SetCondition)
			.def("CanTake", &CInventoryItem::CanTake)
			.def("has_any_upgrades", &CInventoryItem::has_any_upgrades)
			.def("verify_upgrade", &CInventoryItem::verify_upgrade)
			.def("install_upgrade", &CInventoryItem::install_upgrade)
			.def("pre_install_upgrade", &CInventoryItem::pre_install_upgrade)
			.def_readwrite("m_flags", &CInventoryItem::m_flags)
			.property("slot", &InventoryItemScript::GetSlot, &InventoryItemScript::SetSlot)
			.property("slots", &InventoryItemScript::GetSlots)
			.property("item_place", &InventoryItemScript::GetItemPlace)
			.property("cost", &InventoryItemScript::GetCost, &InventoryItemScript::SetCost)
			.property("inv_weight", &InventoryItemScript::GetWeight, &InventoryItemScript::SetWeight)
			.property("item_condition", &InventoryItemScript::GetCondition, &InventoryItemScript::SetCondition)
			.property("item_flags", &InventoryItemScript::GetFlags, &InventoryItemScript::SetFlags)
			.property("item_name", &InventoryItemScript::GetName)
			.property("item_name_short", &InventoryItemScript::GetShortName)
			.property("quest_item", &InventoryItemScript::IsQuestItem, &InventoryItemScript::SetQuestItem)
			.def("set_visible_in_slot", &InventoryItemScript::SetVisibleInSlot)
			.def("visible_in_slot", &InventoryItemScript::VisibleInSlot)
		];
}