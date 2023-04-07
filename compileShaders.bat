for /r %%i in (*.frag, *.vert) do C:\VulkanSDK\1.3.239.0\Bin\glslc.exe %%i -o resources\shaders\%%~nxi.spv
pause