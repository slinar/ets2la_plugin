#pragma once
#include "prism/collections/array_dyn.hpp"
#include "prism/unit/unit.hpp"
#include "./core_camera.hpp"
#include "core.hpp"
#include "patterns.hpp"
#include "memory/memory_utils.hpp"

#pragma pack(push, 1)

namespace ets2_la_plugin::prism
{
	thread_local static uint64_t camera_manager_instance_ptr_address = 0;

	class camera_manager_u : public unit_t // Size: 0x0098
	{
	public:
		uint32_t current_camera; //0x0010 (0x04)
		uint32_t total_camera_count; //0x0014 (0x04)
		char pad_0018[24]; //0x0018 (0x18)
		array_dyn_t<core_camera_u*> cameras; //0x0030 (0x20)
		char pad_0050[40]; //0x0050 (0x28)
		core_camera_u* core_cam; //0x0078 (0x08)
		char pad_0080[24]; //0x0080 (0x18)

		static camera_manager_u* get()
		{
			if (camera_manager_instance_ptr_address != 0) return *reinterpret_cast<camera_manager_u**>(camera_manager_instance_ptr_address);

			const auto addr = memory::get_address_for_pattern(patterns::camera_manager);

			if (addr == 0) return nullptr;
			camera_manager_instance_ptr_address = addr + *reinterpret_cast<int32_t*>(addr + 3) + 7;

			CCore::g_instance->debug("Found camera_manager @ +{:x}", memory::as_offset(camera_manager_instance_ptr_address));

			return *reinterpret_cast<camera_manager_u**>(camera_manager_instance_ptr_address);
		}

	};
	static_assert(sizeof(camera_manager_u) == 0x98);

}

#pragma pack(pop)
