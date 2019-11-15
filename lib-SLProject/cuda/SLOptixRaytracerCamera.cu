#include <SLOptixDefinitions.h>
#include <SLOptixRaytracerHelper.h>

extern "C" {
__constant__ Params params;
}

extern "C" __global__ void __raygen__draw_solid_color()
{
    uint3 launch_index = optixGetLaunchIndex();
    params.image[launch_index.y * params.width + launch_index.x] = make_uchar4( 255, 0, 0, 255 );
}

extern "C" __global__ void __raygen__pinhole_camera()
{
    const uint3 idx = optixGetLaunchIndex();
    const uint3 dim = optixGetLaunchDimensions();

    const CameraData* rtData = (CameraData*)optixGetSbtDataPointer();
    const float2      d = 2.0f * make_float2(
            static_cast<float>( idx.x ) / static_cast<float>( dim.x ),
            static_cast<float>( idx.y ) / static_cast<float>( dim.y )
    ) - 1.0f;

    const float3 origin      = rtData->eye;
    const float3 direction   = normalize( d.x * rtData->U + d.y * rtData->V + rtData->W );

    params.image[idx.y * params.width + idx.x] = make_color( tracePrimaryRay(params.handle, origin, direction) );
}