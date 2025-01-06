#include "mpi/example/include/ops_mpi.hpp"

#include <algorithm>
#include <functional>
#include <random>
#include <string>
#include <thread>
#include <vector>

bool nesterov_a_test_task_mpi::TestMPITaskSequential::pre_processing_impl() {
  // Init vectors
  input_ = std::vector<int>(taskData->inputs_count[0]);
  auto* tmp_ptr = reinterpret_cast<int*>(taskData->inputs[0]);
  for (unsigned i = 0; i < taskData->inputs_count[0]; i++) {
    input_[i] = tmp_ptr[i];
  }
  // Init value for output
  res = 0;
  return true;
}

bool nesterov_a_test_task_mpi::TestMPITaskSequential::validation_impl() {
  // Check count elements of output
  return taskData->outputs_count[0] == 1;
}

bool nesterov_a_test_task_mpi::TestMPITaskSequential::run_impl() {
  if (ops == "+") {
    res = std::accumulate(input_.begin(), input_.end(), 0);
  } else if (ops == "-") {
    res = -std::accumulate(input_.begin(), input_.end(), 0);
  } else if (ops == "max") {
    res = *std::max_element(input_.begin(), input_.end());
  }
  return true;
}

bool nesterov_a_test_task_mpi::TestMPITaskSequential::post_processing_impl() {
  reinterpret_cast<int*>(taskData->outputs[0])[0] = res;
  return true;
}

bool nesterov_a_test_task_mpi::TestMPITaskParallel::pre_processing_impl() {
  unsigned int delta = 0;
  if (world.rank() == 0) {
    delta = taskData->inputs_count[0] / world.size();
  }
  broadcast(world, delta, 0);

  if (world.rank() == 0) {
    // Init vectors
    input_ = std::vector<int>(taskData->inputs_count[0]);
    auto* tmp_ptr = reinterpret_cast<int*>(taskData->inputs[0]);
    for (unsigned i = 0; i < taskData->inputs_count[0]; i++) {
      input_[i] = tmp_ptr[i];
    }
    for (int proc = 1; proc < world.size(); proc++) {
      world.send(proc, 0, input_.data() + proc * delta, delta);
    }
  }
  local_input_ = std::vector<int>(delta);
  if (world.rank() == 0) {
    local_input_ = std::vector<int>(input_.begin(), input_.begin() + delta);
  } else {
    world.recv(0, 0, local_input_.data(), delta);
  }
  // Init value for output
  res = 0;
  return true;
}

bool nesterov_a_test_task_mpi::TestMPITaskParallel::validation_impl() {
  if (world.rank() == 0) {
    // Check count elements of output
    return taskData->outputs_count[0] == 1;
  }
  return true;
}

bool nesterov_a_test_task_mpi::TestMPITaskParallel::run_impl() {
  int local_res;
  if (ops == "+") {
    local_res = std::accumulate(local_input_.begin(), local_input_.end(), 0);
  } else if (ops == "-") {
    local_res = -std::accumulate(local_input_.begin(), local_input_.end(), 0);
  } else if (ops == "max") {
    local_res = *std::max_element(local_input_.begin(), local_input_.end());
  }

  if (ops == "+" || ops == "-") {
    reduce(world, local_res, res, std::plus(), 0);
  } else if (ops == "max") {
    reduce(world, local_res, res, boost::mpi::maximum<int>(), 0);
  }
  return true;
}

bool nesterov_a_test_task_mpi::TestMPITaskParallel::post_processing_impl() {
  if (world.rank() == 0) {
    reinterpret_cast<int*>(taskData->outputs[0])[0] = res;
  }
  return true;
}
