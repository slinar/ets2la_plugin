#pragma once
#include "vehicle_shared.hpp"

#pragma pack(push, 1)
namespace ets2_la_plugin::prism
{
    class physics_vehicle_u : public vehicle_shared_u // Size: 0x1208
    {
    public:
        class array_dyn_t< class model_object_u* > cab_model_objects; //0x0D58 (0x20)
        class model_object_u* ext_interior_model_object; //0x0D78 (0x08)
        class model_object_u* wipers_model_object; //0x0D80 (0x08)
        class model_object_u* driver_swheel_model_object; //0x0D88 (0x08)
        class model_object_u* win_open_model_object; //0x0D90 (0x08)
        char pad_0D98[ 8 ]; //0x0D98 (0x08)
        class N0000592A* N00004468; //0x0DA0 (0x08)
        class N0000592A* N00004469; //0x0DA8 (0x08)
        class accessory_truck_data_u* accessory_truck_data; //0x0DB0 (0x08)
        class vehicle_accessory_u* transmission_accessory; //0x0DB8 (0x08)
        class vehicle_accessory_u* engine_accessory; //0x0DC0 (0x08)
        class vehicle_accessory_u* chassis_accessory; //0x0DC8 (0x08)
        class vehicle_accessory_u* cabin_accessory; //0x0DD0 (0x08)
        char pad_0DD8[ 176 ]; //0x0DD8 (0xb0)
        float air_pressure; //0x0E88 (0x04)
        char pad_0E8C[ 4 ]; //0x0E8C (0x04)
        float oil_pressure; //0x0E90 (0x04)
        float oil_temperature; //0x0E94 (0x04)
        char pad_0E98[ 4 ]; //0x0E98 (0x04)
        float water_temperature; //0x0E9C (0x04)
        char pad_0EA0[ 4 ]; //0x0EA0 (0x04)
        float nominal_battery_voltage_factor; //0x0EA4 (0x04)
        float brake_temps; //0x0EA8 (0x04)
        float brake_input; //0x0EAC (0x04)
        char pad_0EB0[ 568 ]; //0x0EB0 (0x238)
        float steering; //0x10E8 (0x04)
        char pad_10EC[ 24 ]; //0x10EC (0x18)
        float throttle; //0x1104 (0x04)
        float throttle_2; //0x1108 (0x04)
        float clutch; //0x110C (0x04)
        char pad_1110[ 16 ]; //0x1110 (0x10)
        uint64_t coasting_idk; //0x1120 (0x08)
        int32_t gear; //0x1128 (0x04)
        char pad_112C[ 4 ]; //0x112C (0x04)
        uint32_t trailer_attach_state; //0x1130 (0x04) 0 = detached, 1 = detaching, 2 = kingpin locked brace not raised, 3 = attaching 4 = attached
        char pad_1134[ 212 ]; //0x1134 (0xd4)
    };

    static_assert(sizeof( physics_vehicle_u ) == 0x1208);


    class game_physics_vehicle_u : public physics_vehicle_u // Size: 0x12D8
    {
    public:
        char pad_1208[ 64 ]; //0x1208 (0x40)
        class eutpp_paint_u* paint; //0x1248 (0x08)
        class eut2pp_truckpaint_u* truckpaint; //0x1250 (0x08)
        char pad_1258[ 56 ]; //0x1258 (0x38)
        class pp_lamp_colors_u* lamp_colors; //0x1290 (0x08)
        class pp_lamp_colors_u* lamp_colors2; //0x1298 (0x08)
        class pp_lamp_colors_u* lamp_colors3; //0x12A0 (0x08)
        class eut2pp_luminosity_u* luminosity; //0x12A8 (0x08)
        char pad_12B0[ 40 ]; //0x12B0 (0x28)
    };

    static_assert(sizeof( game_physics_vehicle_u ) == 0x12D8);
}

#pragma pack(pop)
