#include "memory/virtual/memory_handler.hpp"
#include "core.hpp"
#include <sstream>

namespace ets2_la_plugin
{
    CMemoryHandler::CMemoryHandler(scs_log_t scs_log) : scs_log_(scs_log),
        input_h_map_file(NULL),
        camera_h_map_file(NULL),
        traffic_h_map_file(NULL),
        semaphore_h_map_file(NULL),
        route_h_map_file(NULL)
    {
    }

    CMemoryHandler::~CMemoryHandler()
    {
        destroy();
    }

    bool CMemoryHandler::init()
    {
        this->initialize_memory_file(const_cast<wchar_t*>(input_mem_name), L"fbi", input_h_map_file);
        this->initialize_memory_file(const_cast<wchar_t*>(camera_mem_name), L"ffffssffff", camera_h_map_file);
        this->create_traffic_memory();
        this->create_semaphore_memory();
        this->create_route_memory();
        
        return true;
    }

    void CMemoryHandler::destroy()
    {
        // Close all handles
        if (input_h_map_file != NULL) {
            CloseHandle(input_h_map_file);
            input_h_map_file = NULL;
        }
        
        if (camera_h_map_file != NULL) {
            CloseHandle(camera_h_map_file);
            camera_h_map_file = NULL;
        }
        
        if (traffic_h_map_file != NULL) {
            CloseHandle(traffic_h_map_file);
            traffic_h_map_file = NULL;
        }
        
        if (semaphore_h_map_file != NULL) {
            CloseHandle(semaphore_h_map_file);
            semaphore_h_map_file = NULL;
        }
        
        if (route_h_map_file != NULL) {
            CloseHandle(route_h_map_file);
            route_h_map_file = NULL;
        }
    }

    // Initialize a shared memory file.
    // format:
    // f - float
    // b - bool
    // i - int
    // s - short
    // l - long long
    void CMemoryHandler::initialize_memory_file(wchar_t* file_name, wchar_t* format, HANDLE& output_h_map_file) const {
        std::wstring wformat(format);
        std::string sformat(wformat.begin(), wformat.end());
        std::stringstream log_message;
        log_message << "Opening shared memory file with format " << sformat;
        this->info(log_message.str().c_str());

        const char float_type = 'f';
        const char boolean_type = 'b';
        const char integer_type = 'i';
        const char short_type = 's';
        const char long_long_type = 'l';

        size_t size = 0;
        for (int i = 0; format[i] != '\0'; i++)
        {
            if (format[i] == float_type)
            {
                size += sizeof(float);
            }
            else if (format[i] == boolean_type)
            {
                size += sizeof(bool);
            }
            else if (format[i] == integer_type)
            {
                size += sizeof(int);
            }
            else if (format[i] == short_type)
            {
                size += sizeof(short);
            }
            else if (format[i] == long_long_type)
            {
                size += sizeof(long long);
            }
        }

        output_h_map_file = CreateFileMapping(
            INVALID_HANDLE_VALUE,    // use paging file
            NULL,                    // default security
            PAGE_READWRITE,          // read/write access
            0,                       // maximum object size (high-order DWORD)
            size,                    // maximum object size (low-order DWORD)
            file_name);              // name of mapping object

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
            if (format[i] == float_type)
            {
                float f = 0.0f;
                memcpy(static_cast<char*>(pBuf) + offset, &f, sizeof(float));
                offset += sizeof(float);
            }
            else if (format[i] == boolean_type)
            {
                bool b = false;
                memcpy(static_cast<char*>(pBuf) + offset, &b, sizeof(bool));
                offset += sizeof(bool);
            }
            else if (format[i] == integer_type)
            {
                int n = 0;
                memcpy(static_cast<char*>(pBuf) + offset, &n, sizeof(int));
                offset += sizeof(int);
            }
            else if (format[i] == short_type)
            {
                short s = 0;
                memcpy(static_cast<char*>(pBuf) + offset, &s, sizeof(short));
                offset += sizeof(short);
            }
            else if (format[i] == long_long_type)
            {
                long long l = 0;
                memcpy(static_cast<char*>(pBuf) + offset, &l, sizeof(long long));
                offset += sizeof(long long);
            }
        }

        UnmapViewOfFile(pBuf);

        this->info("Successfully opened shared mem file with size {}", size);
    }

    // Read the input memory file
    InputMemData CMemoryHandler::read_input_mem() const {
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
    void CMemoryHandler::write_camera_mem(const CameraMemData data) const {
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

    void CMemoryHandler::write_traffic_mem(const TrafficMemData data) const
    {
        if (traffic_h_map_file == NULL)
        {
            this->error("Traffic shared mem file not open.");
            return;
        }

        void* pBuf = MapViewOfFile(traffic_h_map_file, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(TrafficMemData));
        int offset = 0;

        for (int i = 0; i < 40; i++)
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

    void CMemoryHandler::write_semaphore_mem(const SemaphoreMemData data) const
    {
        if (semaphore_h_map_file == NULL)
        {
            this->error("Semaphore shared mem file not open.");
            return;
        }

        void* pBuf = MapViewOfFile(semaphore_h_map_file, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SemaphoreMemData));
        int offset = 0;

        for (int i = 0; i < 40; i++)
        {
            memcpy(static_cast<char*>(pBuf) + offset, &data.semaphores[i].x, sizeof(float));
            memcpy(static_cast<char*>(pBuf) + offset + 4, &data.semaphores[i].y, sizeof(float));
            memcpy(static_cast<char*>(pBuf) + offset + 8, &data.semaphores[i].z, sizeof(float));
            memcpy(static_cast<char*>(pBuf) + offset + 12, &data.semaphores[i].cx, sizeof(float));
            memcpy(static_cast<char*>(pBuf) + offset + 16, &data.semaphores[i].cz, sizeof(float));
            memcpy(static_cast<char*>(pBuf) + offset + 20, &data.semaphores[i].qw, sizeof(float));
            memcpy(static_cast<char*>(pBuf) + offset + 24, &data.semaphores[i].qx, sizeof(float));
            memcpy(static_cast<char*>(pBuf) + offset + 28, &data.semaphores[i].qy, sizeof(float));
            memcpy(static_cast<char*>(pBuf) + offset + 32, &data.semaphores[i].qz, sizeof(float));
            memcpy(static_cast<char*>(pBuf) + offset + 36, &data.semaphores[i].type, sizeof(int));
            memcpy(static_cast<char*>(pBuf) + offset + 40, &data.semaphores[i].time_remaining, sizeof(float));
            memcpy(static_cast<char*>(pBuf) + offset + 44, &data.semaphores[i].state, sizeof(int));
            memcpy(static_cast<char*>(pBuf) + offset + 48, &data.semaphores[i].id, sizeof(int));

            offset += 52;
        }

        UnmapViewOfFile(pBuf);
    }

    void CMemoryHandler::write_route_mem(const RouteMemData data) const
    {
        if (route_h_map_file == NULL)
        {
            this->error("Route shared mem file not open.");
            return;
        }

        void* pBuf = MapViewOfFile(route_h_map_file, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(RouteMemData));
        int offset = 0;

        for (int i = 0; i < 6000; i++)
        {
            memcpy(static_cast<char*>(pBuf) + offset, &data.tasks[i].uid, sizeof(long long));
            memcpy(static_cast<char*>(pBuf) + offset + 8, &data.tasks[i].distance, sizeof(float));
            memcpy(static_cast<char*>(pBuf) + offset + 12, &data.tasks[i].time, sizeof(float));

            offset += 16;
        }

        UnmapViewOfFile(pBuf);
    }

    void CMemoryHandler::create_traffic_memory() const {
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

        int vehicle_count = 40;
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

        this->initialize_memory_file(const_cast<wchar_t*>(traffic_mem_name), total_vehicle_format, 
            const_cast<HANDLE&>(traffic_h_map_file));
        free(vehicle_object);
        free(total_vehicle_format);
    }

    void CMemoryHandler::create_semaphore_memory() const {
        //                           xyz      t s
        const wchar_t* semaphore = L"fffffffffifii"; // 52 bytes
        //                              ccwxyz r id

        size_t semaphore_length = wcslen(semaphore);
        size_t semaphore_object_length = semaphore_length + 1; // +1 for null terminator

        wchar_t* semaphore_object = (wchar_t*)malloc(semaphore_object_length * sizeof(wchar_t));
        if (!semaphore_object) {
            this->error("Memory allocation failed for semaphore_object");
            return;
        }

        wcscpy(semaphore_object, semaphore); // Copy semaphore

        int semaphore_count = 40;
        size_t total_len = semaphore_count * semaphore_object_length + 1; // +1 for null terminator

        wchar_t* total_semaphore_format = (wchar_t*)malloc(total_len * sizeof(wchar_t));
        if (!total_semaphore_format) {
            this->error("Memory allocation failed for total_semaphore_format");
            free(semaphore_object);
            return;
        }

        total_semaphore_format[0] = L'\0'; // Initialize as an empty string
        for (int i = 0; i < semaphore_count; i++) {
            wcscat(total_semaphore_format, semaphore_object);
        }

        this->initialize_memory_file(const_cast<wchar_t*>(semaphore_mem_name), total_semaphore_format, 
            const_cast<HANDLE&>(semaphore_h_map_file));
        free(semaphore_object);
        free(total_semaphore_format);
    }

    void CMemoryHandler::create_route_memory() const {
        //                       uid
        const wchar_t* route = L"lff"; // 16 bytes
        //                        dt

        size_t route_length = wcslen(route);
        size_t route_object_length = route_length + 1; // +1 for null terminator

        wchar_t* route_object = (wchar_t*)malloc(route_object_length * sizeof(wchar_t));
        if (!route_object) {
            this->error("Memory allocation failed for route_object");
            return;
        }

        wcscpy(route_object, route); // Copy route

        int route_count = 6000;
        size_t total_len = route_count * route_object_length + 1; // +1 for null terminator

        wchar_t* total_route_format = (wchar_t*)malloc(total_len * sizeof(wchar_t));
        if (!total_route_format) {
            this->error("Memory allocation failed for total_route_format");
            free(route_object);
            return;
        }

        total_route_format[0] = L'\0'; // Initialize as an empty string
        for (int i = 0; i < route_count; i++) {
            wcscat(total_route_format, route_object);
        }

        this->initialize_memory_file(const_cast<wchar_t*>(route_mem_name), total_route_format, 
            const_cast<HANDLE&>(route_h_map_file));
        free(route_object);
        free(total_route_format);
    }
}