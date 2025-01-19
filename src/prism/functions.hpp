#pragma once
namespace ets2_la_plugin
{
    struct float3_t;

    namespace prism
    {
        class game_physics_vehicle_u;
        class vehicle_wheel_steering_data_t;

        using game_physics_vehicle_u_steering_advance_fn = uint64_t __fastcall( game_physics_vehicle_u* );
        using set_individual_steering_fn = void __fastcall(vehicle_wheel_steering_data_t*, float);
    }
}
