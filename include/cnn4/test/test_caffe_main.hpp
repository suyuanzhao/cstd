// The main caffe test code. Your test cpp code should include this hpp
// to allow a main function to be compiled into the binary.
#ifndef CAFFE_TEST_TEST_CAFFE_MAIN_HPP_
#define CAFFE_TEST_TEST_CAFFE_MAIN_HPP_

#include "wstd/logging.hpp"
#include "gtest/gtest.h"

#include <cstdio>
#include <cstdlib>

#include "caffe/common.hpp"

using std::cout;
using std::endl;

#ifdef CMAKE_BUILD
#include "caffe_config.h"
#else
#define CUDA_TEST_DEVICE -1
#define CMAKE_SOURCE_DIR "src/"
#define EXAMPLES_SOURCE_DIR "examples/"
#define CMAKE_EXT ""
#endif

int main1(int argc, char** argv);

namespace caffe
{

  template <typename TypeParam>
  class MultiDeviceTest : public ::testing::Test
  {
  public:
    typedef typename TypeParam::Dtype Dtype;
  protected:
    MultiDeviceTest() {
      set_mode(TypeParam::device);
    }
    virtual ~MultiDeviceTest() {}
  };

  typedef ::testing::Types<float, double> TestDtypes;

  template <typename TypeParam>
  struct CPUDevice {
    typedef TypeParam Dtype;
    static const Brew device = CPU;
  };

  template <typename Dtype>
  class CPUDeviceTest : public MultiDeviceTest<CPUDevice >
  {
  };

#ifdef CPU_ONLY

  typedef ::testing::Types < CPUDevice<float>,
          CPUDevice<double> > TestDtypesAndDevices;

#else

  template <typename TypeParam>
  struct GPUDevice {
    typedef TypeParam Dtype;
    static const Brew device = GPU;
  };

  template <typename Dtype>
  class GPUDeviceTest : public MultiDeviceTest<GPUDevice >
  {
  };

  typedef ::testing::Types < CPUDevice<float>, CPUDevice<double>,
          GPUDevice<float>, GPUDevice<double> >
          TestDtypesAndDevices;

#endif

}  // namespace caffe

#endif  // CAFFE_TEST_TEST_CAFFE_MAIN_HPP_
