#pragma once

#include "./traffic.hpp"
#include "core.hpp"
#include "patterns.hpp"
#include "memory/memory_utils.hpp"

#pragma pack(push, 1)

namespace ets2_la_plugin::prism
{
	thread_local static uint64_t game_traffic_instance_ptr_address = 0;

	class game_traffic_u : public traffic_u // Size: 0x05F8
	{
	public:
		char pad_05D8[32]; //0x05D8 (0x20)


		static game_traffic_u* get()
		{
			if (game_traffic_instance_ptr_address != 0) return *reinterpret_cast<game_traffic_u**>(game_traffic_instance_ptr_address);

			const auto addr = memory::get_address_for_pattern(patterns::game_traffic);

			if (addr == 0) return nullptr;
			game_traffic_instance_ptr_address = addr + *reinterpret_cast<int32_t*>(addr + 6) + 10;

			CCore::g_instance->debug("Found game_traffic @ +{:x}", memory::as_offset(game_traffic_instance_ptr_address));

			return *reinterpret_cast<game_traffic_u**>(game_traffic_instance_ptr_address);
		}
	};
	static_assert(sizeof(game_traffic_u) == 0x5F8);
}

#pragma pack(pop)
