#include "core.hpp"

#include <MinHook.h>

#include "consts.hpp"

#include "memory/memory_utils.hpp"
#include "sdk/stores.hpp"

#include <common/scssdk_telemetry_truck_common_channels.h>

#include "prism/camera/camera_manager.hpp"
#include "prism/traffic/game_traffic.hpp"
#include "prism/traffic/traffic_ai_trailer.hpp"

#include "hooks/function_hook.hpp"
#include "prism/functions.hpp"
#include "prism/vehicles/game_physics_vehicle.hpp"

#include <array>

namespace ets2_la_plugin
{
    // Plugin input memory implementation
    HANDLE input_h_map_file;
    const int input_float_count = 1;
    const int input_bool_count = 1;
    const wchar_t* input_mem_name = L"Local\\ETS2LAPluginInput";
    const size_t boot_size = input_bool_count * sizeof(bool);
    const size_t float_size = input_float_count * sizeof(float);
    const size_t size = float_size + boot_size;

    // Function to initialize shared memory
    void CCore::initialize_mem() const {
        input_h_map_file = CreateFileMapping(
            INVALID_HANDLE_VALUE,    // use paging file
            NULL,                    // default security
            PAGE_READWRITE,          // read/write access
            0,                       // maximum object size (high-order DWORD)
            size,                    // maximum object size (low-order DWORD)
            input_mem_name);                // name of mapping object

        if (input_h_map_file == NULL) {
            DWORD dw = GetLastError();
            std::stringstream ss;
            ss << dw;
            std::string message = "Failed to create file mapping. Error code: " + ss.str();
            this->error(message.c_str());
            return;
        }

        void* pBuf = MapViewOfFile(input_h_map_file, FILE_MAP_ALL_ACCESS, 0, 0, size);

        if (pBuf == NULL) {
            this->error("Failed to map view of file.");
            CloseHandle(input_h_map_file);
            input_h_map_file = NULL;
            return;
        }

        float data[input_bool_count + input_float_count] = {};
        for (int i = 0; i < input_bool_count + input_float_count; i++)
        {
            if (i < input_float_count)
            {
                data[i] = 0.0;
            }
            else
            {
                data[i] = false;
            }
        }
        memcpy(pBuf, data, size);

        UnmapViewOfFile(pBuf);

        this->info("Successfully opened shared mem file.");
    }

    // Function to read shared memory
    std::pair<std::array<float, input_float_count>, std::array<bool, input_bool_count>> CCore::read_mem() const {
        if (input_h_map_file == NULL) {
            this->error("Shared mem file not open.");
            return std::make_pair(std::array<float, input_float_count>{}, std::array<bool, input_bool_count>{});
        }

        void* pBuf = MapViewOfFile(input_h_map_file, FILE_MAP_ALL_ACCESS, 0, 0, size);

        std::array<float, input_float_count> float_data;
        std::array<bool, input_bool_count> bool_data;

        memcpy(float_data.data(), pBuf, float_size);
        memcpy(bool_data.data(), static_cast<char*>(pBuf) + float_size, boot_size);

        UnmapViewOfFile(pBuf);
        return std::make_pair(float_data, bool_data);
    }

    CCore *CCore::g_instance = nullptr;
    std::shared_ptr<CFunctionHook> steering_advance_hook = nullptr;

    // set these 2 from remote connection
    bool should_override_user_steering_input = false;
    float custom_steering_angle = 0.0f;

    SCSAPI_VOID telemetry_tick(const scs_event_t event, const void *const event_info, scs_context_t context)
    {
        CCore::g_instance->tick();
    }

    /**
     * \brief Hook for prism::game_physics_vehicle_u::steering_advance so we can override the user input
     * \param self /
     * \return /
     */
    uint64_t hk_steering_advance(prism::game_physics_vehicle_u *self)
    {
        if (should_override_user_steering_input)
        {
            self->steering = custom_steering_angle;
        }

        return steering_advance_hook->get_original<prism::game_physics_vehicle_u_steering_advance_fn>()(self);
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

        // Get steering data from the shared memory file
        std::pair<std::array<float, input_float_count>, std::array<bool, input_bool_count>> data = this->read_mem();
        std::array<float, input_float_count> floats = data.first;
        std::array<bool, input_bool_count> bools = data.second;

        should_override_user_steering_input = bools[0];
        custom_steering_angle = floats[0];
    }

    bool CCore::init_truck_steering_manipulation() const
    {
        // using pattern instead of the vtable hook that I used previously, this way we don't have to keep checking in a loop until a vehicle exists
        const auto physics_vehicle_u_steering_advance_fn_address = memory::get_address_for_pattern(patterns::physics_vehicle_u_steering_advance);

        steering_advance_hook = g_instance->get_hooks_manager()->register_function_hook(
            "physics_vehicle_u::steering_advance",
            physics_vehicle_u_steering_advance_fn_address,
            reinterpret_cast<uint64_t>(&hk_steering_advance));

        if (steering_advance_hook->hook() != CHook::HOOKED)
        {
            g_instance->error("Could not hook the physics_vehicle_u::steering_advance function");
            return false;
        }

        g_instance->debug("Found physics_vehicle_u::steering_advance function @ +{:x}", memory::as_offset(physics_vehicle_u_steering_advance_fn_address));

        return true;
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

        this->initialize_mem(); // Initialize shared memory file

        if (this->init_params_->register_for_event(SCS_TELEMETRY_EVENT_frame_end, telemetry_tick, nullptr) != SCS_RESULT_ok)
        {
            this->error("Could not register for frame_end event");
            return false;
        }

        if (!this->init_truck_steering_manipulation())
        {
            this->error("Could not initialize truck steering data");
            return false;
        }

        return true;
    }

    void CCore::destroy()
    {
        delete this->hooks_manager_;
    }
}
