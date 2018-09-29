echo off
for /r %%f in (*.frag;*.vert) do (
    e:\Projects\VulkanSDK\1.1.77.0\Bin32\glslangValidator.exe -V %%f -o %%f.spv
)