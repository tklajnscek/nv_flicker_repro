Looks like a specific combination of a vertex and pixel shader causes the interpolated values to fail randomly and makes the rendered output flicker. Adding some minimal math (e.g. + FLT_EPSILON) to the output 'out2' stops the flickering. Removing the unused vertex shader output 'out1' also stop the flickering. Removing the math on the primary position output also stops the flickering. It feels like this is hitting a bug in the driver shader compiler.

It's also very reminiscent of this which is supposed to be fixed
https://bugs.chromium.org/p/chromium/issues/detail?id=836788

Nvidia bug report opened here:
https://developer.nvidia.com/nvidia_bug/3381217

I originally hit this bug in GL code running via ANGLE (which inserts the unused interpolator for internal use), but it also fails with vanilla DX11 as demonstrated by the attached repro.

You can try it with the precompiled binaries or build the solution from source (it should just build out of the box with Visual Studio).

Run nv_flicker_repro.exe or from VS and it should flicker by default

It uses this shader:

    vs_out vs_main(vs_in input)
    {
        vs_out output;
        output.position = float4(input.position.xyz + FLT_EPSILON, 1.0);
        output.out1 = float4(input.position.xyz, 1.0);
        
        // This doesn't work - it flickers
        output.out2 = float4(input.position.xyz, 1.0);
        // But this does - adding some math to the output stops the flickering 
        // output.out2 = float4(input.position.xyz + FLT_EPSILON, 1.0);
    
        return output;
    }

Then open shaders.hlsl and change the vertex shader to use the other, previously commented out line and re-run the exe.
This shouldn't flicker anymore.

    vs_out vs_main(vs_in input)
    {
        vs_out output;
        output.position = float4(input.position.xyz + FLT_EPSILON, 1.0);
        output.out1 = float4(input.position.xyz, 1.0);
        
        // This doesn't work - it flickers
        // output.out2 = float4(input.position.xyz, 1.0);
        // But this does - adding some math to the output stops the flickering 
        output.out2 = float4(input.position.xyz + FLT_EPSILON, 1.0);
    
        return output;
    }

The only difference being the `+ FLT_EPSILON` in the workaround version.

Tested on multiple nvidia GPUs:
- GTX 980M
- RTX 2060
- NVIDIA Quadro M2000M
