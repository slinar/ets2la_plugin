#pragma once

#include <windows.h>
#include "scssdk_telemetry.h"
#include "fmt/core.h"
#include "array"

#include "managers/hooks_manager.hpp"

namespace ets2_la_plugin
{

    namespace prism
    {
        class base_ctrl_u;
        class game_actor_u;
        class traffic_object_t;
    };

    struct InputMemData
    {
        float steering;
        bool override_steering;
        int timestamp;
    };

    struct CameraMemData
    {
        float fov;   // 0
        float pos_x; // 4
        float pos_y; // 8
        float pos_z; // 12
        int16_t cx;  // 16
        int16_t cz;  // 18
        float qw;    // 20
        float qx;    // 24
        float qy;    // 28
        float qz;    // 32
                     // 36
    };

    struct TrafficVehicle
    {
        float x;               // 0
        float y;               // 4
        float z;               // 8
        float qw;              // 12
        float qx;              // 16
        float qy;              // 20
        float qz;              // 24
        float width;           // 28
        float height;          // 32
        float length;          // 36
        float speed;           // 40
        float acceleration;    // 44
        short trailer_count;   // 48
        short id;              // 52
                               // 56
    };

    struct TrafficTrailer
    {
        float x;        // 0
        float y;        // 4
        float z;        // 8
        float qw;       // 12
        float qx;       // 16
        float qy;       // 20
        float qz;       // 24
        float width;    // 28
        float height;   // 32
        float length;   // 36
                        // 40
    };

    struct TrafficVehicleObject
    {
        TrafficVehicle vehicle;      // 0
        TrafficTrailer trailers[2];  // 56
                                     // 136
    };

    struct TrafficMemData
    {
        std::array<TrafficVehicleObject, 20> vehicles; // 2720
    };

    struct SemaphoreObject
    {
        float x;                // 0
        float y;                // 4
        float z;                // 8
        float cx;               // 12
        float cz;               // 16
        float qw;               // 20
        float qx;               // 24
        float qy;               // 28
        float qz;               // 32
        int type;               // 36
        float time_remaining;   // 40
        int state;              // 44
        int id;                 // 48
                                // 52
    };

    struct SemaphoreMemData
    {
        std::array<SemaphoreObject, 20> semaphores; // 1040
    };

    struct RouteTaskObject
    {
        long long uid;  // 0
        float distance; // 8
        float time;     // 12
                        // 16
    };

    struct RouteMemData
    {
        std::array<RouteTaskObject, 5000> tasks; // 80 000
    };

    class CCore
    {
    private:
        const scs_telemetry_init_params_v101_t *init_params_;
        scs_log_t scs_log_;

        mutable size_t last_route_length_{0};

        CHooksManager *hooks_manager_;

    public:
        scs_value_dplacement_t truck_pos;

        // Virtual memory file
        void initialize_memory_file(wchar_t* file_name, wchar_t* format, HANDLE& output_h_map_file) const;
        InputMemData read_input_mem() const;
        void write_camera_mem(const CameraMemData data) const;
        void create_traffic_memory(const wchar_t* traffic_mem_name, HANDLE& traffic_h_map_file) const;
        void create_semaphore_memory(const wchar_t* semaphore_mem_name, HANDLE& semaphore_h_map_file) const;
        void create_route_memory(const wchar_t* route_mem_name, HANDLE& route_h_map_file) const;
        void write_traffic_mem(const TrafficMemData data) const;
        void write_semaphore_mem(const SemaphoreMemData data) const;
        void write_route_mem(const RouteMemData data) const;

        static CCore *g_instance;

        CCore(const scs_telemetry_init_params_v101_t *init_params);
        ~CCore();
        void get_camera_data() const;
        void get_ai_traffic_data() const;

        void get_truckersmp_traffic_data() const;

        void get_traffic_objects_data() const;
        void get_navigation_data() const;

        bool init();
        void destroy();

        void tick() const;

        bool init_truck_steering_manipulation() const;

        CHooksManager *get_hooks_manager() const { return this->hooks_manager_; }

        // TODO: change to file only or something
        template <class... T>
        void debug(const char *fmt_s, T &&...args) const
        {
#ifdef _DEBUG
            scs_log_(0, fmt::vformat(std::string("[ets2_la_plugin] ") + fmt_s, fmt::make_format_args(args...)).c_str());
#endif
        }

        template <class... T>
        void info(const char *fmt_s, T &&...args) const
        {
            scs_log_(0, fmt::vformat(std::string("[ets2_la_plugin] ") + fmt_s, fmt::make_format_args(args...)).c_str());
        }

        template <class... T>
        void warning(const char *fmt_s, T &&...args) const
        {
            scs_log_(1, fmt::vformat(std::string("[ets2_la_plugin] ") + fmt_s, fmt::make_format_args(args...)).c_str());
        }

        template <class... T>
        void error(const char *fmt_s, T &&...args) const
        {
            scs_log_(2, fmt::vformat(std::string("[ets2_la_plugin] ") + fmt_s, fmt::make_format_args(args...)).c_str());
        }
    };
}
