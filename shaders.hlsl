#define FLT_EPSILON 1.19209290E-07

struct vs_in
{
    float3 position : POS;
};

struct vs_out
{
    float4 position : SV_POSITION;
    float4 out1 : OUT1;
    float4 out2 : OUT2;
};

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

float4 ps_main(vs_out input) : SV_TARGET
{
    return input.out2;
}
