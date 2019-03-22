#ifndef CAFFE_RNG_CPP_HPP_
#define CAFFE_RNG_CPP_HPP_

#include <algorithm>
#include <iterator>
#include <random>

#include "caffe/common.hpp"

namespace caffe
{

  typedef std::mt19937 rng_t;

  inline rng_t* caffe_rng()
  {
    static rng_t _rng_;
    static unsigned int inited_rng = 0;
    if (0 == inited_rng) {
      inited_rng = std::random_device()();
      _rng_.seed(inited_rng);
    }
    return &_rng_;
  }

// Fisher–Yates algorithm
  template <class RandomAccessIterator, class RandomGenerator>
  inline void shuffle(RandomAccessIterator begin, RandomAccessIterator end,
                      RandomGenerator* gen)
  {
    typedef typename std::iterator_traits<RandomAccessIterator>::difference_type
    difference_type;
    difference_type length = std::distance(begin, end);
    if (length <= 0) { return; }
    for (difference_type i = length - 1; i > 0; --i) {
      int t = rand()%i;
      std::iter_swap(begin + i, begin + t);
    }
  }

  template <class RandomAccessIterator>
  inline void shuffle(RandomAccessIterator begin, RandomAccessIterator end)
  {
    shuffle(begin, end, caffe_rng());
  }
}  // namespace caffe

#endif  // CAFFE_RNG_HPP_
