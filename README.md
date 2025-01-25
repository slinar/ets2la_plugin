# ets2la-plugin
Please see the [original repository](https://github.com/dariowouters/ts-extra-utilities) by dariowouters!

# What does this do?
- [x] Expose a virtual memory file to `Local\ETS2LAPluginInput` to send steering commands directly into the game memory.
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
- [x] Expose a virtual memory file at `Local\ETS2LACameraProps` to get the current camera properties from the game.
    ```python
    import time, mmap
    buf = mmap.mmap(0, 36, r"Local\ETS2LACameraProps")
    while True:
        format = "=ffffhhffff"
        data = struct.unpack(format, buf[:36])
        data # fov, x, y, z, cx, cz, qw, qx, qy, qz
    ```
- [x] Expose a virtual memory file at `Local\ETS2LATraffic` to get the closest 20 traffic vehicles.
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

# Credits
- **dariowouters** - Massive help and the creator of the original plugin.
- **ziakhan4505** - Has been developing our side of the plugin with the help of dariowouters.