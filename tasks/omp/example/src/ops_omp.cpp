#include "omp/example/include/ops_omp.hpp"

#include <omp.h>

#include <iostream>
#include <numeric>
#include <string>
#include <thread>
#include <vector>

bool nesterov_a_test_task_omp::TestOMPTaskSequential::pre_processing_impl() {
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

bool nesterov_a_test_task_omp::TestOMPTaskSequential::validation_impl() {
  // Check count elements of output
  return taskData->outputs_count[0] == 1;
}

bool nesterov_a_test_task_omp::TestOMPTaskSequential::run_impl() {
  if (ops == "+") {
    res = std::accumulate(input_.begin(), input_.end(), 1);
  } else if (ops == "-") {
    res -= std::accumulate(input_.begin(), input_.end(), 0);
  } else if (ops == "*") {
    res = std::accumulate(input_.begin(), input_.end(), 1, std::multiplies<>());
  }
  return true;
}

bool nesterov_a_test_task_omp::TestOMPTaskSequential::post_processing_impl() {
  reinterpret_cast<int*>(taskData->outputs[0])[0] = res;
  return true;
}

bool nesterov_a_test_task_omp::TestOMPTaskParallel::pre_processing_impl() {
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

bool nesterov_a_test_task_omp::TestOMPTaskParallel::validation_impl() {
  // Check count elements of output
  return taskData->outputs_count[0] == 1;
}

bool nesterov_a_test_task_omp::TestOMPTaskParallel::run_impl() {
  double start = omp_get_wtime();
  auto temp_res = res;
  if (ops == "+") {
#pragma omp parallel for reduction(+ : temp_res)
    for (int i = 0; i < static_cast<int>(input_.size()); i++) {
      temp_res += input_[i];
    }
  } else if (ops == "-") {
#pragma omp parallel for reduction(- : temp_res)
    for (int i = 0; i < static_cast<int>(input_.size()); i++) {
      temp_res -= input_[i];
    }
  } else if (ops == "*") {
#pragma omp parallel for reduction(* : temp_res)
    for (int i = 0; i < static_cast<int>(input_.size()); i++) {
      temp_res *= input_[i];
    }
  }
  res = temp_res;
  double finish = omp_get_wtime();
  std::cout << "How measure time in OpenMP: " << finish - start << std::endl;
  return true;
}

bool nesterov_a_test_task_omp::TestOMPTaskParallel::post_processing_impl() {
  reinterpret_cast<int*>(taskData->outputs[0])[0] = res;
  return true;
}
