// Imports specific to Wolfenstein: The Old Blood.
#include <string.h>
#include "rt.h"

// cudart64_40_17.dll (CUDA 4.0 runtime). The engine can transcode virtual textures on an NVIDIA GPU; on a Mac
// there is no CUDA device, so this is a runtime with zero devices. The CRT's static initializers register the
// kernels, textures and variables in .nvFatBi, so registration has to succeed; every query afterwards reports
// cudaErrorNoDevice and the engine uses its CPU transcoder. (The New Order only imported the fat-binary
// registration; The Old Blood imports the whole transcode path.)
#define cudaSuccess 0
#define cudaErrorMemoryAllocation 2
#define cudaErrorInsufficientDriver 35
#define cudaErrorNoDevice 38

static void cuda_log(const char *fn) {
    static int logged;
    if (!logged++) rt_log("[cuda] %s: no CUDA device (CPU virtual-texture transcode)\n", fn);
}
#define CUDA_FAIL(fn, err) void I_cudart64_40_17_dll_##fn(CPU *c) { cuda_log(#fn); RET(err); }
#define CUDA_OK(fn)        void I_cudart64_40_17_dll_##fn(CPU *c) { RET(cudaSuccess); }

// ---- static registration (CRT initializers / atexit) ----
static uint64_t fatbin_handle[4];
void I_cudart64_40_17_dll___cudaRegisterFatBinary(CPU *c) { RET(G(fatbin_handle)); }
void I_cudart64_40_17_dll___cudaUnregisterFatBinary(CPU *c) { }
void I_cudart64_40_17_dll___cudaRegisterFunction(CPU *c) { }
void I_cudart64_40_17_dll___cudaRegisterVar(CPU *c) { }
void I_cudart64_40_17_dll___cudaRegisterTexture(CPU *c) { }

// struct cudaChannelFormatDesc { int x, y, z, w; enum cudaChannelFormatKind f; } cudaCreateChannelDesc(x, y, z, w, f)
// 20 bytes: returned through a hidden pointer in rcx, so the arguments start at rdx, and rax returns the pointer.
void I_cudart64_40_17_dll_cudaCreateChannelDesc(CPU *c) {
    int32_t *d = PTR(A0);
    d[0] = (int32_t)A1; d[1] = (int32_t)A2; d[2] = (int32_t)A3;
    d[3] = (int32_t)AN(4); d[4] = (int32_t)AN(5);
    RET(A0);
}

// ---- device and version queries: no device ----
void I_cudart64_40_17_dll_cudaGetDeviceCount(CPU *c) {
    cuda_log("cudaGetDeviceCount");
    if (A0) *(int32_t *)PTR(A0) = 0;
    RET(cudaErrorNoDevice);
}
void I_cudart64_40_17_dll_cudaDriverGetVersion(CPU *c) { if (A0) *(int32_t *)PTR(A0) = 0; RET(cudaSuccess); }
void I_cudart64_40_17_dll_cudaRuntimeGetVersion(CPU *c) { if (A0) *(int32_t *)PTR(A0) = 4000; RET(cudaSuccess); }
void I_cudart64_40_17_dll_cudaGetLastError(CPU *c) { RET(cudaSuccess); }
void I_cudart64_40_17_dll_cudaGetErrorString(CPU *c) {
    static const char no_device[] = "no CUDA-capable device is detected", other[] = "CUDA is not available";
    RET(G((uint32_t)A0 == cudaErrorNoDevice ? no_device : other));
}
CUDA_FAIL(cudaGetDeviceProperties, cudaErrorNoDevice)
CUDA_FAIL(cudaSetDeviceFlags, cudaErrorNoDevice)
CUDA_FAIL(cudaGLSetGLDevice, cudaErrorNoDevice)
CUDA_FAIL(cudaFuncGetAttributes, cudaErrorNoDevice)
CUDA_OK(cudaDeviceReset)
CUDA_OK(cudaThreadSynchronize)

// ---- memory: allocations fail, frees of NULL succeed ----
static void zero_out(uint64_t p) { if (p) *(uint64_t *)PTR(p) = 0; }
void I_cudart64_40_17_dll_cudaMalloc(CPU *c) { zero_out(A0); RET(cudaErrorMemoryAllocation); }
void I_cudart64_40_17_dll_cudaMallocArray(CPU *c) { zero_out(A0); RET(cudaErrorMemoryAllocation); }
void I_cudart64_40_17_dll_cudaHostAlloc(CPU *c) { zero_out(A0); RET(cudaErrorMemoryAllocation); }
void I_cudart64_40_17_dll_cudaHostGetDevicePointer(CPU *c) { zero_out(A0); RET(cudaErrorNoDevice); }
CUDA_OK(cudaFree)
CUDA_OK(cudaFreeHost)
CUDA_FAIL(cudaMemcpyAsync, cudaErrorNoDevice)
CUDA_FAIL(cudaMemcpy2DToArray, cudaErrorNoDevice)
CUDA_FAIL(cudaBindTextureToArray, cudaErrorNoDevice)

// ---- streams, events, launches, GL interop ----
void I_cudart64_40_17_dll_cudaStreamCreate(CPU *c) { zero_out(A0); RET(cudaErrorNoDevice); }
void I_cudart64_40_17_dll_cudaEventCreate(CPU *c) { zero_out(A0); RET(cudaErrorNoDevice); }
CUDA_OK(cudaStreamDestroy)
CUDA_OK(cudaEventDestroy)
CUDA_FAIL(cudaEventRecord, cudaErrorNoDevice)
CUDA_FAIL(cudaEventQuery, cudaErrorNoDevice)
CUDA_FAIL(cudaEventSynchronize, cudaErrorNoDevice)
void I_cudart64_40_17_dll_cudaEventElapsedTime(CPU *c) { if (A0) *(float *)PTR(A0) = 0; RET(cudaErrorNoDevice); }
CUDA_FAIL(cudaConfigureCall, cudaErrorNoDevice)
CUDA_FAIL(cudaSetupArgument, cudaErrorNoDevice)
CUDA_FAIL(cudaLaunch, cudaErrorNoDevice)
void I_cudart64_40_17_dll_cudaGraphicsGLRegisterBuffer(CPU *c) { zero_out(A0); RET(cudaErrorNoDevice); }
CUDA_FAIL(cudaGraphicsMapResources, cudaErrorNoDevice)
CUDA_FAIL(cudaGraphicsUnmapResources, cudaErrorNoDevice)
void I_cudart64_40_17_dll_cudaGraphicsResourceGetMappedPointer(CPU *c) { zero_out(A0); RET(cudaErrorNoDevice); }

// kernel32: the CRT asks which code page file APIs use. ANSI (TRUE), as on a default Windows install.
// It is also one of the first calls after the log is open, so the save-folder setup (cloud_wolf.c, which runs
// before the log exists) reports here.
void wolf_cloud_log(void);
void I_kernel32_dll_AreFileApisANSI(CPU *c) { wolf_cloud_log(); RET(1); }
