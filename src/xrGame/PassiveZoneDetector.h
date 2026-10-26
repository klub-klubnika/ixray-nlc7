#pragma once
#include "../xrEngine/Feel_Touch.h"
#include "HudSound.h"

class CAnomalyZone;
class CGameObject;

// SoC style anomaly detector: beeps for nearby zones while the item is in a slot or on the belt.
// Config: radius, zone_class_N, zone_min_freq_N, zone_max_freq_N, zone_sound_N_, zone_map_location_N
class CPassiveZoneDetector final : public Feel::Touch
{
	struct SZoneType
	{
		float MinFreq = 0.0f;
		float MaxFreq = 0.0f;
		HUD_SOUND_ITEM Sounds;
		shared_str MapLocation;
	};

	xr_map<CLASS_ID, SZoneType> ZoneTypes;
	xr_map<CAnomalyZone*, u32> SoundTimes;
	CGameObject* Owner = nullptr;
	float Radius = 0.0f;
	bool Working = false;

public:
	CPassiveZoneDetector() = default;
	~CPassiveZoneDetector() override;

	void Load(const char* Section, CGameObject* Item);
	bool IsAvailable() const { return !ZoneTypes.empty(); }

	void SetWorking(bool Value);
	bool IsWorking() const { return Working; }

	// Zone search, called from shedule_Update with the holder position
	void UpdateZones(Fvector Position);
	// Beeps, called every frame
	void UpdateSounds(const Fvector& HolderPosition);
	void Clear();

protected:
	bool feel_touch_contact(CObject* O) override;
	void feel_touch_new(CObject* O) override;
	void feel_touch_delete(CObject* O) override;

private:
	void UpdateMapLocation(CAnomalyZone* Zone, bool Add);
	void StopSounds();
};
