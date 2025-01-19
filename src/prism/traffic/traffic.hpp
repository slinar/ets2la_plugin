#pragma once
#include "prism/collections/array_dyn.hpp"
#include "prism/unit/unit.hpp"

#include "./traffic_ai_vehicle.hpp"

#pragma pack(push, 1)

namespace ets2_la_plugin::prism
{
	class traffic_u : public unit_t // Size: 0x05D8
	{
	public:
		char pad_0010[184]; //0x0010 (0xb8)
		array_dyn_t<spawned_vehicle_t> ai_vehicles; //0x00C8 (0x20)
		char pad_00E8[1264]; //0x00E8 (0x4f0)
	};
	static_assert(sizeof(traffic_u) == 0x5D8);

}

#pragma pack(pop)
