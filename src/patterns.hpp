#pragma once

namespace ets2_la_plugin::patterns
{
    // 48 8b 05 ? ? ? ? 48 8b ? 48 8b 49 ? 48 8b 80
    inline constexpr auto base_ctrl =
        "48 8b 05 ? ? ? ? " // mov rax, cs:xxx (base_ctrl offset)
        "48 8b ? "          // mov rdi, rcx
        "48 8b 49 ? "       // mov rcx, [rcx+xx]
        "48 8b 80";         // mov rax, [rax+xxxxh] (game_actor offset)


    // 48 8b 05 ? ? ? ? 8b 58 ? 75 ? 3b 9f ? ? ? ? 74
    inline constexpr auto camera_manager =
        "48 8b 05 ? ? ? ? " // mov rax, cs:xxx (camera_manager offset)
        "8b 58 ? "          // mov ebx, [rax+xxh] (active camera offset)
        "75 ? "             // jnz loc_xx
        "3b 9f ? ? ? ? "    // cmp ebx, [rdi+xxxh]
        "74";               // jz ...

    // 48 8b d9 48 8b 0d ? ? ? ? 48 85 c9 74 ? 48 8b 83 ? ? ? ? 48 85 c0
    inline constexpr auto game_traffic =
        "48 8b d9 "         // mov rbx, rcx
        "48 8b 0d ? ? ? ? " // mov rcx, cs:xxx (game_traffic offset)
        "48 85 c9 "         // test rcx, rcx
        "74 ? "             // jz loc_xx
        "48 8b 83 ? ? ? ? " // mov rax, [rbx+xxxh]
        "48 85 c0";         // test rax, rax

    // start of the function
    // 40 53 48 81 ec ? ? ? ? 48 8b 05 ? ? ? ? 48 8b d9 0f 29 74
    inline constexpr auto physics_vehicle_u_steering_advance =
        "40 53 "            // push rbx
        "48 81 ec ? ? ? ? " // sub rsp, xxh
        "48 8b 05 ? ? ? ? " // mov rax, cs:xxx (base_ctrl offset)
        "48 8b d9 "         // mov rbx, rcx
        "0f 29 74";         // movaps ...

    // eb ? 48 8b 87 ? ? ? ? 48 8d 55 ? 48 8d 8f ? ? ? ? ff 90
    inline constexpr auto gps_manager =
        "eb ? " // jmp short loc_xxx
        "48 8b 87 ? ? ? ? " // mov rax, [rdi + xxxxh] (gps_manager offset in game_ctrl)
        "48 8d 55 ? " // lea rdx, [rbp + xxh]
        "48 8d 8f ? ? ? ? "// lea rcx, [rdi + xxxxh]
        "ff 90"; // cal qword ptr[rax + 0A0h]

    // f3 0f 10 93 ? ? ? ? f3 0f 5c 93 ? ? ? ? 0f 2f d3
    inline constexpr auto vehicle_shared_u_steering_offset =
        "F3 0F 10 93 ? ? ? ? " // movss xmm2, dword ptr [rbx+xxxh] (steering member offset in vehicle_shared)
        "F3 0F 5C 93 ? ? ? ? " // subss xmm2, dword ptr [rbx+xxxxh]
        "0F 2F D3"; // comiss  xmm2, xmm3

    // 48 b8 3F 7C FD CC 88 01 00 00 4c 89 ? ? ? ? ? 0f 57 c0 48 89 ? ? ? ? ? 48 8d 8f
    inline constexpr auto game_actor_throttle_input_offset =
        "48 b8 3F 7C FD CC 88 01 00 00 " //mov rax, 188CCFD7C3Fh
        "4C 89 ? ? ? ? ? " // mov ...
        "0F 57 C0 " // xorps xmm0, xmm0
        "48 89 ? ? ? ? ? " // mov ...
        "48 8D 8F"; // lea rcx, [rdi+xxxxh] (throttle input offset in game_actor_u)

    // 48 b8 C7 91 B7 6C 4E 3A 00 00 4c 89 ? ? ? ? ? 0f 57 c0 48 89 ? ? ? ? ? 48 8d 8f
    inline constexpr auto game_actor_brake_input_offset =
        "48 b8 C7 91 B7 6C 4E 3A 00 00 " //mov rax, 3A4E6CB791C7h
        "4C 89 ? ? ? ? ? " // mov ...
        "0F 57 C0 " // xorps xmm0, xmm0
        "48 89 ? ? ? ? ? " // mov ...
        "48 8D 8F"; // lea rcx, [rdi+xxxxh] (brake input offset in game_actor_u)

    // f0 0f c1 42 ? 48 8d b9 ? ? ? ? 48 8b 07
    inline constexpr auto game_ctrl_u_some_nearby_non_ai_vehicles =
        "F0 0F C1 42 ? " // lock xadd [rdx+xh], eax
        "48 8D B9 ? ? ? ? " // lea rdi, [rcx+xxxxh] (some_nearby_non_ai_vehicles offset in game_ctrl)
        "48 8B 07"; // mov rax, [rdi]

    // 49 8d 8e ? ? ? ? 48 89 45 ? e8 ? ? ? ? 49 8b 86
    inline constexpr auto base_ctrl_u_some_nearby_kdop_items =
        "49 8d 8e ? ? ? ? " // lea rcx, [r14+xxxh] (some_nearby_kdop_items offset in base_ctrl)
        "48 89 45 ? " // mov ...
        "e8 ? ? ? ? " // call ...
        "49 8b 86"; // mov ...


}
