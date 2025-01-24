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
- [ ] Expose a virtual memory file at `Local\ETS2LACameraProps` to get the current camera properties from the game.
- [ ] Expose a virtual memory file at `Local\ETS2LATraffic` to get the closest 20 traffic vehicles.

# Credits
- **dariowouters** - Massive help and the creator of the original plugin.
- **ziakhan4505** - Has been developing our side of the plugin with the help of dariowouters.