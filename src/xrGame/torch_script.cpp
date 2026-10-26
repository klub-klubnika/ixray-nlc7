#include "StdAfx.h"
#include "pch_script.h"
#include "Torch.h"
#include "PDA.h"
#include "SimpleDetector.h"
#include "EliteDetector.h"
#include "AdvancedDetector.h"

using namespace luabind;

#pragma optimize("s",on)
void CTorch::script_register	(lua_State *L)
{
	module(L)
	[
		class_<CTorch,CGameObject>("CTorch")
			.def(constructor<>())
			.def("set_animation", &CTorch::SetAnimation)
			.def("set_brightness", &CTorch::SetBrightness)
			.def("set_rgb", &CTorch::SetRGB)
			.def("set_rgb", +[](CTorch* Torch, float R, float G, float B) { Torch->SetRGB(R, G, B, 0); })
			.def("set_angle", &CTorch::SetAngle)
			.def("set_angle", +[](CTorch* Torch, float Angle) { Torch->SetAngle(Angle, 0); })
			.def("set_range", &CTorch::SetRange)
			.def("set_range", +[](CTorch* Torch, float Range) { Torch->SetRange(Range, 0); }),
		class_<CPda,CGameObject>("CPda")
			.def(constructor<>()),
		class_<CScientificDetector,CGameObject>("CScientificDetector")
			.def(constructor<>()),
		class_<CEliteDetector,CGameObject>("CEliteDetector")
			.def(constructor<>()),
		class_<CAdvancedDetector,CGameObject>("CAdvancedDetector")
			.def(constructor<>()),
		class_<CSimpleDetector,CGameObject>("CSimpleDetector")
			.def(constructor<>())
	];
}
