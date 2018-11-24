#include <vector>

#include "gtest/gtest.h"

#include "caffe/blob.hpp"
#include "caffe/common.hpp"
#include "caffe/filler.hpp"
#include "caffe/layers/infogain_loss_layer.hpp"

#include "caffe/test/test_caffe_main.hpp"
#include "caffe/test/test_gradient_check_util.hpp"

namespace caffe
{

  template <typename TypeParam>
  class InfogainLossLayerTest : public MultiDeviceTest<TypeParam>
  {
    typedef typename TypeParam::Dtype Dtype;

  protected:
    InfogainLossLayerTest()
      : blob_bottom_data_(new Blob(10, 5, 1, 1)),
        blob_bottom_label_(new Blob(10, 1, 1, 1)),
        blob_bottom_infogain_(new Blob(1, 1, 5, 5)),
        blob_top_loss_(new Blob()) {
      set_random_seed(1701);
      FillerParameter filler_param;
      PositiveUnitballFiller filler(filler_param);
      filler.Fill(this->blob_bottom_data_);
      blob_bottom_vec_.push_back(blob_bottom_data_);
      for (int i = 0; i < blob_bottom_label_->count(); ++i) {
        blob_bottom_label_->cpu_mdata()[i] = caffe_rng_rand() % 5;
      }
      blob_bottom_vec_.push_back(blob_bottom_label_);
      filler_param.set_min(0.1);
      filler_param.set_max(2.0);
      UniformFiller infogain_filler(filler_param);
      infogain_filler.Fill(this->blob_bottom_infogain_);
      blob_bottom_vec_.push_back(blob_bottom_infogain_);
      blob_top_vec_.push_back(blob_top_loss_);
    }
    virtual ~InfogainLossLayerTest() {
      delete blob_bottom_data_;
      delete blob_bottom_label_;
      delete blob_bottom_infogain_;
      delete blob_top_loss_;
    }
    Blob* const blob_bottom_data_;
    Blob* const blob_bottom_label_;
    Blob* const blob_bottom_infogain_;
    Blob* const blob_top_loss_;
    vector<Blob*> blob_bottom_vec_;
    vector<Blob*> blob_top_vec_;
  };

  TYPED_TEST_CASE(InfogainLossLayerTest, TestDtypesAndDevices);


  TYPED_TEST(InfogainLossLayerTest, TestGradient)
  {
    typedef typename TypeParam::Dtype Dtype;
    LayerParameter layer_param;
    InfogainLossLayer layer(layer_param);
    GradientChecker checker(1e-4, 2e-2, 1701, 1, 0.01);
    checker.CheckGradientExhaustive(&layer, this->blob_bottom_vec_,
                                    this->blob_top_vec_, 0);
  }

}  // namespace caffe