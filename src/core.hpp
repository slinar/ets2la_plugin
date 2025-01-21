#pragma once

#include "scssdk_telemetry.h"
#include "fmt/core.h"

#include "managers/hooks_manager.hpp"

namespace ets2_la_plugin
{

    namespace prism
    {
        class base_ctrl_u;
        class game_actor_u;
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
