#pragma once
#include "game_traffic.hpp"

#include "core.hpp"
#include "memory/memory_utils.hpp"
#include "patterns.hpp"

namespace ets2_la_plugin::prism
{
    game_traffic_u* game_traffic_u::get()
    {
        static uint64_t game_traffic_instance_ptr_address = 0;

        if ( game_traffic_instance_ptr_address != 0 )
            return *reinterpret_cast< game_traffic_u** >( game_traffic_instance_ptr_address );

        const auto addr = memory::get_address_for_pattern( patterns::game_traffic, 6 );

        if ( addr == 0 )
            return nullptr;
        game_traffic_instance_ptr_address = addr + *reinterpret_cast< int32_t* >( addr ) + 4;

        CCore::g_instance->debug(
            "Found game_traffic @ +{:x}", memory::as_offset( game_traffic_instance_ptr_address )
        );

        return *reinterpret_cast< game_traffic_u** >( game_traffic_instance_ptr_address );
    }
}
