#include "StdAfx.h"
#include "PassiveZoneDetector.h"
#include "AnomalyZone.h"
#include "Level.h"
#include "map_manager.h"
#include "../xrSound/ai_sounds.h"

CPassiveZoneDetector::~CPassiveZoneDetector()
{
	Clear();
	for (auto& [ClassId, ZoneType] : ZoneTypes)
	{
		HUD_SOUND_ITEM::DestroySound(ZoneType.Sounds);
	}
}

void CPassiveZoneDetector::Load(const char* Section, CGameObject* Item)
{
	Owner = Item;
	Radius = pSettings->read_if_exists<float>(Section, "radius", 0.0f);
	if (Radius <= 0.0f)
	{
		return;
	}

	string256 Line;
	for (u32 i = 1;; ++i)
	{
		xr_sprintf(Line, "zone_class_%d", i);
		if (!pSettings->line_exist(Section, Line))
		{
			break;
		}

		const char* ZoneSection = pSettings->r_string(Section, Line);
		if (!pSettings->section_exist(ZoneSection) || !pSettings->line_exist(ZoneSection, "class"))
		{
			Msg("! [%s] %s: unknown zone section [%s]", Section, Line, ZoneSection);
			continue;
		}

		SZoneType& ZoneType = ZoneTypes[TEXT2CLSID(pSettings->r_string(ZoneSection, "class"))];

		xr_sprintf(Line, "zone_min_freq_%d", i);
		ZoneType.MinFreq = pSettings->read_if_exists<float>(Section, Line, 0.1f);
		xr_sprintf(Line, "zone_max_freq_%d", i);
		ZoneType.MaxFreq = pSettings->read_if_exists<float>(Section, Line, 1.0f);

		xr_sprintf(Line, "zone_sound_%d_", i);
		HUD_SOUND_ITEM::LoadSound(Section, Line, ZoneType.Sounds, SOUND_TYPE_ITEM);

		xr_sprintf(Line, "zone_map_location_%d", i);
		ZoneType.MapLocation = pSettings->read_if_exists<str_c>(Section, Line, nullptr);
	}
}

void CPassiveZoneDetector::SetWorking(bool Value)
{
	if (Working == Value)
	{
		return;
	}

	Working = Value;
	if (!Working)
	{
		Clear();
	}
}

void CPassiveZoneDetector::UpdateZones(Fvector Position)
{
	if (Working)
	{
		feel_touch_update(Position, Radius);
	}
}

void CPassiveZoneDetector::UpdateSounds(const Fvector& HolderPosition)
{
	if (!Working)
	{
		return;
	}

	for (auto& [Zone, SoundTime] : SoundTimes)
	{
		auto TypeIt = ZoneTypes.find(Zone->CLS_ID);
		if (TypeIt == ZoneTypes.end() || !Zone->IsEnabled())
		{
			continue;
		}

		SZoneType& ZoneType = TypeIt->second;

		float DistToZone = HolderPosition.distance_to(Zone->Position()) - 0.8f * Zone->Radius();
		if (DistToZone < 0.0f)
		{
			DistToZone = 0.0f;
		}

		float RelPower = 1.0f - DistToZone / Radius;
		clamp(RelPower, 0.0f, 1.0f);

		// Beep frequency grows to max_freq near the zone
		const float Freq = ZoneType.MinFreq + (ZoneType.MaxFreq - ZoneType.MinFreq) * RelPower * RelPower * RelPower * RelPower;
		const float Period = 1000.0f / Freq;

		if (float(SoundTime) > Period)
		{
			SoundTime = 0;
			HUD_SOUND_ITEM::PlaySound(ZoneType.Sounds, Fvector().set(0.0f, 0.0f, 0.0f), Owner, true, false, true);
		}
		else
		{
			SoundTime += Device.dwTimeDelta;
		}
	}
}

void CPassiveZoneDetector::Clear()
{
	for (auto& [Zone, SoundTime] : SoundTimes)
	{
		UpdateMapLocation(Zone, false);
	}

	StopSounds();
	SoundTimes.clear();
	Feel::Touch::feel_touch.clear();
}

bool CPassiveZoneDetector::feel_touch_contact(CObject* O)
{
	CGameObject* GameObject = O != nullptr ? O->cast_game_object() : nullptr;
	CAnomalyZone* Zone = GameObject != nullptr ? GameObject->cast_anomaly_zone() : nullptr;
	return Zone != nullptr && ZoneTypes.find(Zone->CLS_ID) != ZoneTypes.end();
}

void CPassiveZoneDetector::feel_touch_new(CObject* O)
{
	CAnomalyZone* Zone = O->cast_game_object()->cast_anomaly_zone();
	if (Zone != nullptr && Zone->IsEnabled())
	{
		SoundTimes[Zone] = 0;
		UpdateMapLocation(Zone, true);
	}
}

void CPassiveZoneDetector::feel_touch_delete(CObject* O)
{
	CGameObject* GameObject = O->cast_game_object();
	CAnomalyZone* Zone = GameObject != nullptr ? GameObject->cast_anomaly_zone() : nullptr;
	if (Zone != nullptr)
	{
		SoundTimes.erase(Zone);
		UpdateMapLocation(Zone, false);
	}
}

void CPassiveZoneDetector::UpdateMapLocation(CAnomalyZone* Zone, bool Add)
{
	auto TypeIt = ZoneTypes.find(Zone->CLS_ID);
	if (TypeIt == ZoneTypes.end() || TypeIt->second.MapLocation.size() == 0)
	{
		return;
	}

	if (Add)
	{
		Level().MapManager().AddMapLocation(TypeIt->second.MapLocation, Zone->ID());
	}
	else
	{
		Level().MapManager().RemoveMapLocation(TypeIt->second.MapLocation, Zone->ID());
	}
}

void CPassiveZoneDetector::StopSounds()
{
	for (auto& [ClassId, ZoneType] : ZoneTypes)
	{
		HUD_SOUND_ITEM::StopSound(ZoneType.Sounds);
	}
}
