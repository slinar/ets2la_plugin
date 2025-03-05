#pragma once
#include "base_ctrl.hpp"

#include "memory/memory_utils.hpp"
#include "patterns.hpp"

#include "core.hpp"

namespace ets2_la_plugin::prism
{
    base_ctrl_u* base_ctrl_u::get()
    {
        static uint64_t base_ctrl_instance_ptr_address = 0;

        if ( base_ctrl_instance_ptr_address != 0 )
            return *reinterpret_cast< base_ctrl_u** >( base_ctrl_instance_ptr_address );

        const auto addr = memory::get_address_for_pattern( patterns::base_ctrl, 3 );

        if ( addr == 0 )
            return nullptr;
        base_ctrl_instance_ptr_address = addr + *reinterpret_cast< int32_t* >( addr ) + 4;

        CCore::g_instance->debug( "Found base_ctrl @ +{:x}", memory::as_offset( base_ctrl_instance_ptr_address ) );

        return *reinterpret_cast< base_ctrl_u** >( base_ctrl_instance_ptr_address );
    }
}
