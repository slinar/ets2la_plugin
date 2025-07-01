#pragma once
#include "prism/collections/array_dyn.hpp"
#include "prism/string.hpp"
#include "prism/token.hpp"
#include "prism/unit/unit.hpp"

#pragma pack( push, 1 )

namespace ets2_la_plugin::prism
{
    class base_ctrl_u : public unit_t // Size: 0x2F98 1.55
    {
    public:
        array_dyn_t< class kdop_item_t* > map_kdop_items;                     // 0x0010 (0x20)
        array_dyn_t< class node_item_t* > map_nodes;                          // 0x0030 (0x20)
        char pad_0050[ 16 ];                                                  // 0x0050 (0x10)
        class game_data_u* game_data;                                         // 0x0060 (0x08)
        class game_traffic_u* game_traffic;                                   // 0x0068 (0x08)
        char pad_0070[ 24 ];                                                  // 0x0070 (0x18)
        class map_data_u* map_data;                                           // 0x0088 (0x08)
        char pad_0090[ 48 ];                                                  // 0x0090 (0x30)
        class string_dyn_t timezone;                                          // 0x00C0 (0x18)
        char pad_00D8[ 664 ];                                                 // 0x00D8 (0x298)
        array_dyn_t< class core_camera_u* > cameras;                          // 0x0370 (0x20)
        array_dyn_t< class trailer_u* > unk_trailers;                         // 0x0390 (0x20)
        array_dyn_t< class trailer_def_u* > unk_trailer_defs;                 // 0x03B0 (0x20)
        array_dyn_t< class traffic_vehicle_type_u* > traffic_vehicle_types;   // 0x03D0 (0x20)
        array_dyn_t< class traffic_trailer_type_u* > traffic_trailer_types;   // 0x03F0 (0x20)
        array_dyn_t< class accessory_hookup_data_u* > accessory_hookup_datas; // 0x0410 (0x20)
        class license_plate_manager_u* license_plate_manager;                 // 0x0430 (0x08)
        class driver_plate_manager_u* driver_plate_manager;                   // 0x0438 (0x08)
        class player_tag_manager_u* player_tag_manager;                       // 0x0440 (0x08)
        class traffic_sign_manager_u* traffic_sign_manager;                   // 0x0448 (0x08)
        char pad_0450[ 16 ];                                                  // 0x0450 (0x10)
        array_dyn_t< class kdop_item_t* > kdop_items_unk;                     // 0x0460 (0x20)
        array_dyn_t< class kdop_item_t* > kdop_items_unk2;                    // 0x0480 (0x20)
        char pad_04A0[ 32 ];                                                  // 0x04A0 (0x20)
        array_dyn_t< class kdop_item_t* > some_nearby_kdop_items;             // 0x04C0 (0x20) I think
        char pad_04E0[ 512 ];                                                 // 0x04E0 (0x200)
        array_dyn_t< token_t > N000004AC;                                     // 0x06E0 (0x20)
        array_dyn_t< token_t > N000004B0;                                     // 0x0700 (0x20)
        char pad_0720[ 24 ];                                                  // 0x0720 (0x18)
        class trigger_manager_u* trigger_manager;                             // 0x0738 (0x08)
        class camera_manager_u* camera_manager;                               // 0x0740 (0x08)
        class cutscene_manager_u* cutscene_manager;                           // 0x0748 (0x08)
        class journey_events_manager_u* journey_events_manager;               // 0x0750 (0x08)
        char pad_0758[ 16 ];                                                  // 0x0758 (0x10)
        class core_camera_u* core_camera;                                     // 0x0768 (0x08)
        class environment_u* environment;                                     // 0x0770 (0x08)
        class oversize_manager_u* oversize_manager;                           // 0x0778 (0x08)
        char pad_0780[ 72 ];                                                  // 0x0780 (0x48)
        class eye_state_u* eye_state;                                         // 0x07C8 (0x08)
        char pad_07D0[ 48 ];                                                  // 0x07D0 (0x30)
        array_dyn_t< token_t > speed_classes;                                 // 0x0800 (0x20)
        uint32_t N000004DA;                                                   // 0x0820 (0x04)
        char pad_0824[ 8396 ];                                                // 0x0824 (0x20cc)
        class local_gameplay_mode_u* local_gameplay_mode;                     // 0x28F0 (0x08)
        char pad_28F8[ 520 ];                                                 // 0x28F8 (0x208)
        uint64_t selected_physics_engine;                                     // 0x2B00 (0x08) 1 = physx
        class physics_server_physx_t* physics_server;                         // 0x2B08 (0x08)
        char pad_2B10[ 24 ];                                                  // 0x2B10 (0x18)
        class physics_data_u* physics_data;                                   // 0x2B28 (0x08)
        class physics_interior_camera_data_u* physics_interior_camera_data;   // 0x2B30 (0x08)
        class force_feedback_data_u* force_feedback_data;                     // 0x2B38 (0x08)
        class vehicle_damage_data_u* vehicle_damage_data;                     // 0x2B40 (0x08)
        class vehicle_service_data_u* vehicle_service_data;                   // 0x2B48 (0x08)
        class sector_manager_u* sector_manager;                               // 0x2B50 (0x08)
        char pad_2B58[ 344 ];                                                 // 0x2B58 (0x158)
        class asset_catalog_u* asset_catalog;                                 // 0x2CB0 (0x08)
        char pad_2CB8[ 424 ];                                                 // 0x2CB8 (0x1a8)
        class game_actor_u* game_actor__dont_use;                             // 0x2E60 (0x08)
        char pad_2E68[ 304 ];                                                 // 0x2E68 (0x130)

        static base_ctrl_u* get();
    };

    static_assert( sizeof( base_ctrl_u ) == 0x2F98 );
}
#pragma pack( pop )
