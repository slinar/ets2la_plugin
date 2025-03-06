# ets2la-plugin
Please see the [original repository](https://github.com/dariowouters/ts-extra-utilities) by dariowouters!

# What does this do?
- [x] Expose a virtual memory file to `Local\ETS2LAPluginInput` to **send steering commands** directly into the game memory.
    ```python
    import time, mmap
    buf = mmap.mmap(0, 9, r"Local\ETS2LAPluginInput")
    while True:
        buf[:] = struct.pack('=f?l', 
            0.00, # Steering value
            True, # Should we steer?
            math.floor(time.time()) # Current timestamp
        )
    ```
- [x] Expose a virtual memory file at `Local\ETS2LACameraProps` to get the **current camera properties** from the game.
    ```python
    import time, mmap
    buf = mmap.mmap(0, 36, r"Local\ETS2LACameraProps")
    while True:
        format = "=ffffhhffff"
        data = struct.unpack(format, buf[:36])
        data # fov, x, y, z, cx, cz, qw, qx, qy, qz
    ```
- [x] Expose a virtual memory file at `Local\ETS2LATraffic` to get the **closest 20 traffic vehicles**.
    ```python
    import time, mmap
    buf = mmap.mmap(0, 2600, r"Local\ETS2LATraffic")
    while True:
        vehicle_format = "ffffffffffffh"
        trailer_format = "ffffffffff"
        vehicle_object_format = vehicle_format + trailer_format + trailer_format
        total_format = "=" + vehicle_object_format * 20
        data = struct.unpack(total_format, buf[:2600])
        data # (vehicle + trailer + trailer) * 20
        # vehicle:
        # x, y, z, qw, qx, qy, qz, width, height, length, speed, acceleration, trailer_count
        # trailer:
        # x, y, z, qw, qx, qy, qz, width, height, length
    ```

- [x] Expose a virtual memory file at `Local\ETS2LASemaphore` to get the **closest 20 traffic lights and gates**.
    ```python
    import time, mmap
    buf = mmap.mmap(0, 1040, r"Local\ETS2LASemaphore")

    # Traffic light states
    OFF = 0
    ORANGE_TO_RED = 1
    RED = 2
    ORANGE_TO_GREEN = 4
    GREEN = 8
    SLEEP = 32

    # Gate states
    CLOSING = 0
    CLOSED = 1
    OPENING = 2
    OPEN = 3

    while True:
        semaphore_format = "fffffffffifii"
        total_format = "=" + semaphore_format * 20
        data = struct.unpack(total_format, buf[:1040])
        data # (semaphore_format) * 20
        # traffic_light (index 9 == 1):
        # x, y, z, cx, cz, qw, qx, qy, qz, type, time_remaining (in state), state, id
        # gate (index 9 == 2):
        # x, y, z, cx, cz, qw, qx, qy, qz, type, time_remaining (in state), state, id 
    ```
- [x] Expose a virtual memory file at `Local\ETS2LARoute` to output the **current navigation route**. NOTE: Limited to 5000 items! (mods might go over)
    ```python
    import time, mmap
    buf = mmap.mmap(0, 80_000, r"Local\ETS2LARoute")

    while True:
        item_format = "qff"
        total_format = "=" + item_format * 5000
        data = struct.unpack(total_format, buf[:80_000])
        data # (item_format) * 20
        # route item:
        # node_uid, distance_to_end (m), time_to_end (s)
    ```

# Credits
- **dariowouters** - Massive help and the creator of the original plugin.
- **ziakhan4505** - Has been developing our side of the plugin with the help of dariowouters.
- **Tumppi066** - Virtual memory implementation.