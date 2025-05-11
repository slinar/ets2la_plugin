#pragma once

#include <windows.h>
#include "scssdk_telemetry.h"
#include "fmt/core.h"
#include "array"

#include "managers/hooks_manager.hpp"
#include "memory/virtual/memory_handler.hpp"

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
        short id;              // 50
                               // 52
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
        TrafficTrailer trailers[2];  // 52
                                     // 132
    };

    struct TrafficMemData
    {
        std::array<TrafficVehicleObject, 40> vehicles; // 5280
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
        std::array<SemaphoreObject, 40> semaphores; // 2080
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
        std::array<RouteTaskObject, 6000> tasks; // 96 000
    };

    class CCore
    {
    private:
        const scs_telemetry_init_params_v101_t *init_params_;
        scs_log_t scs_log_;

        mutable size_t last_route_length_{0};

        CHooksManager *hooks_manager_;
        CMemoryHandler *memory_manager_;

    public:
        scs_value_dplacement_t truck_pos;

        static CCore *g_instance;

        CCore(const scs_telemetry_init_params_v101_t *init_params);
        ~CCore();
        void get_camera_data() const;
        void get_ai_traffic_data() const;

        bool get_truckersmp_traffic_data() const;

        void get_traffic_objects_data() const;
        void get_navigation_data() const;

        bool init();
        void destroy();

        void tick() const;

        bool init_truck_steering_manipulation() const;

        CHooksManager *get_hooks_manager() const { return this->hooks_manager_; }
        CMemoryHandler *get_memory_manager() const { return this->memory_manager_; }

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