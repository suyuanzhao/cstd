

#include "caffe/layers/absval_layer.hpp"


namespace
{

  template <typename Dtype>
  void AbsValLayer<Dtype>::LayerSetUp(const vector<Blob<Dtype>*> & bottom,
                                      const vector<Blob<Dtype>*> & top)
  {
    NeuronLayer<Dtype>::LayerSetUp(bottom, top);
    CHECK_NE(top[0], bottom[0]) << this->type() << " Layer does not "
                                "allow in-place computation.";
  }

  template <typename Dtype>
  void AbsValLayer<Dtype>::Forward(_CONTEXT,
    const vector<Blob<Dtype>*> & bottom, const vector<Blob<Dtype>*> & top)
  {
    const int count = top[0]->count();
    Dtype* top_data = top[0]->mutable_data<Context>();
    caffe_abs(count, bottom[0]->data<Context>(), top_data);
  }

  template <typename Dtype>
  void AbsValLayer<Dtype>::Backward(CPUContext* context, const vector<Blob<Dtype>*> & top,
                                        const vector<Blob<Dtype>*> & bottom)
  {
    const int count = top[0]->count();
    const Dtype* top_diff = top[0]->diff<Context>();
    if (bottom[0]->propagate_down_) {
      const Dtype* bottom_data = bottom[0]->data<Context>();
      Dtype* bottom_diff = bottom[0]->mutable_diff<Context>();
      caffe_sign(count, bottom_data, bottom_diff);
      caffe_mul(count, bottom_diff, top_diff, bottom_diff);
    }
  }

#ifdef CPU_ONLY
  STUB_GPU(AbsValLayer);
#endif

  INSTANTIATE_CLASS(AbsValLayer);
  REGISTER_LAYER_CLASS(AbsVal);

}  // namespace