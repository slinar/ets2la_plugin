#pragma once

#include "base_ctrl.hpp"

#include "prism/collections/list_dyn.hpp"
#include "prism/common.hpp"
#include "prism/navigation/gps_manager.hpp"

#include <cstdint>

#pragma pack( push, 1 )

namespace ets2_la_plugin::prism
{
    class game_ctrl_u : public base_ctrl_u // Size: 0x3F88 1.54
    {
    public:
        char pad_2E98[ 8 ];                                                                //0x2E98 (0x08)
        class radio_u* radio;                                                              //0x2EA0 (0x08)
        class police_ctrl_u* police_ctrl;                                                  //0x2EA8 (0x08)
        char pad_2EB0[ 480 ];                                                              //0x2EB0 (0x1e0)
        class list_dyn_t< class vehicle_shared_u* > some_nearby_non_ai_vehicles__dont_use; //0x3090 (0x38)
        char pad_30C8[ 36 ];                                                               //0x30C8 (0x24)
        class int3_t last_ferry_position;                                                  //0x30EC (0x0c)
        char pad_30F8[ 2264 ];                                                             //0x30F8 (0x8d8)
        class gps_manager_t gps_manager__dont_use;                                         //0x39D0 (0x590)
        char pad_3F60[ 40 ];                                                               //0x3F60 (0x28)

        static uint32_t some_nearby_non_ai_vehicles_offset;
        static uint32_t gps_manager_offset;

        static bool scan_patterns();
        static game_ctrl_u* get();

        list_dyn_t< class vehicle_shared_u* >* get_some_nearby_non_ai_vehicles_list() const;
    };
    static_assert( sizeof( game_ctrl_u ) == 0x3F88 );
}
#pragma pack( pop )
