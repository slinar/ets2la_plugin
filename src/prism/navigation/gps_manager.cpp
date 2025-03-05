#pragma once
#include "gps_manager.hpp"
#include "memory/memory_utils.hpp"
#include "patterns.hpp"
#include "prism/controllers/game_ctrl.hpp"

namespace ets2_la_plugin::prism
{
    gps_manager_t* gps_manager_t::get()
    {
        static uint32_t gps_manager_offset_in_game_ctrl = 0;

        const auto* game_ctrl = game_ctrl_u::get();

        if ( game_ctrl == nullptr )
            return nullptr;

        if ( gps_manager_offset_in_game_ctrl != 0 )
        {
            return reinterpret_cast< gps_manager_t* >(
                reinterpret_cast< uint64_t >( game_ctrl ) + gps_manager_offset_in_game_ctrl
            );
        }

        const auto addr = memory::get_address_for_pattern( patterns::gps_manager, 5 );
        if ( addr == 0 )
            return nullptr;
        gps_manager_offset_in_game_ctrl = *reinterpret_cast< uint32_t* >( addr );

        return reinterpret_cast< gps_manager_t* >(
            reinterpret_cast< uint64_t >( game_ctrl ) + gps_manager_offset_in_game_ctrl
        );
    }
}
