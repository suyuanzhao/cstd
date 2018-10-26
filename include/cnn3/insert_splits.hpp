#ifndef _CAFFE_UTIL_INSERT_SPLITS_HPP_
#define _CAFFE_UTIL_INSERT_SPLITS_HPP_

#include <string>

#include "proto.h"

namespace caffe
{

// Copy NetParameters with SplitLayers added to replace any shared bottom
// blobs with unique bottom blobs provided by the SplitLayer.
  void InsertSplits(CJSON* param, CJSON* param_split);

  void ConfigureSplitLayer(const string & layer_name, const string & blob_name,
                           const int blob_idx, const int split_count, const float loss_weight,
                           CJSON* split_layer_param);

  string SplitLayerName(const string & layer_name, const string & blob_name,
                        const int blob_idx);

  string SplitBlobName(const string & layer_name, const string & blob_name,
                       const int blob_idx, const int split_idx);

}  // namespace caffe

#endif  // CAFFE_UTIL_INSERT_SPLITS_HPP_