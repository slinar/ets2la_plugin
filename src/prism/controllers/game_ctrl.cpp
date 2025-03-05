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
}
