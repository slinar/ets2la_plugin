#include "core.hpp"

#include <MinHook.h>

#include "consts.hpp"

#include "memory/memory_utils.hpp"
#include "sdk/stores.hpp"

#include <common/scssdk_telemetry_truck_common_channels.h>

#include "prism/camera/camera_manager.hpp"
#include "prism/traffic/game_traffic.hpp"
#include "prism/traffic/traffic_ai_trailer.hpp"
#include "prism/traffic/traffic_ai_vehicle.hpp"

#include "hooks/function_hook.hpp"
#include "prism/functions.hpp"
#include "prism/vehicles/game_physics_vehicle.hpp"

#include <ctime>
#include <cmath>

#include <vector>
#include <algorithm>
#include <array>
#include <map>

namespace ets2_la_plugin
{
    // Plugin input memory implementation
    HANDLE input_h_map_file;
    HANDLE camera_h_map_file;
    HANDLE traffic_h_map_file;
    const wchar_t* input_mem_name = L"Local\\ETS2LAPluginInput";
    const wchar_t* camera_mem_name = L"Local\\ETS2LACameraProps";
    const wchar_t* traffic_mem_name = L"Local\\ETS2LATraffic";

    // Initialize a shared memory file.
    // format:
    // f - float
    // b - bool
    // i - int
    // s - short
    void CCore::initialize_memory_file(wchar_t* file_name, wchar_t* format, HANDLE& output_h_map_file) const {
        std::wstring wformat(format);
        std::string sformat(wformat.begin(), wformat.end());
        std::stringstream log_message;
        log_message << "Opening shared memory file with format " << sformat;
        this->info(log_message.str().c_str());

        size_t size = 0;
        for (int i = 0; format[i] != '\0'; i++)
        {
            if (format[i] == 'f')
            {
                size += sizeof(float);
            }
            else if (format[i] == 'b')
            {
                size += sizeof(bool);
            }
            else if (format[i] == 'i')
            {
                size += sizeof(int);
            }
            else if (format[i] == 's')
            {
                size += sizeof(short);
            }
        }
        
        output_h_map_file = CreateFileMapping(
            INVALID_HANDLE_VALUE,    // use paging file
            NULL,                    // default security
            PAGE_READWRITE,          // read/write access
            0,                       // maximum object size (high-order DWORD)
            size,                    // maximum object size (low-order DWORD)
            file_name);                // name of mapping object

        if (output_h_map_file == NULL) {
            DWORD dw = GetLastError();
            std::stringstream ss;
            ss << dw;
            std::string message = "Failed to create file mapping. Error code: " + ss.str();
            this->error(message.c_str());
            return;
        }

        void* pBuf = MapViewOfFile(output_h_map_file, FILE_MAP_ALL_ACCESS, 0, 0, size);

        if (pBuf == NULL) {
            this->error("Failed to map view of file.");
            CloseHandle(output_h_map_file);
            output_h_map_file = NULL;
            return;
        }

        int offset = 0;
        for (int i = 0; format[i] != '\0'; i++)
        {
            if (format[i] == 'f')
            {
                float f = 0.0f;
                memcpy(static_cast<char*>(pBuf) + offset, &f, sizeof(float));
                offset += sizeof(float);
            }
            else if (format[i] == 'b')
            {
                bool b = false;
                memcpy(static_cast<char*>(pBuf) + offset, &b, sizeof(bool));
                offset += sizeof(bool);
            }
            else if (format[i] == 'i')
            {
                int n = 0;
                memcpy(static_cast<char*>(pBuf) + offset, &n, sizeof(int));
                offset += sizeof(int);
            }
            else if (format[i] == 's')
            {
                short s = 0;
                memcpy(static_cast<char*>(pBuf) + offset, &s, sizeof(short));
                offset += sizeof(short);
            }
        }

        UnmapViewOfFile(pBuf);
        
        this->info("Successfully opened shared mem file with size {}", size);
    }

    // Read the input memory file
    InputMemData CCore::read_input_mem() const {
        if (input_h_map_file == NULL) {
            this->error("Shared mem file not open.");
            return InputMemData();
        }

        void* pBuf = MapViewOfFile(input_h_map_file, FILE_MAP_ALL_ACCESS, 0, 0, 9);

        float steering = 0.0f;
        bool override_steering = false;
        int timestamp = 0;

        steering = *reinterpret_cast<float*>(static_cast<char*>(pBuf));
        override_steering = *reinterpret_cast<bool*>(static_cast<char*>(pBuf) + 4);
        timestamp = *reinterpret_cast<int*>(static_cast<char*>(pBuf) + 5);

        UnmapViewOfFile(pBuf);

        return InputMemData{ steering, override_steering, timestamp };
    }

    // Output camera data to the shared memory file
    void CCore::write_camera_mem(const CameraMemData data) const {
        if (camera_h_map_file == NULL) {
            this->error("Shared mem file not open.");
            return;
        }

        void* pBuf = MapViewOfFile(camera_h_map_file, FILE_MAP_ALL_ACCESS, 0, 0, 36);

        memcpy(static_cast<char*>(pBuf), &data.fov, sizeof(float));
        memcpy(static_cast<char*>(pBuf) + 4, &data.pos_x, sizeof(float));
        memcpy(static_cast<char*>(pBuf) + 8, &data.pos_y, sizeof(float));
        memcpy(static_cast<char*>(pBuf) + 12, &data.pos_z, sizeof(float));
        memcpy(static_cast<char*>(pBuf) + 16, &data.cx, sizeof(int16_t));
        memcpy(static_cast<char*>(pBuf) + 18, &data.cz, sizeof(int16_t));
        memcpy(static_cast<char*>(pBuf) + 20, &data.qw, sizeof(float));
        memcpy(static_cast<char*>(pBuf) + 24, &data.qx, sizeof(float));
        memcpy(static_cast<char*>(pBuf) + 28, &data.qy, sizeof(float));
        memcpy(static_cast<char*>(pBuf) + 32, &data.qz, sizeof(float));

        UnmapViewOfFile(pBuf);
    }

    void CCore::write_traffic_mem(const TrafficMemData data) const 
    {
        if (traffic_h_map_file == NULL) 
        {
            this->error("Traffic shared mem file not open.");
            return;
        }

        void* pBuf = MapViewOfFile(traffic_h_map_file, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(TrafficMemData));
        int offset = 0;

        for (int i = 0; i < 20; i++) 
        {
            memcpy(static_cast<char*>(pBuf) + offset, &data.vehicles[i].vehicle.x, sizeof(float));
            memcpy(static_cast<char*>(pBuf) + offset + 4, &data.vehicles[i].vehicle.y, sizeof(float));
            memcpy(static_cast<char*>(pBuf) + offset + 8, &data.vehicles[i].vehicle.z, sizeof(float));
            memcpy(static_cast<char*>(pBuf) + offset + 12, &data.vehicles[i].vehicle.qw, sizeof(float));
            memcpy(static_cast<char*>(pBuf) + offset + 16, &data.vehicles[i].vehicle.qx, sizeof(float));
            memcpy(static_cast<char*>(pBuf) + offset + 20, &data.vehicles[i].vehicle.qy, sizeof(float));
            memcpy(static_cast<char*>(pBuf) + offset + 24, &data.vehicles[i].vehicle.qz, sizeof(float));
            memcpy(static_cast<char*>(pBuf) + offset + 28, &data.vehicles[i].vehicle.width, sizeof(float));
            memcpy(static_cast<char*>(pBuf) + offset + 32, &data.vehicles[i].vehicle.height, sizeof(float));
            memcpy(static_cast<char*>(pBuf) + offset + 36, &data.vehicles[i].vehicle.length, sizeof(float));
            memcpy(static_cast<char*>(pBuf) + offset + 40, &data.vehicles[i].vehicle.speed, sizeof(float));
            memcpy(static_cast<char*>(pBuf) + offset + 44, &data.vehicles[i].vehicle.acceleration, sizeof(float));
            memcpy(static_cast<char*>(pBuf) + offset + 48, &data.vehicles[i].vehicle.trailer_count, sizeof(short));
            memcpy(static_cast<char*>(pBuf) + offset + 50, &data.vehicles[i].vehicle.id, sizeof(short));
            offset += 52;

            for (int j = 0; j < 2; j++) // Trailers
            {
                memcpy(static_cast<char*>(pBuf) + offset, &data.vehicles[i].trailers[j].x, sizeof(float));
                memcpy(static_cast<char*>(pBuf) + offset + 4, &data.vehicles[i].trailers[j].y, sizeof(float));
                memcpy(static_cast<char*>(pBuf) + offset + 8, &data.vehicles[i].trailers[j].z, sizeof(float));
                memcpy(static_cast<char*>(pBuf) + offset + 12, &data.vehicles[i].trailers[j].qw, sizeof(float));
                memcpy(static_cast<char*>(pBuf) + offset + 16, &data.vehicles[i].trailers[j].qx, sizeof(float));
                memcpy(static_cast<char*>(pBuf) + offset + 20, &data.vehicles[i].trailers[j].qy, sizeof(float));
                memcpy(static_cast<char*>(pBuf) + offset + 24, &data.vehicles[i].trailers[j].qz, sizeof(float));
                memcpy(static_cast<char*>(pBuf) + offset + 28, &data.vehicles[i].trailers[j].width, sizeof(float));
                memcpy(static_cast<char*>(pBuf) + offset + 32, &data.vehicles[i].trailers[j].height, sizeof(float));
                memcpy(static_cast<char*>(pBuf) + offset + 36, &data.vehicles[i].trailers[j].length, sizeof(float));
                offset += 40;
            }
        }

        UnmapViewOfFile(pBuf);
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
            CameraMemData data;
            data.fov = current_camera->camera_fov;
            data.pos_x = current_camera->placement.pos.x;
            data.pos_y = current_camera->placement.pos.y;
            data.pos_z = current_camera->placement.pos.z;
            data.cx = current_camera->placement.cx;
            data.cz = current_camera->placement.cz;
            data.qw = current_camera->placement.rot.w;
            data.qx = current_camera->placement.rot.x;
            data.qy = current_camera->placement.rot.y;
            data.qz = current_camera->placement.rot.z;

            this->write_camera_mem(data);
        }
        // or loop all cameras to get data from all of them
        for (const auto *camera : camera_manager->cameras)
        {
            // ...
        }
    }

    #include <map> // Include map instead of unordered_map

    void CCore::get_ai_traffic_data() const
    {
        auto* game_traffic = prism::game_traffic_u::get();

        if (game_traffic == nullptr)
            return;

        const auto truck_pos = this->truck_pos; // our truck position from the SDK telemetry
        const auto truck_x = truck_pos.position.x;
        const auto truck_y = truck_pos.position.y;
        const auto truck_z = truck_pos.position.z;

        struct ai_vehicle_sort {
            const prism::spawned_vehicle_t* ai_vehicle;
            float distance;
            int id;
        };

        std::vector<ai_vehicle_sort> sorted_ai_vehicles;

        // Static map to store unique IDs for each vehicle
        static std::map<const prism::spawned_vehicle_t*, int> vehicle_uids;
        static int next_vehicle_id = 0;

        int i = 0;
        for (const auto& ai_vehicle : game_traffic->ai_vehicles)
        {
            if (ai_vehicle.vehicle == nullptr || ai_vehicle.vehicle->traffic_vehicle == nullptr || ai_vehicle.vehicle->physics_data == nullptr)
                continue;

            const auto ai_x = ai_vehicle.vehicle->placement.cx * 512 + ai_vehicle.vehicle->placement.pos.x;
            const auto ai_y = ai_vehicle.vehicle->placement.pos.y;
            const auto ai_z = ai_vehicle.vehicle->placement.cz * 512 + ai_vehicle.vehicle->placement.pos.z;

            const float dx = ai_x - truck_x;
            const float dy = ai_y - truck_y;
            const float dz = ai_z - truck_z;
            const float distance = std::sqrt(dx * dx + dy * dy + dz * dz);

            // Assign a persistent unique ID if the vehicle doesn't already have one
            if (vehicle_uids.find(&ai_vehicle) == vehicle_uids.end()) {
                vehicle_uids[&ai_vehicle] = next_vehicle_id++;
            }

            sorted_ai_vehicles.push_back({ &ai_vehicle, distance, vehicle_uids[&ai_vehicle] });
            i++;
        }

        // Sort the vector based on the distance
        std::sort(sorted_ai_vehicles.begin(), sorted_ai_vehicles.end(), [](const ai_vehicle_sort& a, const ai_vehicle_sort& b) {
            return a.distance < b.distance;
        });

        std::array<TrafficVehicleObject, 20> vehicles = {};

        i = 0;
        for (const auto& vehicle_data : sorted_ai_vehicles)
        {
            if (i >= 20)
            {
                break;
            }

            TrafficVehicleObject vehicle_object = {};
            TrafficVehicle& vehicle = vehicle_object.vehicle;
            const auto& ai_vehicle = *vehicle_data.ai_vehicle;

            vehicle.x = ai_vehicle.vehicle->placement.cx * 512 + ai_vehicle.vehicle->placement.pos.x;
            vehicle.y = ai_vehicle.vehicle->placement.pos.y;
            vehicle.z = ai_vehicle.vehicle->placement.cz * 512 + ai_vehicle.vehicle->placement.pos.z;

            vehicle.qw = ai_vehicle.vehicle->placement.rot.w;
            vehicle.qx = ai_vehicle.vehicle->placement.rot.x;
            vehicle.qy = ai_vehicle.vehicle->placement.rot.y;
            vehicle.qz = ai_vehicle.vehicle->placement.rot.z;

            vehicle.width = ai_vehicle.vehicle->physics_data->bounding_box.x;
            vehicle.height = ai_vehicle.vehicle->physics_data->bounding_box.y;
            vehicle.length = ai_vehicle.vehicle->physics_data->bounding_box.z;

            vehicle.speed = ai_vehicle.vehicle->physics_data->speed;
            vehicle.acceleration = ai_vehicle.vehicle->physics_data->acceleration;

            vehicle.trailer_count = 0;

            vehicle.id = vehicle_data.id; // Persistent unique ID

            const auto* trailer = ai_vehicle.vehicle->trailer;

            while (trailer != nullptr)
            {
                if (trailer->physics_data == nullptr)
                {
                    break;
                }

                if (vehicle.trailer_count >= 2)
                {
                    break;
                }

                TrafficTrailer& trailer_data = vehicle_object.trailers[vehicle.trailer_count];

                trailer_data.x = trailer->placement.cx * 512 + trailer->placement.pos.x;
                trailer_data.y = trailer->placement.pos.y;
                trailer_data.z = trailer->placement.cz * 512 + trailer->placement.pos.z;

                trailer_data.qw = trailer->placement.rot.w;
                trailer_data.qx = trailer->placement.rot.x;
                trailer_data.qy = trailer->placement.rot.y;
                trailer_data.qz = trailer->placement.rot.z;

                trailer_data.width = trailer->physics_data->bounding_box.x;
                trailer_data.height = trailer->physics_data->bounding_box.y;
                trailer_data.length = trailer->physics_data->bounding_box.z;

                trailer = trailer->slave_trailer;

                vehicle.trailer_count++;
            }

            vehicles[i] = vehicle_object;
            i++;
        }

        TrafficMemData data = { vehicles };
        this->write_traffic_mem(data);
    }

    void CCore::tick() const
    {
        this->get_camera_data();
        this->get_ai_traffic_data();

        // Get steering data from the shared memory file
        InputMemData data = this->read_input_mem();

        should_override_user_steering_input = data.override_steering;
        custom_steering_angle = data.steering;

        int timestamp = data.timestamp;
        int current_time = std::time(0);
        if (current_time - timestamp > 1) // Data is over a second old
        {
            should_override_user_steering_input = false;
        }
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

    void CCore::create_traffic_memory(const wchar_t* traffic_mem_name, HANDLE& traffic_h_map_file) const {
        //                   xyz    whl  tc
        wchar_t* vehicle = L"ffffffffffffss"; // 52 bytes
        //                      wxyz  sa  id

        //                   xyz    whl
        wchar_t* trailer = L"ffffffffff";    // 40 bytes
        //                      wxyz

        // Concatenate vehicle + trailer + trailer
        size_t vehicle_len = wcslen(vehicle);
        size_t trailer_len = wcslen(trailer);
        size_t vehicle_object_len = vehicle_len + 2 * trailer_len + 1; // +1 for null terminator

        wchar_t* vehicle_object = (wchar_t*)malloc(vehicle_object_len * sizeof(wchar_t));
        if (!vehicle_object) {
            this->error("Memory allocation failed for vehicle_object");
            return;
        }

        wcscpy(vehicle_object, vehicle); // Copy vehicle
        wcscat(vehicle_object, trailer); // Append trailer
        wcscat(vehicle_object, trailer); // Append trailer again

        int vehicle_count = 20;
        size_t total_len = vehicle_count * vehicle_object_len + 1; // +1 for null terminator

        wchar_t* total_vehicle_format = (wchar_t*)malloc(total_len * sizeof(wchar_t));
        if (!total_vehicle_format) {
            this->error("Memory allocation failed for total_vehicle_format");
            free(vehicle_object);
            return;
        }

        total_vehicle_format[0] = L'\0'; // Initialize as an empty string
        for (int i = 0; i < vehicle_count; i++) {
            wcscat(total_vehicle_format, vehicle_object);
        }

        this->initialize_memory_file(const_cast<wchar_t*>(traffic_mem_name), total_vehicle_format, traffic_h_map_file);
        free(vehicle_object);
        free(total_vehicle_format);
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

        // ETS2LA specific memory files
        this->initialize_memory_file(const_cast<wchar_t*>(input_mem_name), L"fbi", input_h_map_file);
        this->initialize_memory_file(const_cast<wchar_t*>(camera_mem_name), L"ffffssffff", camera_h_map_file);
        this->create_traffic_memory(const_cast<wchar_t*>(traffic_mem_name), traffic_h_map_file);

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
