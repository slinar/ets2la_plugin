#pragma once

#include "base_ctrl.hpp"

#include "prism/collections/list_dyn.hpp"
#include "prism/common.hpp"
#include "prism/navigation/gps_manager.hpp"

#include <cstdint>

#pragma pack( push, 1 )

namespace ets2_la_plugin::prism
{
    class game_ctrl_u : public base_ctrl_u // Size: 0x38D8
    {
    public:
        char pad_2988[ 8 ];                                //0x2988 (0x08)
        class radio_u* radio;                              //0x2990 (0x08)
        class police_ctrl_u* police_ctrl;                  //0x2998 (0x08)
        char pad_29A0[ 480 ];                              //0x29A0 (0x1e0)
        list_dyn_t< class light_source_u* > light_sources; //0x2B80 (0x38)
        char pad_2BB8[ 36 ];                               //0x2BB8 (0x24)
        int3_t last_ferry_position;                        //0x2BDC (0x0c)
        char pad_2BE8[ 1848 ];                             //0x2BE8 (0x738)
        gps_manager_t gps_manager__dont_use;               //0x3320 (0x590)
        char pad_38B0[ 40 ];                               //0x38B0 (0x28)

        static game_ctrl_u* get();
    };
    static_assert( sizeof( game_ctrl_u ) == 0x38D8 );
}
#pragma pack( pop )
