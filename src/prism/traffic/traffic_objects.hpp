#pragma once

#include "prism/common.hpp"
#include <cstdint>

#pragma pack( push, 1 )

namespace ets2_la_plugin::prism
{

    class traffic_object_t // Size: 0x0010
    {
    public:
        char pad_0008[ 8 ]; // 0x0008 (0x08)

        virtual void destructor();
        virtual uint64_t get_type() const; // 0x7 = traffic_semaphore_actor_t
    };
    static_assert( sizeof( traffic_object_t ) == 0x10 );

    class traffic_actor_t : public traffic_object_t // Size: 0x0080
    {
    public:
        char pad_0010[ 24 ];   // 0x0010 (0x18)
        placement_t placement; // 0x0028 (0x20)
        char pad_0048[ 56 ];   // 0x0048 (0x38)
    };
    static_assert( sizeof( traffic_actor_t ) == 0x80 );

    class traffic_semaphore_actor_t : public traffic_actor_t // Size: 0x01D8
    {
    public:
        char pad_0080[ 64 ];                // 0x0080 (0x40)
        class tr_semaphore_model_u* model;  // 0x00C0 (0x08)
        char pad_00C8[ 16 ];                // 0x00C8 (0x10)
        class model_object_u* model_object; // 0x00D8 (0x08)
        char pad_00E0[ 16 ];                // 0x00E0 (0x10)
        class model_coll_u* model_coll;     // 0x00F0 (0x08)
        class traffic_rule_t* traffic_rule; // 0x00F8 (0x08)
        uint8_t state;                      // 0x0100 (0x01) 2 = opening; 3 = open; 0 = closing; 1 = closed
        char pad_0101[ 15 ];                // 0x0101 (0x0f)
        float time_to_open;                 // 0x0110 (0x04)
        float open_time_remaining;          // 0x0114 (0x04) used for double animation
        char pad_0118[ 8 ];                 // 0x0118 (0x08)
        float time_to_close;                // 0x0120 (0x04)
        float close_time_remaining;         // 0x0124 (0x04) used for double animation
        char pad_0128[ 6 ];                 // 0x0128 (0x06)
        uint8_t animation_mode;             // 0x012E (0x01) 1 = single animation; 2 = double animation
        uint8_t coll_mode;                  // 0x012e (0x01)
        char pad_0130[ 4 ];                 // 0x0130 (0x04)
        float animation_time_elapsed;       // 0x0134 (0x04) used for single animation
        uint8_t N00007222;                  // 0x0138 (0x01)
        uint8_t N0000723A;                  // 0x0139 (0x01)
        char pad_013A[ 158 ];               // 0x013A (0x9e)
    };
    static_assert( sizeof( traffic_semaphore_actor_t ) == 0x1D8 );

}

#pragma pack( pop )
