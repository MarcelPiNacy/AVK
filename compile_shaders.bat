C:/VulkanSDK/1.1.121.2/Bin/glslc.exe avk/internal/shaders/bar_graph.vert -o avk/internal/shaders/bar_graph_vs.spv -O
C:/VulkanSDK/1.1.121.2/Bin/glslc.exe avk/internal/shaders/bar_graph.frag -o avk/internal/shaders/bar_graph_fs.spv -O
robocopy avk/internal/shaders/ build/Debug_x64/ *.spv /xo
robocopy avk/internal/shaders/ build/Release_x64/ *.spv /xo
robocopy avk/internal/shaders/ build/Debug_Win32/ *.spv /xo
robocopy avk/internal/shaders/ build/Release_Win32/ *.spv /xo
exit /B 0