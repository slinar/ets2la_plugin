#include "vehicle_shared.hpp"

#include "core.hpp"
#include "memory/memory_utils.hpp"
#include "patterns.hpp"

namespace ets2_la_plugin::prism
{
    bool vehicle_shared_u::set_steering_angle( const float new_steering_value )
    {
        static uint64_t steering_offset = 0;

        if ( steering_offset != 0 )
        {
            *reinterpret_cast< float* >( reinterpret_cast< char* >( this ) + steering_offset ) = new_steering_value;
            return true;
        }

        const auto addr = memory::get_address_for_pattern( patterns::vehicle_shared_u_steering_offset, 4 );

        if ( addr == 0 )
            return false;

        steering_offset = *reinterpret_cast< int32_t* >( addr );

        CCore::g_instance->debug( "Found vehicle_shared_u::steering offset @ +{:x}", steering_offset );

        *reinterpret_cast< float* >( reinterpret_cast< char* >( this ) + steering_offset ) = new_steering_value;

        return true;
    }
}
