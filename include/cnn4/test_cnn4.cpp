
#include "proto.h"
#include "solver.hpp"
#include "SGDSolver.hpp"
#include "insert_splits.hpp"

void Lock() {}
void Unlock() {}
void InitMutex() {}

template <typename Dtype>
void ClearParamDiffs(Blob<Dtype>** learnable_params_, int learnable_params_size)
{
  for (int i = 0; i < learnable_params_size; ++i) {
    BlobF* blob = learnable_params_[i];
    caffe_set<Dtype>(CONTEXT, blob->count(), static_cast<Dtype>(0), blob->mutable_cpu_diff());
  }
}

int test_cnn4() {
  const char* fn = NULL;
  //test_caffe2json();
  if (1) {
    fn = "C:/caffe_train/mnist/lenet.json";
    _chdir("C:/caffe_train/mnist/");
  }
  if (0) {
    fn = "E:/OCR_Line/model/densenet-no-blstm/densenet-no-blstm.json";
    _chdir("E:/OCR_Line/model/densenet-no-blstm/");
  }

  CJSON* root = cJSON_OpenFile(fn);
  typedef float Dtype;
  Net* net = new Net;
  DataShape d = {0};
  //im2col_nd<float>(GPUCONTEXT, 0, 0, 0, d, d, d, d, d, d, 0);
  //im2col_nd<float>(CPUCONTEXT, 0, 0, 0, d, d, d, d, d, d, 0);
  //caffe_copy<float>(GPUCONTEXT, 0, 0, 0);
  //caffe_set<float>((GPUContext*)0, 0, 0, 0);
  if (net->FromProto(root)) {
    Solver* solver = new SGDSolver();
    solver->init(net);
    solver->Solve();
    delete solver;
    net->Free();
    //ClearParamDiffs(net->layers_->blobs_, 1);
  }
  cJSON_Delete(root);
  delete net;
  return 0;
}
