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
        float x;
        float y;
        float z;
        float qw;
        float qx;
        float qy;
        float qz;
        float width;
        float height;
        float length;
        float speed;
        float acceleration;
        short trailer_count;
    };

    struct TrafficTrailer
    {
        float x;
        float y;
        float z;
        float qw;
        float qx;
        float qy;
        float qz;
        float width;
        float height;
        float length;
    };

    struct TrafficVehicleObject
    {
        TrafficVehicle vehicle;
        TrafficTrailer trailers[2];
    };

    struct TrafficMemData
    {
        std::array<TrafficVehicleObject, 20> vehicles;
    };

    class CCore
    {
    private:
        const scs_telemetry_init_params_v101_t *init_params_;
        scs_log_t scs_log_;

        CHooksManager *hooks_manager_;

    public:
        // just putting this here whatever
        scs_value_dplacement_t truck_pos;

        // Virtual memory file
        void initialize_memory_file(wchar_t* file_name, wchar_t* format, HANDLE& output_h_map_file) const;
        InputMemData read_input_mem() const;
        void write_camera_mem(const CameraMemData data) const;
        void create_traffic_memory(const wchar_t* traffic_mem_name, HANDLE& traffic_h_map_file) const;
        void write_traffic_mem(const TrafficMemData data) const;

        static CCore *g_instance;

        CCore(const scs_telemetry_init_params_v101_t *init_params);
        ~CCore();
        void get_camera_data() const;
        void get_ai_traffic_data() const;

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
