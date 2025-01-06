#include "tbb/example/include/ops_tbb.hpp"

#include <tbb/tbb.h>

#include <functional>
#include <numeric>
#include <random>
#include <string>
#include <thread>
#include <vector>

bool nesterov_a_test_task_tbb::TestTBBTaskSequential::pre_processing_impl() {
  // Init vectors
  input_ = std::vector<int>(taskData->inputs_count[0]);
  auto* tmp_ptr = reinterpret_cast<int*>(taskData->inputs[0]);
  for (unsigned i = 0; i < taskData->inputs_count[0]; i++) {
    input_[i] = tmp_ptr[i];
  }
  // Init value for output
  res = 1;
  return true;
}

bool nesterov_a_test_task_tbb::TestTBBTaskSequential::validation_impl() {
  // Check count elements of output
  return taskData->outputs_count[0] == 1;
}

bool nesterov_a_test_task_tbb::TestTBBTaskSequential::run_impl() {
  if (ops == "+") {
    res = std::accumulate(input_.begin(), input_.end(), 1);
  } else if (ops == "-") {
    res -= std::accumulate(input_.begin(), input_.end(), 0);
  } else if (ops == "*") {
    res = std::accumulate(input_.begin(), input_.end(), 1, std::multiplies<>());
  }
  return true;
}

bool nesterov_a_test_task_tbb::TestTBBTaskSequential::post_processing_impl() {
  reinterpret_cast<int*>(taskData->outputs[0])[0] = res;
  return true;
}

bool nesterov_a_test_task_tbb::TestTBBTaskParallel::pre_processing_impl() {
  // Init vectors
  input_ = std::vector<int>(taskData->inputs_count[0]);
  auto* tmp_ptr = reinterpret_cast<int*>(taskData->inputs[0]);
  for (unsigned i = 0; i < taskData->inputs_count[0]; i++) {
    input_[i] = tmp_ptr[i];
  }
  // Init value for output
  res = 1;
  return true;
}

bool nesterov_a_test_task_tbb::TestTBBTaskParallel::validation_impl() {
  // Check count elements of output
  return taskData->outputs_count[0] == 1;
}

bool nesterov_a_test_task_tbb::TestTBBTaskParallel::run_impl() {
  if (ops == "+") {
    res += oneapi::tbb::parallel_reduce(
        oneapi::tbb::blocked_range<std::vector<int>::iterator>(input_.begin(), input_.end()), 0,
        [](tbb::blocked_range<std::vector<int>::iterator> r, int running_total) {
          running_total += std::accumulate(r.begin(), r.end(), 0);
          return running_total;
        },
        std::plus<>());
  } else if (ops == "-") {
    res -= oneapi::tbb::parallel_reduce(
        oneapi::tbb::blocked_range<std::vector<int>::iterator>(input_.begin(), input_.end()), 0,
        [](tbb::blocked_range<std::vector<int>::iterator> r, int running_total) {
          running_total += std::accumulate(r.begin(), r.end(), 0);
          return running_total;
        },
        std::plus<>());
  } else if (ops == "*") {
    res *= oneapi::tbb::parallel_reduce(
        oneapi::tbb::blocked_range<std::vector<int>::iterator>(input_.begin(), input_.end()), 1,
        [](tbb::blocked_range<std::vector<int>::iterator> r, int running_total) {
          running_total *= std::accumulate(r.begin(), r.end(), 1, std::multiplies<>());
          return running_total;
        },
        std::multiplies<>());
  }
  return true;
}

bool nesterov_a_test_task_tbb::TestTBBTaskParallel::post_processing_impl() {
  reinterpret_cast<int*>(taskData->outputs[0])[0] = res;
  return true;
}
