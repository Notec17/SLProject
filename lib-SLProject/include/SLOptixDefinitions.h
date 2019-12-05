//
// Created by nic on 24.10.19.
//

#ifndef SLPROJECT_SLOPTIXDEFINITIONS_H
#define SLPROJECT_SLOPTIXDEFINITIONS_H

#include <vector_types.h>
#include <optix_types.h>
#include <cuda.h>

struct Material
{
    float4  diffuse_color;
    float4  ambient_color;
    float4  specular_color;
    float4  emissive_color;
    float   shininess;
    float   kr;
    float   kt;
    float   kn;
};

struct Light
{
    float4  diffuse_color;
    float4  ambient_color;
    float4  specular_color;
    float3  position;
    float   spotCutOffDEG;
    float   spotExponent;
    float   spotCosCut;
    float3  spotDirWS;
    float   kc;
    float   kl;
    float   kq;
};

struct Params
{
    uchar4*                 image;
    unsigned int            width;
    unsigned int            height;

    int                     max_depth;
    float                   scene_epsilon;

    OptixTraversableHandle  handle;

    Light*                  lights;
    unsigned int            numLights;
    float4                  globalAmbientColor;

    float3*                 debug;
};

enum RayType
{
    RAY_TYPE_RADIANCE   = 0,
    RAY_TYPE_OCCLUSION  = 1,
    RAY_TYPE_COUNT
};

struct CameraData
{
    float3       eye;
    float3       U;
    float3       V;
    float3       W;
};

struct MissData
{
    float4 bg_color;
};

struct HitData
{
    Material    material;
    CUtexObject textureObject;

    int         sbtIndex;
    float3*     normals;
    short3*     indices;
    float2*     texCords;
};

template <typename T>
struct SbtRecord
{
    __align__( OPTIX_SBT_RECORD_ALIGNMENT ) char header[OPTIX_SBT_RECORD_HEADER_SIZE];
    T data;
};

typedef SbtRecord<CameraData>   RayGenSbtRecord;
typedef SbtRecord<MissData>     MissSbtRecord;
typedef SbtRecord<HitData>      HitSbtRecord;

#endif //SLPROJECT_SLOPTIXDEFINITIONS_H