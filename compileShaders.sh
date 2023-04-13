#!/bin/bash
find . -type f \( -name "*.frag" -o -name "*.vert" \) -exec sh -c '$HOME/VulkanSDK/1.3.239.0/macOS/bin/glslc {} -o  $PWD/resources/shaders/$(basename {}).spv' \;
