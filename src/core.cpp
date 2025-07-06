#include "core.hpp"

#include <MinHook.h>

#include "consts.hpp"

#include "memory/memory_utils.hpp"
#include "sdk/stores.hpp"

#include <common/scssdk_telemetry_truck_common_channels.h>

#include "prism/controllers/base_ctrl.hpp"
#include "prism/controllers/game_ctrl.hpp"
#include "prism/camera/camera_manager.hpp"
#include "prism/traffic/game_traffic.hpp"
#include "prism/traffic/traffic_ai_trailer.hpp"
#include "prism/traffic/traffic_ai_vehicle.hpp"
#include "prism/traffic/traffic_objects.hpp"
#include "prism/traffic/traffic_rules.hpp"

#include "prism/management/item/kdop_item.hpp"
#include "prism/management/item/node_item.hpp"
#include "prism/management/item/prefab_item.hpp"
#include "prism/management/item/segment.hpp"
#include "prism/management/item/semaphore_instance.hpp"

#include "prism/navigation/route_task.hpp"

#include "hooks/function_hook.hpp"
#include "prism/functions.hpp"
#include "prism/vehicles/game_physics_vehicle.hpp"
#include "prism/vehicles/game_trailer_actor.hpp"
#include "prism/game_actor.hpp"

#include <ctime>
#include <cmath>

#include <vector>
#include <algorithm>
#include <array>
#include <map>

namespace ets2_la_plugin
{
    CCore *CCore::g_instance = nullptr;
    std::shared_ptr<CFunctionHook> steering_advance_hook = nullptr;

    // set these 2 from remote connection
    bool should_override_user_input = false;
    float custom_steering_angle = 0.0f;
    float custom_throttle_input = 0.0f;
    float custom_brake_input = 0.0f;

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
        if (should_override_user_input)
        {
            self->set_steering_angle(custom_steering_angle);

            auto* game_actor = prism::game_actor_u::get();
            if (game_actor != nullptr)
            {
                game_actor->set_throttle_input(custom_throttle_input);
                game_actor->set_brake_input(custom_brake_input);
            }
        }

        return steering_advance_hook->get_original<prism::game_physics_vehicle_u_steering_advance_fn>()(self);
    }

    CCore::CCore(const scs_telemetry_init_params_v101_t *init_params) : init_params_(init_params)
    {
        this->hooks_manager_ = new CHooksManager();
        scs_log_ = init_params->common.log;
        memory_manager_ = new CMemoryHandler(scs_log_);
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

            this->memory_manager_->write_camera_mem(data);
        }
    }

    void CCore::get_ai_traffic_data() const
    {
        auto* game_traffic = prism::game_traffic_u::get();

        if (game_traffic == nullptr)
            return;

        // our truck position from the SDK telemetry
        const auto truck_pos = this->truck_pos;
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

        std::array<TrafficVehicleObject, 40> vehicles = {};

        i = 0;
        for (const auto& vehicle_data : sorted_ai_vehicles)
        {
            if (i >= 40)
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
        this->memory_manager_->write_traffic_mem(data);
    }

    bool CCore::get_truckersmp_traffic_data() const
    {
        auto* game_ctrl = prism::game_ctrl_u::get();
    
        if (game_ctrl == nullptr)
        {
            return false;
        }
    
        // Get current truck position for distance calculation
        const auto truck_pos = this->truck_pos;
        const auto truck_x = truck_pos.position.x;
        const auto truck_y = truck_pos.position.y;
        const auto truck_z = truck_pos.position.z;
        
        struct mp_vehicle_sort {
            float x, y, z;               // Position
            float qw, qx, qy, qz;        // Rotation
            float width, height, length; // Dimensions
            float speed;                 // Speed (set to 0, and then calculated on ETS2LA side)
            float distance;              // Distance from player
            int id;                      // Unique ID (generated)
            bool is_trailer;             // Whether this is a trailer
        };
    
        static prism::unit_descriptor_t *stored_game_trailer_actor_unit_descriptor = nullptr;
        
        // Collection of TMP trucks
        std::vector<mp_vehicle_sort> sorted_mp_vehicles;
        
        // Static map for unique IDs
        static std::map<const void*, int> mp_vehicle_uids;
        static int next_mp_vehicle_id = 1000; // From 1000 to differentiate from AI traffic

        const auto* vehicles_list = game_ctrl->get_some_nearby_non_ai_vehicles_list();
        if (vehicles_list == nullptr)
        {
            return false; // No vehicles list available
        }
    
        auto* node = vehicles_list->begin;
        while(node->item != vehicles_list->empty_item)
        {
            const auto unit_descriptor = node->item->get_unit_descriptor();
    
            if (stored_game_trailer_actor_unit_descriptor == nullptr)
            {
                // check if the node item is of type 'game_trailer_actor' and store the address of the unit descriptor if it is
                if (strcmp(*unit_descriptor->p_class_name, "game_trailer_actor") == 0)
                {
                    stored_game_trailer_actor_unit_descriptor = unit_descriptor;
                }
            }
    
            const auto is_trailer = stored_game_trailer_actor_unit_descriptor == unit_descriptor;
    
            mp_vehicle_sort vehicle_data = {};
            vehicle_data.is_trailer = is_trailer;
            
            // Unique ID if not already assigned
            if (mp_vehicle_uids.find(node->item) == mp_vehicle_uids.end()) {
                mp_vehicle_uids[node->item] = next_mp_vehicle_id++;
            }
            vehicle_data.id = mp_vehicle_uids[node->item];
    
            if (is_trailer)
            {
                auto *trailer = static_cast<const prism::game_trailer_actor_u *>(node->item);
                prism::placement_t trailer_placement;
                trailer->get_physics_placement(&trailer_placement);
    
                vehicle_data.x = trailer_placement.cx * 512 + trailer_placement.pos.x;
                vehicle_data.y = trailer_placement.pos.y;
                vehicle_data.z = trailer_placement.cz * 512 + trailer_placement.pos.z;
    
                vehicle_data.qw = trailer_placement.rot.w;
                vehicle_data.qx = trailer_placement.rot.x;
                vehicle_data.qy = trailer_placement.rot.y;
                vehicle_data.qz = trailer_placement.rot.z;
    
                vehicle_data.width = trailer->dimensions.end.x - trailer->dimensions.start.x;
                vehicle_data.height = trailer->dimensions.end.y - trailer->dimensions.start.y;
                vehicle_data.length = trailer->dimensions.end.z - trailer->dimensions.start.z;
                
                vehicle_data.speed = -1.0f;
                
                // Distance to player
                const float dx = vehicle_data.x - truck_x;
                const float dy = vehicle_data.y - truck_y;
                const float dz = vehicle_data.z - truck_z;
                vehicle_data.distance = std::sqrt(dx * dx + dy * dy + dz * dz);
                
                sorted_mp_vehicles.push_back(vehicle_data);
            }
            else
            {
                auto *truck = static_cast<const prism::game_physics_vehicle_u *>(node->item);
                prism::placement_t truck_placement;
                truck->get_physics_placement(&truck_placement);
    
                vehicle_data.x = truck_placement.cx * 512 + truck_placement.pos.x;
                vehicle_data.y = truck_placement.pos.y;
                vehicle_data.z = truck_placement.cz * 512 + truck_placement.pos.z;
    
                vehicle_data.qw = truck_placement.rot.w;
                vehicle_data.qx = truck_placement.rot.x;
                vehicle_data.qy = truck_placement.rot.y;
                vehicle_data.qz = truck_placement.rot.z;
    
                vehicle_data.width = truck->dimensions.end.x - truck->dimensions.start.x;
                vehicle_data.height = truck->dimensions.end.y - truck->dimensions.start.y;
                vehicle_data.length = truck->dimensions.end.z - truck->dimensions.start.z;
                
                vehicle_data.speed = -1.0f;

                // Distance to player
                const float dx = vehicle_data.x - truck_x;
                const float dy = vehicle_data.y - truck_y;
                const float dz = vehicle_data.z - truck_z;
                vehicle_data.distance = std::sqrt(dx * dx + dy * dy + dz * dz);
                
                sorted_mp_vehicles.push_back(vehicle_data);
            }
    
            node = node->next;
        }
        
        if (sorted_mp_vehicles.empty()) {
            return false;
        }
        
        // Sort by distance
        std::sort(sorted_mp_vehicles.begin(), sorted_mp_vehicles.end(), 
            [](const mp_vehicle_sort& a, const mp_vehicle_sort& b) {
                return a.distance < b.distance;
        });

        // Create traffic objects and add to memory
        TrafficMemData traffic_data = {};
        int max_vehicles = 40;
        int cur_count = 0;
        for (const auto& mp_vehicle : sorted_mp_vehicles) {
            if (cur_count >= max_vehicles) {
                break;
            }
            
            TrafficVehicleObject vehicle_object = {};
            TrafficVehicle& vehicle = vehicle_object.vehicle;
            
            // Fill in the truck data
            vehicle.x = mp_vehicle.x;
            vehicle.y = mp_vehicle.y;
            vehicle.z = mp_vehicle.z;
            
            vehicle.qw = mp_vehicle.qw;
            vehicle.qx = mp_vehicle.qx;
            vehicle.qy = mp_vehicle.qy;
            vehicle.qz = mp_vehicle.qz;
            
            vehicle.width = mp_vehicle.width;
            vehicle.height = mp_vehicle.height;
            vehicle.length = mp_vehicle.length;
            
            vehicle.speed = mp_vehicle.speed;

            // Acceleration is used to differentiate between truck and trailer
            if (mp_vehicle.is_trailer)
            {
                vehicle.acceleration = -2.0f;
            }
            else
            {
                vehicle.acceleration = -1.0f;
            }
            
            vehicle.id = mp_vehicle.id;
            vehicle.trailer_count = 0;
            
            // Add to the data array
            traffic_data.vehicles[cur_count] = vehicle_object;
            cur_count++;
        }
        
        if (cur_count > 0) {
            // Write the data to memory
            this->memory_manager_->write_traffic_mem(traffic_data);
            return true;
        }
        return false;
    }

    void CCore::get_traffic_objects_data() const
    {

        struct TrafficObjectData
        {
            const prism::traffic_semaphore_actor_t *semaphore_actor;
            float distance;
            int id;
        };

        auto* base_ctrl = prism::base_ctrl_u::get();

        if (base_ctrl == nullptr)
            return;

        const auto truck_pos = this->truck_pos;
        const auto truck_x = truck_pos.position.x;
        const auto truck_y = truck_pos.position.y;
        const auto truck_z = truck_pos.position.z;

        std::vector<TrafficObjectData> traffic_objects_with_distance;

        // Gather traffic lights and gates
        for (const auto *kdop_item : base_ctrl->some_nearby_kdop_items)
        {
            if (kdop_item->item_type != 4) // we only want prefabs
            {
                if (kdop_item->item_type > 4)
                {
                    break; // items seem to be ordered by item type, so we can stop looping after going through all prefabs
                }
                continue;
            }
            const auto prefab_kdop_item = static_cast<const prism::prefab_item_t *>(kdop_item);

            if (prefab_kdop_item->segment == nullptr)
            {
                continue;
            }

            for (const auto &semaphore_instance : prefab_kdop_item->segment->semaphore_instances)
            {
                if (semaphore_instance.actor == nullptr || semaphore_instance.actor->get_type() != 0x07) // we only want type 0x07 (traffic_semaphore_actor_t)
                {
                    continue;
                }
                const auto* semaphore_actor = static_cast<const prism::traffic_semaphore_actor_t*>(semaphore_instance.actor);

                // Get distance
                const float object_x = semaphore_actor->placement.pos.x + 512 * semaphore_actor->placement.cx;
                const float object_y = semaphore_actor->placement.pos.y;
                const float object_z = semaphore_actor->placement.pos.z + 512 * semaphore_actor->placement.cz;

                const float dx = object_x - truck_x;
                const float dy = object_y - truck_y;
                const float dz = object_z - truck_z;
                const float distance = std::sqrt(dx * dx + dy * dy + dz * dz);

                const int id = static_cast<int>(semaphore_instance.id);

                traffic_objects_with_distance.push_back({semaphore_actor, distance, id});
            }
        }

        auto* game_traffic = prism::game_traffic_u::get();

        if (game_traffic != nullptr)
        {
            for (const auto* traffic_object : game_traffic->traffic_objects)
            {
                if (traffic_object->get_type() != 0x07)
                {
                    continue; // we only want type 0x07 (traffic_semaphore_actor_t)
                }
               const auto* semaphore_actor = static_cast<const prism::traffic_semaphore_actor_t*>(traffic_object);

                // Get distance
                const float object_x = semaphore_actor->placement.pos.x + 512 * semaphore_actor->placement.cx;
                const float object_y = semaphore_actor->placement.pos.y;
                const float object_z = semaphore_actor->placement.pos.z + 512 * semaphore_actor->placement.cz;

                const float dx = object_x - truck_x;
                const float dy = object_y - truck_y;
                const float dz = object_z - truck_z;
                const float distance = std::sqrt(dx * dx + dy * dy + dz * dz);

                traffic_objects_with_distance.push_back({semaphore_actor, distance, 0});
            }
        }

        // Sort semaphores by distance
        std::sort(traffic_objects_with_distance.begin(), traffic_objects_with_distance.end(), [](const TrafficObjectData& a, const TrafficObjectData& b) {
            return a.distance < b.distance;
        });

        std::array<SemaphoreObject, 40> semaphores = {};

        // Process the sorted traffic objects
        int i = 0;
        for (const auto& traffic_object_data : traffic_objects_with_distance)
        {
            if (i >= 40)
            {
                break;
            }

            const auto* semaphore_actor = traffic_object_data.semaphore_actor;
            const float distance = traffic_object_data.distance;
            const int id = traffic_object_data.id;

            SemaphoreObject semaphore_object = {};
            semaphore_object.x = semaphore_actor->placement.pos.x;
            semaphore_object.y = semaphore_actor->placement.pos.y;
            semaphore_object.z = semaphore_actor->placement.pos.z;
            semaphore_object.cx = semaphore_actor->placement.cx;
            semaphore_object.cz = semaphore_actor->placement.cz;
            semaphore_object.qw = semaphore_actor->placement.rot.w;
            semaphore_object.qx = semaphore_actor->placement.rot.x;
            semaphore_object.qy = semaphore_actor->placement.rot.y;
            semaphore_object.qz = semaphore_actor->placement.rot.z;
            semaphore_object.id = id;

            if (semaphore_actor->traffic_rule != nullptr && semaphore_actor->traffic_rule->get_type() == prism::traffic_light_t::ID)
            {
                const auto* traffic_light = static_cast<const prism::traffic_light_t*>(semaphore_actor->traffic_rule);

                semaphore_object.type = 1; // 1 = traffic light
                semaphore_object.time_remaining = traffic_light->state_time_remaining;
                // 0 = off, 1 = orange_to_red, 2 = red, 4 = orange_to_green, 8 = green, 32 = sleep (blinking orange)
                semaphore_object.state = traffic_light->state;
            }
            else // gates
            {
                // state // 0 = closing; 1 = closed; 3 = open; 2 = opening
                semaphore_object.type = 2; // 2 = gate
                semaphore_object.state = semaphore_actor->state;

                if (semaphore_actor->animation_mode == 2)
                {
                    semaphore_object.time_remaining = semaphore_actor->open_time_remaining;
                } else {
                    semaphore_object.time_remaining = semaphore_actor->animation_time_elapsed;
                }
            }

            semaphores[i] = semaphore_object;
            i++;
        }

        SemaphoreMemData data = { semaphores };
        this->memory_manager_->write_semaphore_mem(data);
    }

    void CCore::get_navigation_data() const
    {
        auto *gps_manager = prism::gps_manager_t::get();

        if (gps_manager == nullptr)
        {
            return;
        }

        // route_task is nullptr when no route is set
        if (gps_manager->simple_route_source.route_task != nullptr)
        {
            size_t n = 0;
            for (const auto &route_item : gps_manager->simple_route_source.route_task->physical_route_items)
            {
                n++;
            }

            if (n != last_route_length_)
            {
                last_route_length_ = n;
                std::array<RouteTaskObject, 6000> route_tasks = {};
                int i = 0;
                for (const auto &route_item : gps_manager->simple_route_source.route_task->physical_route_items)
                {
                    RouteTaskObject task = {};
                    prism::node_item_t *node = route_item.node;
                    task.uid = node->uid;

                    task.distance = route_item.total_distance_till_end;
                    task.time = route_item.total_time_till_end;

                    route_tasks[i] = task;
                    i++;
                }

                RouteMemData data = { route_tasks };
                this->memory_manager_->write_route_mem(data);
            }
        }

        for (const auto &waypoint : gps_manager->waypoints)
        {
            // float x = waypoint.node->coords.x / 256.f;
            // float y = waypoint.node->coords.y / 256.f;
            // float z = waypoint.node->coords.z / 256.f;
        }
        for (const auto &waypoint : gps_manager->avoid_waypoints)
        {
            // float x = waypoint.node->coords.x / 256.f;
            // float y = waypoint.node->coords.y / 256.f;
            // float z = waypoint.node->coords.z / 256.f;
        }
    }

    void CCore::tick() const
    {
        int start_time = std::time(0);
        this->get_camera_data();

        // Try and get truckersmp traffic data first
        if(!this->get_truckersmp_traffic_data())
        {
            // Then get normal AI traffic data
            this->get_ai_traffic_data();
        }

        this->get_traffic_objects_data();
        this->get_navigation_data();

        // Get input data from the shared memory file
        InputMemData data = this->memory_manager_->read_input_mem();

        should_override_user_input = data.override_input;
        custom_steering_angle = data.steering;
        custom_throttle_input = data.throttle;
        custom_brake_input = data.brake;

        int timestamp = data.timestamp;
        int current_time = std::time(0);
        if (current_time - timestamp > 1) // Data is over a second old
        {
            should_override_user_input = false;
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

    bool CCore::scan_for_required_patterns()
    {
        if ( prism::base_ctrl_u::scan_patterns() )
        {
            CCore::g_instance->debug(
                "Found base_ctrl @ +{:x}", memory::as_offset( prism::base_ctrl_u::instance_ptr_address )
            );
        }
        else
        {
            this->error( "Could not find base_ctrl patterns" );
            return false;
        }

        try
        {
            prism::game_ctrl_u::scan_patterns();
        }
        catch( std::exception& e )
        {
            this->error("Error when scanning game_ctrl memory patterns: {}", e.what());
            return false;
        }

        if ( prism::camera_manager_u::scan_patterns() )
        {
            CCore::g_instance->debug(
                "Found camera_manager @ +{:x}", memory::as_offset( prism::camera_manager_u::instance_ptr_address )
            );
        }
        else
        {
            this->error( "Could not find camera_manager patterns" );
            return false;
        }

        if ( prism::game_traffic_u::scan_patterns() )
        {
            CCore::g_instance->debug(
                "Found game_traffic @ +{:x}", memory::as_offset( prism::game_traffic_u::instance_ptr_address )
            );
        }
        else
        {
            this->error( "Could not find game_traffic patterns" );
            return false;
        }

        if ( !prism::vehicle_shared_u::scan_patterns() )
        {
            this->error( "Could not find vehicle_shared patterns" );
            return false;
        }

        try
        {
            prism::game_actor_u::scan_patterns();
        }
        catch( std::exception& e )
        {
            this->error("Error when scanning game_actor memory patterns: {}", e.what());
            return false;
        }

        return true;
    }

    bool CCore::init()
    {
        MH_Initialize();
        this->info("Initializing {}", VERSION);

        if ( !this->scan_for_required_patterns() )
        {
            this->error( "Could not find required memory patterns" );
            return false;
        }

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

        // Initialize memory manager
        this->memory_manager_->init();

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
        delete this->memory_manager_;
        delete this->hooks_manager_;
    }
}
