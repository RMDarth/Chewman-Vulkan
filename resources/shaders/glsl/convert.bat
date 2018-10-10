echo off
for /r %%f in (*.frag;*.vert) do (
    echo Compiling %%f
    E:\Projects\VulkanSDK\1.1.77.0\Bin\glslc.exe -c %%f -o %%f.spv
)