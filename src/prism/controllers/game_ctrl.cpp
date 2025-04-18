#pragma once
#include "game_ctrl.hpp"
#include "memory/memory_utils.hpp"
#include "patterns.hpp"

namespace ets2_la_plugin::prism
{
    game_ctrl_u* game_ctrl_u::get()
    {
        static unit_descriptor_t* stored_game_ctrl_unit_descriptor = nullptr;

        auto* base_ctrl = base_ctrl_u::get();

        if ( base_ctrl == nullptr )
            return nullptr;

        // base_ctrl has multiple derived units that this ptr could be
        // so we need to check that the current one is 'game_ctrl'

        if ( stored_game_ctrl_unit_descriptor == nullptr )
        {
            // check if the base_ctrl ptr is of type 'game_ctrl' and store the address of the unit descriptor if it is
            // that way we can just compare the unit descriptor addresses instead of comparing strings every time
            if ( strcmp( *base_ctrl->get_unit_descriptor()->p_class_name, "game_ctrl" ) == 0 )
            {
                stored_game_ctrl_unit_descriptor = base_ctrl->get_unit_descriptor();
            }
        }

        if ( base_ctrl->get_unit_descriptor() == stored_game_ctrl_unit_descriptor )
        {
            return static_cast< game_ctrl_u* >( base_ctrl );
        }
        else
        {
            return nullptr;
        }
    }

    // contains TruckerMP nearby players' trucks and trailers
    // also contains trucks and trailers in menus (background/service center/truck dealer/etc...)
    list_dyn_t< class vehicle_shared_u* >* game_ctrl_u::get_some_nearby_non_ai_vehicles_list() const
    {
        static uint32_t some_nearby_non_ai_vehicles_offset_in_game_ctrl = 0;

        if ( some_nearby_non_ai_vehicles_offset_in_game_ctrl != 0 )
        {
            return reinterpret_cast< list_dyn_t< class vehicle_shared_u* >* >(
                reinterpret_cast< uint64_t >( this ) + some_nearby_non_ai_vehicles_offset_in_game_ctrl
            );
        }

        const auto addr = memory::get_address_for_pattern( patterns::game_ctrl_u_some_nearby_non_ai_vehicles, 8 );
        if ( addr == 0 )
        {
            return nullptr;
        }
        some_nearby_non_ai_vehicles_offset_in_game_ctrl = *reinterpret_cast< uint32_t* >( addr );

        return reinterpret_cast< list_dyn_t< class vehicle_shared_u* >* >(
            reinterpret_cast< uint64_t >( this ) + some_nearby_non_ai_vehicles_offset_in_game_ctrl
        );
    }
}
