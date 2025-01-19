#include "core.hpp"

#include <MinHook.h>

#include "consts.hpp"

#include "memory/memory_utils.hpp"
#include "sdk/stores.hpp"

#include <common/scssdk_telemetry_truck_common_channels.h>

#include "prism/camera/camera_manager.hpp"
#include "prism/traffic/game_traffic.hpp"
#include "prism/traffic/traffic_ai_trailer.hpp"

namespace ets2_la_plugin
{
    CCore *CCore::g_instance = nullptr;

    SCSAPI_VOID telemetry_tick(const scs_event_t event, const void *const event_info, scs_context_t context)
    {
        CCore::g_instance->tick();
    }

    CCore::CCore(const scs_telemetry_init_params_v101_t *init_params) : init_params_(init_params)
    {
        this->hooks_manager_ = new CHooksManager();
        scs_log_ = init_params->common.log;
        g_instance = this;
    }

    CCore::~CCore()
    {
        this->destroy();
        MH_RemoveHook(nullptr);
    }

    void CCore::get_camera_data() const
    {
        const auto camera_manager = prism::camera_manager_u::get();

        if (camera_manager == nullptr)
            return;

        // make sure index is in the list
        if (camera_manager->current_camera >= camera_manager->cameras.size)
            return;

        const auto *current_camera = camera_manager->cameras[camera_manager->current_camera];
        if (current_camera != nullptr)
        {
            // get all wanted data from current camera
            current_camera->camera_fov;
            current_camera->placement;
        }
        // or loop all cameras to get data from all of them
        for (const auto *camera : camera_manager->cameras)
        {
            // ...
        }
    }

    void CCore::get_ai_traffic_data() const
    {
        auto *game_traffic = prism::game_traffic_u::get();

        if (game_traffic == nullptr)
            return;

        const auto truck_pos = this->truck_pos; // our truck position from the SDK telemetry

        // seems like once a spawned_vehicle_t is created in the array it is not removed even if that vehicle has despawned
        // ai_vehicle.vehicle->traffic_vehicle and physics_data will be nullptr when it has despawned
        for (const auto &ai_vehicle : game_traffic->ai_vehicles)
        {
            if (ai_vehicle.vehicle == nullptr || ai_vehicle.vehicle->traffic_vehicle == nullptr || ai_vehicle.vehicle->physics_data == nullptr)
                continue;

            // ai vehicle world coordinates
            const auto ai_x = ai_vehicle.vehicle->placement.cx * 512 + ai_vehicle.vehicle->placement.pos.x;
            const auto ai_y = ai_vehicle.vehicle->placement.pos.y;
            const auto ai_z = ai_vehicle.vehicle->placement.cz * 512 + ai_vehicle.vehicle->placement.pos.z;

            const auto ai_rotation = ai_vehicle.vehicle->placement.rot;

            // ai vehicle bounding box
            const auto &truck_bb = ai_vehicle.vehicle->physics_data->bounding_box; // (width, height, length)

            ai_vehicle.vehicle->physics_data->speed; // m/s
            ai_vehicle.vehicle->physics_data->acceleration;

            const auto *trailer = ai_vehicle.vehicle->trailer;
            // go through all slave trailers
            while (trailer != nullptr)
            {
                if (trailer->physics_data == nullptr)
                {
                    break;
                }

                // ai trailer world coordinates
                const auto ai_trailer_x = trailer->placement.cx * 512 + trailer->placement.pos.x;
                const auto ai_trailer_y = trailer->placement.pos.y;
                const auto ai_trailer_z = trailer->placement.cz * 512 + trailer->placement.pos.z;

                const auto ai_trailer_rotation = trailer->placement.rot;

                // trailer bounding box
                const auto &trailer_bb = trailer->physics_data->bounding_box; // (width, height, length)

                trailer = trailer->slave_trailer;
            }
        }
    }

    void CCore::tick() const
    {
        this->get_camera_data();

        this->get_ai_traffic_data();
    }

    bool CCore::init()
    {
        MH_Initialize();
        this->info("Initializing {}", VERSION);

        if (this->init_params_->register_for_channel(
                SCS_TELEMETRY_TRUCK_CHANNEL_world_placement,
                SCS_U32_NIL,
                SCS_VALUE_TYPE_dplacement,
                SCS_TELEMETRY_CHANNEL_FLAG_none,
                telemetry_store_dplacement,
                &this->truck_pos) != SCS_RESULT_ok)
        {
            this->error("Could not register for truck world_placement channel");
            return false;
        }

        if (this->init_params_->register_for_event(SCS_TELEMETRY_EVENT_frame_end, telemetry_tick, nullptr) != SCS_RESULT_ok)
        {
            this->error("Could not register for frame_end event");
            return false;
        }

        return true;
    }

    void CCore::destroy()
    {
        delete this->hooks_manager_;
    }
}
