/*
 * gVirtuS -- A GPGPU transparent virtualization component.
 *
 * Copyright (C) 2009-2010  The University of Napoli Parthenope at Naples.
 *
 * This file is part of gVirtuS.
 *
 * gVirtuS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * gVirtuS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with gVirtuS; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Written by: Giuseppe Coviello <giuseppe.coviello@uniparthenope.it>,
 *             Department of Applied Science
 */

#include "CudaRtHandler.h"

#include <string>
#include <iostream>
#include <cstdio>
#include <vector>

#include "CudaUtil.h"

using namespace std;

extern "C" {
    extern void** __cudaRegisterFatBinary(void *fatCubin);
    extern void __cudaUnregisterFatBinary(void **fatCubinHandle);
    extern void __cudaRegisterFunction(void **fatCubinHandle,
            const char *hostFun, char *deviceFun, const char *deviceName,
            int thread_limit, uint3 *tid, uint3 *bid, dim3 *bDim, dim3 *gDim,
            int *wSize);
    extern void __cudaRegisterVar(void **fatCubinHandle, char *hostVar,
            char *deviceAddress, const char *deviceName, int ext, int size,
            int constant, int global);
    extern void __cudaRegisterSharedVar(void **fatCubinHandle, void **devicePtr,
            size_t size, size_t alignment, int storage);
    extern void __cudaRegisterShared(void **fatCubinHandle, void **devicePtr);
    extern void __cudaRegisterTexture(void **fatCubinHandle,
            const textureReference *hostVar, void **deviceAddress, char *deviceName,
            int dim, int norm, int ext);
}

CUDA_ROUTINE_HANDLER(RegisterFatBinary) {
    char * handler = input_buffer->AssignString();
    __cudaFatCudaBinary * fatBin =
            CudaUtil::UnmarshalFatCudaBinary(input_buffer);
    void **fatCubinHandler = __cudaRegisterFatBinary((void *) fatBin);
    pThis->RegisterFatBinary(handler, fatCubinHandler);
    return new Result(cudaSuccess);
}

CUDA_ROUTINE_HANDLER(UnregisterFatBinary) {
    char * handler = input_buffer->AssignString();
    void **fatCubinHandle = pThis->GetFatBinary(handler);

    __cudaUnregisterFatBinary(fatCubinHandle);

    pThis->UnregisterFatBinary(handler);

    return new Result(cudaSuccess);
}

CUDA_ROUTINE_HANDLER(RegisterFunction) {
    char * handler = input_buffer->AssignString();
    void **fatCubinHandle = pThis->GetFatBinary(handler);
    const char *hostFun = strdup(input_buffer->AssignString());
    char *deviceFun = strdup(input_buffer->AssignString());
    const char *deviceName = strdup(input_buffer->AssignString());
    int thread_limit = input_buffer->Get<int>();
    uint3 *tid = input_buffer->Assign<uint3 > ();
    uint3 *bid = input_buffer->Assign<uint3 > ();
    dim3 *bDim = input_buffer->Assign<dim3 > ();
    dim3 *gDim = input_buffer->Assign<dim3 > ();
    int *wSize = input_buffer->Assign<int>();


    __cudaRegisterFunction(fatCubinHandle, hostFun, deviceFun, deviceName,
            thread_limit, tid, bid, bDim, gDim, wSize);

    pThis->RegisterDeviceFunction(hostFun, deviceFun);

    Buffer * output_buffer = new Buffer();
    output_buffer->AddString(deviceFun);
    output_buffer->Add(tid);
    output_buffer->Add(bid);
    output_buffer->Add(bDim);
    output_buffer->Add(gDim);
    output_buffer->Add(wSize);

    return new Result(cudaSuccess, output_buffer);
}

CUDA_ROUTINE_HANDLER(RegisterVar) {
    char * handler = input_buffer->AssignString();
    void **fatCubinHandle = pThis->GetFatBinary(handler);
    char *hostVar = (char *)
        CudaUtil::UnmarshalPointer(input_buffer->AssignString());
    char *deviceAddress = strdup(input_buffer->AssignString());
    const char *deviceName = strdup(input_buffer->AssignString());
    int ext = input_buffer->Get<int>();
    int size = input_buffer->Get<int>();
    int constant = input_buffer->Get<int>();
    int global = input_buffer->Get<int>();

    __cudaRegisterVar(fatCubinHandle, hostVar, deviceAddress, deviceName, ext,
            size, constant, global);

    cout << "Registered Var " << deviceAddress << " with handler "
            << (void *) hostVar << endl;

    return new Result(cudaSuccess);
}

CUDA_ROUTINE_HANDLER(RegisterSharedVar) {
    char * handler = input_buffer->AssignString();
    void **fatCubinHandle = pThis->GetFatBinary(handler);
    void **devicePtr = (void **) input_buffer->AssignString();
    size_t size = input_buffer->Get<size_t>();
    size_t alignment = input_buffer->Get<size_t>();
    int storage = input_buffer->Get<int>();

    __cudaRegisterSharedVar(fatCubinHandle, devicePtr, size, alignment, storage);

    cout << "Registered SharedVar " << (char *) devicePtr << endl;

    return new Result(cudaSuccess);
}

CUDA_ROUTINE_HANDLER(RegisterShared) {
    char * handler = input_buffer->AssignString();
    void **fatCubinHandle = pThis->GetFatBinary(handler);
    char *devPtr = strdup(input_buffer->AssignString());
    __cudaRegisterShared(fatCubinHandle, (void **) devPtr);
    cout << "Registerd Shared " << (char *) devPtr << " for " << fatCubinHandle << endl;
    return new Result(cudaSuccess);
}

CUDA_ROUTINE_HANDLER(RegisterTexture) {
    char * handler = input_buffer->AssignString();
    void **fatCubinHandle = pThis->GetFatBinary(handler);
    handler = input_buffer->AssignString();
    textureReference *hostVar = new textureReference;
    memmove(hostVar, input_buffer->Assign<textureReference > (),
            sizeof (textureReference));
    void **deviceAddress = (void **) input_buffer->AssignAll<char>();
    char *deviceName = strdup(input_buffer->AssignString());
    int dim = input_buffer->Get<int>();
    int norm = input_buffer->Get<int>();
    int ext = input_buffer->Get<int>();

    __cudaRegisterTexture(fatCubinHandle, hostVar, deviceAddress, deviceName,
            dim, norm, ext);

    pThis->RegisterTexture(handler, hostVar);

    return new Result(cudaSuccess);
}

