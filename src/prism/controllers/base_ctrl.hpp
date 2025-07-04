#pragma once
#include "prism/collections/array_dyn.hpp"
#include "prism/string.hpp"
#include "prism/token.hpp"
#include "prism/unit/unit.hpp"

#pragma pack( push, 1 )

namespace ets2_la_plugin::prism
{
    class base_ctrl_u : public unit_t // Size: 0x2E98 1.54
    {
    public:
        array_dyn_t< class kdop_item_t* > map_kdop_items;                         //0x0010 (0x20)
        array_dyn_t< class node_item_t* > map_nodes;                              //0x0030 (0x20)
        char pad_0050[ 16 ];                                                      //0x0050 (0x10)
        class game_data_u* game_data;                                             //0x0060 (0x08)
        class game_traffic_u* game_traffic;                                       //0x0068 (0x08)
        char pad_0070[ 24 ];                                                      //0x0070 (0x18)
        class map_data_u* map_data;                                               //0x0088 (0x08)
        char pad_0090[ 48 ];                                                      //0x0090 (0x30)
        class string_dyn_t timezone;                                              //0x00C0 (0x18)
        char pad_00D8[ 312 ];                                                     //0x00D8 (0x138)
        array_dyn_t< class core_camera_u* > cameras;                              //0x0210 (0x20)
        array_dyn_t< class trailer_u* > unk_trailers;                             //0x0230 (0x20)
        array_dyn_t< class trailer_def_u* > unk_trailer_defs;                     //0x0250 (0x20)
        array_dyn_t< class traffic_vehicle_type_u* > traffic_vehicle_types;       //0x0270 (0x20)
        array_dyn_t< class traffic_trailer_type_u* > traffic_trailer_types;       //0x0290 (0x20)
        array_dyn_t< class accessory_hookup_data_u* > accessory_hookup_datas;     //0x02B0 (0x20)
        class license_plate_manager_u* license_plate_manager;                     //0x02D0 (0x08)
        class driver_plate_manager_u* driver_plate_manager;                       //0x02D8 (0x08)
        class player_tag_manager_u* player_tag_manager;                           //0x02E0 (0x08)
        class traffic_sign_manager_u* traffic_sign_manager;                       //0x02E8 (0x08)
        char pad_02F0[ 16 ];                                                      //0x02F0 (0x10)
        array_dyn_t< class kdop_item_t* > kdop_items_unk;                         //0x0300 (0x20)
        array_dyn_t< class kdop_item_t* > kdop_items_unk2;                        //0x0320 (0x20)
        char pad_0340[ 32 ];                                                      //0x0340 (0x20)
        array_dyn_t< class kdop_item_t* > some_nearby_kdop_items;                 //0x0360 (0x20)
        char pad_0380[ 432 ];                                                     //0x0380 (0x1b0)
        class telemetry_channel_provider_list_t* telemetry_channel_provider_list; //0x0530 (0x08)
        char pad_0538[ 168 ];                                                     //0x0538 (0xa8)
        array_dyn_t< token_t > N000004AC;                                         //0x05E0 (0x20)
        array_dyn_t< token_t > N000004B0;                                         //0x0600 (0x20)
        char pad_0620[ 24 ];                                                      //0x0620 (0x18)
        class trigger_manager_u* trigger_manager;                                 //0x0638 (0x08)
        class camera_manager_u* camera_manager;                                   //0x0640 (0x08)
        class cutscene_manager_u* cutscene_manager;                               //0x0648 (0x08)
        class journey_events_manager_u* journey_events_manager;                   //0x0650 (0x08)
        char pad_0658[ 16 ];                                                      //0x0658 (0x10)
        class core_camera_u* core_camera;                                         //0x0668 (0x08)
        class environment_u* environment;                                         //0x0670 (0x08)
        class oversize_manager_u* oversize_manager;                               //0x0678 (0x08)
        char pad_0680[ 72 ];                                                      //0x0680 (0x48)
        class eye_state_u* eye_state;                                             //0x06C8 (0x08)
        char pad_06D0[ 40 ];                                                      //0x06D0 (0x28)
        array_dyn_t< token_t > speed_classes;                                     //0x06F8 (0x20)
        uint32_t N000004DA;                                                       //0x0718 (0x04)
        char pad_071C[ 8404 ];                                                    //0x071C (0x20d4)
        class local_gameplay_mode_u* local_gameplay_mode;                         //0x27F0 (0x08)
        char pad_27F8[ 520 ];                                                     //0x27F8 (0x208)
        uint64_t selected_physics_engine;                                         //0x2A00 (0x08) 1 = physx
        class physics_server_physx_t* physics_server;                             //0x2A08 (0x08)
        char pad_2A10[ 24 ];                                                      //0x2A10 (0x18)
        class physics_data_u* physics_data;                                       //0x2A28 (0x08)
        class physics_interior_camera_data_u* physics_interior_camera_data;       //0x2A30 (0x08)
        class force_feedback_data_u* force_feedback_data;                         //0x2A38 (0x08)
        class vehicle_damage_data_u* vehicle_damage_data;                         //0x2A40 (0x08)
        class vehicle_service_data_u* vehicle_service_data;                       //0x2A48 (0x08)
        class sector_manager_u* sector_manager;                                   //0x2A50 (0x08)
        char pad_2A58[ 360 ];                                                     //0x2A58 (0x168)
        class asset_catalog_u* asset_catalog;                                     //0x2BC0 (0x08)
        char pad_2BC8[ 432 ];                                                     //0x2BC8 (0x1b0)
        class game_actor_u* game_actor__dont_use;                                 //0x2D78 (0x08)
        char pad_2D80[ 280 ];                                                     //0x2D80 (0x118)

        static uint64_t instance_ptr_address;
        static uint32_t game_actor_offset;

        static bool scan_patterns();
        static base_ctrl_u* get();
    };

    static_assert( sizeof( base_ctrl_u ) == 0x2E98 );
}
#pragma pack( pop )
