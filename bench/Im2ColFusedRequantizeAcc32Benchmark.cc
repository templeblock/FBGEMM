/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <random>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

#include "fbgemm/Fbgemm.h"
#include "src/RefImplementations.h"
#include "BenchUtils.h"

using namespace std;
using namespace fbgemm2;

void performance_test() {
  vector<conv_param_t> shapes = {
      // MB, IC, OC, IH, IW, G, KH, KW, stride_h, stride_w, pad_h, pad_w
      conv_param_t(1, 32, 32, 14, 14, 1, 3, 3, 1, 1, 0, 0),
      conv_param_t(1, 32, 32, 14, 14, 1, 3, 3, 1, 1, 1, 1),
      conv_param_t(2, 32, 32, 14, 14, 1, 3, 3, 1, 1, 0, 0),
      conv_param_t(2, 32, 32, 14, 14, 1, 3, 3, 1, 1, 1, 1),
      conv_param_t(   1,  272,  272,  47, 125, 1, 3, 3, 1, 1, 1, 1 ),
      conv_param_t(   1,  272,  272,  64, 125, 1, 3, 3, 1, 1, 1, 1 ),
      conv_param_t(   1,  272,  272,  66, 125, 1, 3, 3, 1, 1, 1, 1 ),
      conv_param_t(   1,  272,  272,  67, 100, 1, 3, 3, 1, 1, 1, 1 ),
      conv_param_t(   1,  272,  272,  75,  75, 1, 3, 3, 1, 1, 1, 1 ),
      conv_param_t(   1,  272,  272,  75,  76, 1, 3, 3, 1, 1, 1, 1 ),
      conv_param_t(   1,  272,  272,  75, 100, 1, 3, 3, 1, 1, 1, 1 ),
      conv_param_t(   1,  272,  272,  94,  75, 1, 3, 3, 1, 1, 1, 1 ),
      conv_param_t(   1,  272,  272, 109,  75, 1, 3, 3, 1, 1, 1, 1 ),
      conv_param_t(   1,  544,  544,  24,  63, 1, 3, 3, 1, 1, 1, 1 ),
      conv_param_t(   1,  544,  544,  33,  63, 1, 3, 3, 1, 1, 1, 1 ),
      conv_param_t(   1,  544,  544,  34,  50, 1, 3, 3, 1, 1, 1, 1 ),
      conv_param_t(   1,  544,  544,  36,  63, 1, 3, 3, 1, 1, 1, 1 ),
      conv_param_t(   1,  544,  544,  38,  38, 1, 3, 3, 1, 1, 1, 1 ),
      conv_param_t(   1,  544,  544,  38,  40, 1, 3, 3, 1, 1, 1, 1 ),
      conv_param_t(   1,  544,  544,  47,  38, 1, 3, 3, 1, 1, 1, 1 ),
      conv_param_t(   1, 1088, 1088,   7,   7, 1, 3, 3, 1, 1, 1, 1 ),
      conv_param_t(  51, 1088, 1088,   7,   7, 1, 3, 3, 1, 1, 1, 1 ),
      conv_param_t( 100, 1088, 1088,   7,   7, 1, 3, 3, 1, 1, 1, 1 ),
      conv_param_t(   1,  248,  248,  93, 250, 1, 3, 3, 2, 2, 1, 1 ),
      conv_param_t(   1,  248,  248, 128, 250, 1, 3, 3, 2, 2, 1, 1 ),
      conv_param_t(   1,  248,  248, 133, 200, 1, 3, 3, 2, 2, 1, 1 ),
      conv_param_t(   1,  248,  248, 150, 150, 1, 3, 3, 2, 2, 1, 1 ),
      conv_param_t(   1,  248,  248, 150, 151, 1, 3, 3, 2, 2, 1, 1 ),
      conv_param_t(   1,  248,  248, 150, 158, 1, 3, 3, 2, 2, 1, 1 ),
      conv_param_t(   1,  248,  248, 188, 150, 1, 3, 3, 2, 2, 1, 1 ),
      conv_param_t(   1,  248,  248, 225, 150, 1, 3, 3, 2, 2, 1, 1 ),
      conv_param_t(   1,  272,  272,  47, 125, 1, 3, 3, 2, 2, 1, 1 ),
      conv_param_t(   1,  272,  272,  64, 125, 1, 3, 3, 2, 2, 1, 1 ),
      conv_param_t(   1,  272,  272,  66, 125, 1, 3, 3, 2, 2, 1, 1 ),
      conv_param_t(   1,  272,  272,  67, 100, 1, 3, 3, 2, 2, 1, 1 ),
      conv_param_t(   1,  272,  272,  75,  75, 1, 3, 3, 2, 2, 1, 1 ),
      conv_param_t(   1,  272,  272,  75,  76, 1, 3, 3, 2, 2, 1, 1 ),
      conv_param_t(   1,  272,  272,  94,  75, 1, 3, 3, 2, 2, 1, 1 ),
      conv_param_t(   1,  544,  544,  14,  14, 1, 3, 3, 2, 2, 1, 1 ),
      conv_param_t(  51,  544,  544,  14,  14, 1, 3, 3, 2, 2, 1, 1 ),
      conv_param_t( 100,  544,  544,  14,  14, 1, 3, 3, 2, 2, 1, 1 ),
      conv_param_t(   1,    8,    8,   4,   4, 1, 3, 3, 1, 1, 1, 1 ),
  };

  bool flush = true;
  std::vector<char> llc;

  if (flush) {
    llc.resize(128 * 1024 * 1024, 1.0);
  }

  constexpr int NWARMUP = 4;
  constexpr int NITER = 10;

  chrono::time_point<chrono::high_resolution_clock> begin, end;
  for (auto conv_p : shapes) {
    aligned_vector<float> Afp32(
        conv_p.MB * conv_p.IH * conv_p.IW * conv_p.IC, 0.0f);
    aligned_vector<uint8_t> Aint8(
        conv_p.MB * conv_p.IH * conv_p.IW * conv_p.IC, 0);

    aligned_vector<uint8_t> Aint8_out(
        conv_p.MB * conv_p.OH * conv_p.OW * conv_p.KH * conv_p.KW * conv_p.IC,
        0);

    aligned_vector<float> Bfp32(
        conv_p.KH * conv_p.KW * conv_p.IC * conv_p.OC, 0.0f);
    aligned_vector<int8_t> Bint8(
        conv_p.KH * conv_p.KW * conv_p.IC * conv_p.OC, 0);

    aligned_vector<int32_t> Cint32_ref(
        conv_p.MB * conv_p.OH * conv_p.OW * conv_p.OC, 0.0f);

    aligned_vector<int32_t> Cint32_fb(
        conv_p.MB * conv_p.OH * conv_p.OW * conv_p.OC, 0);

    aligned_vector<int32_t> Cint32_fb2(
        conv_p.MB * conv_p.OH * conv_p.OW * conv_p.OC, 0);

    cout << conv_p.toString() << endl;

    // A matrix (input activations)
    randFill(Afp32, 0, 5);
    int32_t Aint8_zero_point = 4;
    for (auto i = 0; i < Afp32.size(); ++i) {
      Aint8[i] = static_cast<uint8_t>(Afp32[i]);
    }

    // B matrix (weights)
    randFill(Bfp32, -4, 4);
    // int32_t Bint8_zero_point = -3;
    for (auto i = 0; i < Bfp32.size(); ++i) {
      Bint8[i] = static_cast<int8_t>(Bfp32[i]);
    }

    // reference implementation
    conv_ref(
        conv_p,
        Aint8.data(),
        Aint8_zero_point,
        Bint8.data(),
        Cint32_ref.data());

    // matrix dimensions after im2col
    int MDim = conv_p.MB * conv_p.OH * conv_p.OW;
    int NDim = conv_p.OC;
    int KDim = conv_p.KH * conv_p.KW * conv_p.IC;

    // printMatrix(matrix_op_t::NoTranspose, Bint8.data(), KDim, NDim, NDim,
    // "B unpacked");
    // packedB.printPackedMatrix("B Packed");

    double ttot = 0;

    vector<int32_t> row_offset_buf;
    row_offset_buf.resize(
        PackAWithIm2Col<uint8_t, int32_t>::rowOffsetBufferSize());

    PackAWithIm2Col<uint8_t, int32_t> packA(
        conv_p, Aint8.data(), nullptr, Aint8_zero_point, row_offset_buf.data());

    PackBMatrix<int8_t, int32_t> packedB(
        matrix_op_t::NoTranspose, KDim, NDim, Bint8.data(), NDim);

    // no-op output process objects
    DoNothing<int32_t, int32_t> doNothing32BitObj;
    memCopy<> memcopyObj(doNothing32BitObj);

    ttot = 0;
    for (auto i = 0; i < NWARMUP + NITER; ++i) {
      llc_flush(llc);
      begin = chrono::high_resolution_clock::now();
      fbgemmPacked(
          packA,
          packedB,
          Cint32_fb.data(),
          Cint32_fb.data(),
          NDim,
          memcopyObj,
          0,
          1);
      end = chrono::high_resolution_clock::now();

      if (i >= NWARMUP) {
        auto dur = chrono::duration_cast<chrono::nanoseconds>(end - begin);
        ttot += dur.count();
      }
    }
    cout << fixed << "fused im2col GOPs: "
         << static_cast<double>(NITER) * 2 * MDim * NDim * KDim / ttot << endl;

    compare_buffers(Cint32_ref.data(), Cint32_fb.data(), MDim, NDim, NDim, 5);

    ttot = 0;
    for (auto i = 0; i < NWARMUP + NITER; ++i) {
      llc_flush(llc);
      begin = chrono::high_resolution_clock::now();

      im2col_ref(conv_p, Aint8.data(), Aint8_zero_point, Aint8_out.data());

      // printMatrix(matrix_op_t::NoTranspose, Aint8_out.data(), MDim, KDim,
      // KDim, "A_out after im2col unpacked");

      PackAWithRowOffset<uint8_t, int32_t> packAN(
          matrix_op_t::NoTranspose,
          MDim,
          KDim,
          Aint8_out.data(),
          KDim,
          nullptr,
          1,
          Aint8_zero_point,
          row_offset_buf.data());

      fbgemmPacked(
          packAN,
          packedB,
          Cint32_fb2.data(),
          Cint32_fb2.data(),
          NDim,
          memcopyObj,
          0,
          1);
      end = chrono::high_resolution_clock::now();

      if (i >= NWARMUP) {
        auto dur = chrono::duration_cast<chrono::nanoseconds>(end - begin);
        ttot += dur.count();
      }
    }

    ((volatile char*)(llc.data()));

    // packedB.printPackedMatrix("bench B Packed");
    // printMatrix(matrix_op_t::NoTranspose, Cint32_fb.data(), MDim, NDim, NDim,
    // "C fb fp32");
    // printMatrix(matrix_op_t::NoTranspose, Cint32_fb2.data(),
    // MDim, NDim, NDim, "C fb2 fp32");
    // printMatrix(matrix_op_t::NoTranspose,
    // Cint32_ref.data(), MDim, NDim, NDim, "C ref fp32");

    cout << fixed << "unfused im2col GOPs: "
         << static_cast<double>(NITER) * 2 * MDim * NDim * KDim / ttot << endl;
    // cout << "total time: " << ttot << " ns" << endl;
    compare_buffers(Cint32_ref.data(), Cint32_fb2.data(), MDim, NDim, NDim, 5);
  } // shapes
}

int main() {
#ifdef _OPENMP
  omp_set_num_threads(1);
#endif
  performance_test();
  return 0;
}
