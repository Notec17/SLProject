//
// Created by nic on 24.10.19.
//

#ifndef SLPROJECT_SLOPTIXHELPER_H
#define SLPROJECT_SLOPTIXHELPER_H

// Optix error-checking and CUDA error-checking are copied from nvidia optix sutil
//------------------------------------------------------------------------------
//
// OptiX error-checking
//
//------------------------------------------------------------------------------

#define OPTIX_CHECK( call )                                                    \
    do                                                                         \
    {                                                                          \
        OptixResult res = call;                                                \
        if( res != OPTIX_SUCCESS )                                             \
        {                                                                      \
            std::stringstream ss;                                              \
            ss << "Optix call '" << #call << "' failed: " __FILE__ ":"         \
               << __LINE__ << ")\n";                                           \
            throw SLOptixException( res, ss.str().c_str() );                   \
        }                                                                      \
    } while( 0 )


#define OPTIX_CHECK_LOG( call )                                                \
    do                                                                         \
    {                                                                          \
        OptixResult res = call;                                                \
        if( res != OPTIX_SUCCESS )                                             \
        {                                                                      \
            std::stringstream ss;                                              \
            ss << "Optix call '" << #call << "' failed: " __FILE__ ":"         \
               << __LINE__ << ")\nLog:\n" << log                               \
               << ( sizeof_log > sizeof( log ) ? "<TRUNCATED>" : "" )          \
               << "\n";                                                        \
            throw SLOptixException( res, ss.str().c_str() );                   \
        }                                                                      \
    } while( 0 )


//------------------------------------------------------------------------------
//
// CUDA error-checking
//
//------------------------------------------------------------------------------

#define CUDA_CHECK( call )                                                     \
    do                                                                         \
    {                                                                          \
        cudaError_t error = call;                                              \
        if( error != cudaSuccess )                                             \
        {                                                                      \
            std::stringstream ss;                                              \
            ss << "CUDA call (" << #call << " ) failed with error: '"          \
               << cudaGetErrorString( error )                                  \
               << "' (" __FILE__ << ":" << __LINE__ << ")\n";                  \
            throw SLOptixException( ss.str().c_str() );                        \
        }                                                                      \
    } while( 0 )


#define CUDA_SYNC_CHECK()                                                      \
    do                                                                         \
    {                                                                          \
        cudaDeviceSynchronize();                                               \
        cudaError_t error = cudaGetLastError();                                \
        if( error != cudaSuccess )                                             \
        {                                                                      \
            std::stringstream ss;                                              \
            ss << "CUDA error on synchronize with error '"                     \
               << cudaGetErrorString( error )                                  \
               << "' (" __FILE__ << ":" << __LINE__ << ")\n";                  \
            throw SLOptixException( ss.str().c_str() );                        \
        }                                                                      \
    } while( 0 )

class SLOptixException : public std::runtime_error
{
public:
    SLOptixException( const char* msg )
            : std::runtime_error( msg )
    { }

    SLOptixException( OptixResult res, const char* msg )
            : std::runtime_error( createMessage( res, msg ).c_str() )
    { }

private:
    std::string createMessage( OptixResult res, const char* msg )
    {
        std::ostringstream out;
        out << optixGetErrorName( res ) << ": " << msg;
        return out.str();
    }
};

// Get PTX string from File
std::string getPtxStringFromFile(
        std::string filename,               // Cuda C input file name
        const char** log = NULL );          // (Optional) pointer to compiler log string. If *log == NULL there is no output.

#endif //SLPROJECT_SLOPTIXHELPER_H
