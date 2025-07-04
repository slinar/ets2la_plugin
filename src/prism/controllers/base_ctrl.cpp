#pragma once
#include "base_ctrl.hpp"

#include "memory/memory_utils.hpp"
#include "patterns.hpp"

namespace ets2_la_plugin::prism
{

    uint64_t base_ctrl_u::instance_ptr_address = 0;
    uint32_t base_ctrl_u::game_actor_offset    = 0;

    bool base_ctrl_u::scan_patterns()
    {
        const auto addr = memory::get_address_for_pattern( patterns::base_ctrl );

        if ( addr == 0 )
        {
            return false;
        }
        base_ctrl_u::instance_ptr_address = addr + 3 + *reinterpret_cast< int32_t* >( addr + 3 ) + 4;
        base_ctrl_u::game_actor_offset    = *reinterpret_cast< uint32_t* >( addr + 17 );

        return true;
    }

    base_ctrl_u* base_ctrl_u::get()
    {
        if ( base_ctrl_u::instance_ptr_address != 0 )
        {
            return *reinterpret_cast< base_ctrl_u** >( base_ctrl_u::instance_ptr_address );
        }
        return nullptr;
    }
}
