Looks like a specific combination of a vertex and pixel shader causes the interpolated values to fail randomly and makes the rendered output flicker. Adding some minimal math (e.g. + FLT_EPSILON) to the output 'out2' stops the flickering. Removing the unused vertex shader output 'out1' also stop the flickering. Removing the math on the primary position output also stops the flickering. It feels like this is hitting a bug in the driver shader compiler.

It's also very reminiscent of this which is supposed to be fixed
https://bugs.chromium.org/p/chromium/issues/detail?id=836788

I originally hit this bug in GL code running via ANGLE (which inserts the unused interpolator for internal use), but it also fails with vanilla DX11 as demonstrated by the attached repro.

Build the app and run it and you should see it flicker.
Then go into shader.hlsl and swap out the commented lines in the pixel shader and it should stop flickering.

Tested on multiple nvidia GPUs.