#include "StdAfx.h"
#include "pch_script.h"
#include "InventoryOwner.h"
#include "GameObject.h"
#include "Inventory.h"
#include "inventory_space.h"
#include "inventory_item.h"
#include "Level.h"
using namespace luabind;

namespace InventoryScript
{
	PIItem FindItem(u16 ItemID)
	{
		return smart_cast<PIItem>(Level().Objects.net_Find(ItemID));
	}

	void ToBelt(CInventory* Inventory, u16 ItemID)
	{
		if (PIItem Item = FindItem(ItemID))
		{
			Inventory->Belt(Item);
		}
	}

	void ToSlot(CInventory* Inventory, u16 ItemID, bool NotActivate)
	{
		if (PIItem Item = FindItem(ItemID))
		{
			const u16 FreeSlot = Inventory->FindFreeAllowedSlot(Item);
			Inventory->Slot(FreeSlot != NO_ACTIVE_SLOT ? FreeSlot : Item->BaseSlot(), Item, NotActivate);
		}
	}

	void ToRuck(CInventory* Inventory, u16 ItemID)
	{
		if (PIItem Item = FindItem(ItemID))
		{
			Inventory->Ruck(Item);
		}
	}

	u32 GetMaxBelt(CInventory* Inventory) { return Inventory->BeltWidth(); }
	void SetMaxBelt(CInventory* Inventory, u32 Value) { Inventory->SetMaxBelt(Value); }
	float GetMaxWeight(CInventory* Inventory) { return Inventory->GetMaxWeight(); }
	void SetMaxWeight(CInventory* Inventory, float Value) { Inventory->SetMaxWeight(Value); }
	float GetTotalWeight(CInventory* Inventory) { return Inventory->TotalWeight(); }
}

#pragma optimize("s",on)
void CInventoryOwner::script_register(lua_State *L)
{
	module(L)
		[
			class_<CInventory>("CInventory")
			.def("TotalWeight", &CInventory::TotalWeight)
			.def("CalcTotalWeight", &CInventory::CalcTotalWeight)
			.def("GetActiveSlot", &CInventory::GetActiveSlot)
			.def("SetActiveSlot", &CInventory::SetActiveSlot)
			.def("GetMaxWeight", &CInventory::GetMaxWeight)
			.def("SetMaxWeight", &CInventory::SetMaxWeight)
			.def("BeltWidth", &CInventory::BeltWidth)
			.def("Activate", &CInventory::Activate)
			.def("InSlot", &CInventory::InSlot)
			.def("InBelt", &CInventory::InBelt)
			.def("InRuck", &CInventory::InRuck)
			.def("ItemFromSlot", &CInventory::ItemFromSlot)
			.property("max_belt", &InventoryScript::GetMaxBelt, &InventoryScript::SetMaxBelt)
			.property("max_weight", &InventoryScript::GetMaxWeight, &InventoryScript::SetMaxWeight)
			.property("total_weight", &InventoryScript::GetTotalWeight)
			.def("to_belt", &InventoryScript::ToBelt)
			.def("to_slot", &InventoryScript::ToSlot)
			.def("to_ruck", &InventoryScript::ToRuck)
			.enum_("slots")
			[
				luabind::value("NO_ACTIVE_SLOT", NO_ACTIVE_SLOT),
				luabind::value("KNIFE_SLOT", KNIFE_SLOT),
				luabind::value("INV_SLOT_2", INV_SLOT_2),
				luabind::value("INV_SLOT_3", INV_SLOT_3),
				luabind::value("GRENADE_SLOT", GRENADE_SLOT),
				luabind::value("BINOCULAR_SLOT", BINOCULAR_SLOT),
				luabind::value("BOLT_SLOT", BOLT_SLOT),
				luabind::value("OUTFIT_SLOT", OUTFIT_SLOT),
				luabind::value("PDA_SLOT", PDA_SLOT),
				luabind::value("DEVICE_SLOT", DEVICE_SLOT),
				luabind::value("TORCH_SLOT", TORCH_SLOT),
				luabind::value("ARTEFACT_SLOT", ARTEFACT_SLOT),
				luabind::value("HELMET_SLOT", HELMET_SLOT),
				luabind::value("BACKPACK_SLOT", BACKPACK_SLOT),
				luabind::value("POWER_BANK_SLOT", POWER_BANK_SLOT),
				luabind::value("NVG_SLOT", NVG_SLOT),
				luabind::value("PISTOL_SLOT_NEW", PISTOL_SLOT_NEW),
				luabind::value("CUSTOM_SLOT_1", CUSTOM_SLOT_1),
				luabind::value("CUSTOM_SLOT_2", CUSTOM_SLOT_2),
				luabind::value("CUSTOM_SLOT_3", CUSTOM_SLOT_3),
				luabind::value("CUSTOM_SLOT_4", CUSTOM_SLOT_4),
				luabind::value("CUSTOM_SLOT_5", CUSTOM_SLOT_5),
				luabind::value("CUSTOM_SLOT_6", CUSTOM_SLOT_6),
				luabind::value("CUSTOM_SLOT_7", CUSTOM_SLOT_7),
				luabind::value("CUSTOM_SLOT_8", CUSTOM_SLOT_8),
				luabind::value("CUSTOM_SLOT_9", CUSTOM_SLOT_9),
				luabind::value("CUSTOM_SLOT_10", CUSTOM_SLOT_10),
				luabind::value("LAST_SLOT", LAST_SLOT)
			]
			,
			class_<CInventoryOwner>("CInventoryOwner")
			//.def(constructor<>())
			.def("trade_section", &CInventoryOwner::trade_section)
			.def("sell_useless_items", &CInventoryOwner::sell_useless_items)
			.def("buy_supplies", &CInventoryOwner::buy_supplies)
			.def("inventory", (CInventory& (CInventoryOwner::*)())&CInventoryOwner::inventory)
			.def("IconName", &CInventoryOwner::IconName)
			.def("get_money", &CInventoryOwner::get_money)
			.def("EnableTalk", &CInventoryOwner::EnableTalk)
			.def("DisableTalk", &CInventoryOwner::DisableTalk)
			.def("IsTalkEnabled", &CInventoryOwner::IsTalkEnabled)
			.def("EnableTrade", &CInventoryOwner::EnableTrade)
			.def("DisableTrade", &CInventoryOwner::DisableTrade)
			.def("IsTradeEnabled", &CInventoryOwner::IsTradeEnabled)
			.def("EnableInvUpgrade", &CInventoryOwner::EnableInvUpgrade)
			.def("DisableInvUpgrade", &CInventoryOwner::DisableInvUpgrade)
			.def("IsInvUpgradeEnabled", &CInventoryOwner::IsInvUpgradeEnabled)
			.def("GetTalkPartner", &CInventoryOwner::GetTalkPartner)
			.def("OfferTalk", &CInventoryOwner::OfferTalk)
			.def("StartTalk", &CInventoryOwner::StartTalk)
			.def("StopTalk", &CInventoryOwner::StopTalk)
			.def("IsTalking", &CInventoryOwner::IsTalking)
			.def("deadbody_can_take", &CInventoryOwner::deadbody_can_take)
			.def("deadbody_can_take_status", &CInventoryOwner::deadbody_can_take_status)
			.def("deadbody_closed", &CInventoryOwner::deadbody_closed)
			.def("deadbody_closed_status", &CInventoryOwner::deadbody_closed_status)
		];
}