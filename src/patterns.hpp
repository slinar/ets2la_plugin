#pragma once

namespace ets2_la_plugin::patterns
{
    // 48 8b 05 ? ? ? ? 48 8b 4b ? 48 8b 80 ? ? ? ? 48 8b b9
    inline constexpr auto base_ctrl =
        "48 8b 05 ? ? ? ? " // mov rax, cs:xxx (base_ctrl offset)
        "48 8b 4b ? "       // mov rcx, [rbx+xxh]
        "48 8b 80 ? ? ? ? " // mov rax, [rax+xxxxh] (game_actor offset)
        "48 8b b9";         // mov rdi, [rcx+...

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
}
