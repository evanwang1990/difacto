#include <gtest/gtest.h>
#include "./utils.h"
#include "common/spmv.h"
#include "data/batch_iter.h"
#include "dmlc/timer.h"
#include "./spmv_test.h"

using namespace difacto;

dmlc::data::RowBlockContainer<unsigned> data;
std::vector<feaid_t> uidx;

void gen_sliced_vec(const SArray<real_t>& x,
                    SArray<real_t>* x_val,
                    SArray<int>* x_pos) {
  x_pos->resize(x.size());
  SArray<real_t> itv;
  gen_vals(x.size(), 1, 10, &itv);
  int cur_pos = 0;
  for (size_t i = 0; i < x.size(); ++i) {
    cur_pos += static_cast<int>(itv[i]);
    (*x_pos)[i] = cur_pos;
  }
  gen_vals(cur_pos+1, -10, 10, x_val);
  for (size_t i = 0; i < x.size(); ++i) {
    (*x_val)[(*x_pos)[i]] = x[i];
  }
}

void slice_vec(const SArray<real_t>& x_val,
              const SArray<int>& x_pos,
              SArray<real_t>* x) {
  x->resize(x_pos.size());
  for (size_t i = 0; i < x->size(); ++i) {
    (*x)[i] = x_pos[i] == -1 ? 0 : x_val[x_pos[i]];
  }
}

TEST(SpMV, Times) {
  load_data(&data, &uidx);
  auto D = data.GetBlock();
  SArray<real_t> x;
  gen_vals(uidx.size(), -10, 10, &x);

  SArray<real_t> y1(D.size);
  SArray<real_t> y2(D.size);

  test::SpMV::Times(D, x, &y1);
  SpMV::Times(D, x, &y2);
  EXPECT_EQ(norm2(y1), norm2(y2));
}

TEST(SpMV, TransTimes) {
  load_data(&data, &uidx);
  auto D = data.GetBlock();
  SArray<real_t> x;
  gen_vals(D.size, -10, 10, &x);

  SArray<real_t> y1(uidx.size());
  SArray<real_t> y2(uidx.size());

  test::SpMV::TransTimes(D, x, &y1);
  SpMV::TransTimes(D, x, &y2);
  EXPECT_EQ(norm2(y1), norm2(y2));
}

TEST(SpMV, TimesPos) {
  load_data(&data, &uidx);
  auto D = data.GetBlock();
  SArray<real_t> x;
  gen_vals(uidx.size(), -10, 10, &x);
  SArray<int> x_pos;
  SArray<real_t> x_val;
  gen_sliced_vec(x, &x_val, &x_pos);

  SArray<real_t> y(D.size);
  SArray<int> y_pos;
  SArray<real_t> y_val;
  gen_sliced_vec(y, &y_val, &y_pos);
  memset(y_val.data(), 0, y_val.size()*sizeof(real_t));

  test::SpMV::Times(D, x, &y);
  SpMV::Times(D, x_val, &y_val, DEFAULT_NTHREADS, x_pos, y_pos);

  EXPECT_EQ(norm2(y), norm2(y_val));

  SArray<real_t> y2;
  slice_vec(y_val, y_pos, &y2);
  EXPECT_EQ(norm2(y), norm2(y2));
}

TEST(SpMV, TransTimesPos) {
  load_data(&data, &uidx);
  auto D = data.GetBlock();
  SArray<real_t> x;
  gen_vals(D.size, -10, 10, &x);
  SArray<int> x_pos;
  SArray<real_t> x_val;
  gen_sliced_vec(x, &x_val, &x_pos);

  SArray<real_t> y(uidx.size());
  SArray<int> y_pos;
  SArray<real_t> y_val;
  gen_sliced_vec(y, &y_val, &y_pos);
  memset(y_val.data(), 0, y_val.size()*sizeof(real_t));

  test::SpMV::TransTimes(D, x, &y);
  SpMV::TransTimes(D, x_val, &y_val, DEFAULT_NTHREADS, x_pos, y_pos);
  EXPECT_EQ(norm2(y), norm2(y_val));

  SArray<real_t> y2;
  slice_vec(y_val, y_pos, &y2);
  EXPECT_EQ(norm2(y), norm2(y2));
}
