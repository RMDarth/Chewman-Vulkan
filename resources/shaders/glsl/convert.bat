echo off
for /r %%f in (*.frag;*.vert;*.geom;*.comp) do (
    echo Compiling %%f
    C:\VulkanSDK\1.1.126.0\Bin\glslc.exe -c %%f -o %%f.spv
)