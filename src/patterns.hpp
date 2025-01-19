#pragma once

namespace ets2_la_plugin::patterns
{
    inline constexpr auto base_ctrl =
        "48 8b 05 ? ? ? ? " // mov rax, cs:xxx (base_ctrl offset)
        "48 8b 4b ? "       // mov rcx, [rbx+xxh]
        "48 8b 80 ? ? ? ? " // mov rax, [rax+xxxxh] (game_actor offset)
        "48 8b b9";         // mov rdi, [rcx+...

    inline constexpr auto camera_manager =
        "48 8b 05 ? ? ? ? " // mov rax, cs:xxx (camera_manager offset)
        "8b 58 ? "          // mov ebx, [rax+xxh] (active camera offset)
        "75 ? "             // jnz loc_xx
        "3b 9f ? ? ? ? "    // cmp ebx, [rdi+xxxh]
        "74";               // jz ...

    inline constexpr auto game_traffic =
        "48 8b d9 "         // mov rbx, rcx
        "48 8b 0d ? ? ? ? " // mov rcx, cs:xxx (game_traffic offset)
        "48 85 c9 "         // test rcx, rcx
        "74 ? "             // jz loc_xx
        "48 8b 83 ? ? ? ? " // mov rax, [rbx+xxxh]
        "48 85 c0";         // test rax, rax
}
