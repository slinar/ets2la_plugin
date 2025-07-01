#pragma once

#include "base_ctrl.hpp"

#include "prism/collections/list_dyn.hpp"
#include "prism/common.hpp"
#include "prism/navigation/gps_manager.hpp"

#include <cstdint>

#pragma pack( push, 1 )

namespace ets2_la_plugin::prism
{
    class game_ctrl_u : public base_ctrl_u // Size: 0x4098 1.55
    {
    public:
        char pad_2F98[ 8 ];                                                          // 0x2F98 (0x08)
        class radio_u* radio;                                                        // 0x2FA0 (0x08)
        class police_ctrl_u* police_ctrl;                                            // 0x2FA8 (0x08)
        char pad_2FB0[ 496 ];                                                        // 0x2FB0 (0x1f0)
        list_dyn_t< class vehicle_shared_u* > some_nearby_non_ai_vehicles__dont_use; // 0x31A0 (0x38)
        char pad_31D8[ 36 ];                                                         // 0x31D8 (0x24)
        int3_t last_ferry_position;                                                  // 0x31FC (0x0c)
        char pad_3208[ 2264 ];                                                       // 0x3208 (0x8d8)
        gps_manager_t gps_manager__dont_use;                                         // 0x3AE0 (0x590)
        char pad_4070[ 40 ];                                                         // 0x4070 (0x28)

        static game_ctrl_u* get();

        list_dyn_t< class vehicle_shared_u* >* get_some_nearby_non_ai_vehicles_list() const;
    };
    static_assert( sizeof( game_ctrl_u ) == 0x4098 );
}
#pragma pack( pop )
