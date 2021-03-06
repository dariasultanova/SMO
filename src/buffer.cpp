#include "buffer.h"
#include "counter.h"

#include <stdexcept>
#include <string>

#include <iostream> // TODO: remove

Buffer::Buffer(const unsigned size, Counter_ptr counter) :
  occupied_(0)
{
  if (counter == nullptr) {
    throw std::invalid_argument("counter must be not null");
  }
  counter_ = counter;
  buffer_ = std::vector<Request_ptr>(size, nullptr);
}

void Buffer::add(const Request_ptr request) {
  if (request == nullptr) {
    throw std::invalid_argument("request null ptr");
  }

  for (auto it = buffer_.begin(); it != buffer_.end(); ++it) {
    if (*it == nullptr) {
      *it = request;
      ++occupied_;
      return;
    }
  }

  // all places are occupied
  unsigned priority = request->get_priority();
  counter_->add_rejected(priority);
  throw std::logic_error("rejection for " + std::to_string(priority));
}


Buffer::Request_ptr Buffer::get()
{
  // if no requests in buffer
  if (occupied_ == 0) {
    throw std::logic_error("no requests in buffer");
    //return nullptr;
  }

  // find first not empty place
  unsigned int min;
  for (int i = 0; ; ++i) {
    if (buffer_[i] != nullptr) {
      min = i;
      break;
    }
  }

  auto min_priority = buffer_[min]->get_priority();
  for (unsigned i = (min + 1); i < buffer_.size(); ++i) {
      if (buffer_[i] == nullptr) {
        continue;
      }

      auto current_priority = buffer_[i]->get_priority();

      // if priorities are not the same
      if (current_priority < min_priority) {
        min_priority = current_priority;
        min = i;
        continue;
      }

      // if priorities are the same
      if (current_priority == min_priority) {
        if (buffer_[i]->get_creation_time() < buffer_[min]->get_creation_time()) {
          min = i;
        }
        //continue;
      }

  } // end of min search

  Request_ptr request = buffer_[min];
  buffer_[min] = nullptr;
  --occupied_;

  return request;
}

void Buffer::print_reqs() const {
  int i = 0;
  for (Request_ptr req : buffer_) {
    std::cout << "[" << i++ << "] = ";
    if (req == nullptr) {
      std::cout << "null\n";
      continue;
    }
    std::cout << req->get_priority()      << "."
              << req->get_number()        << ", creation: "
              << req->get_creation_time() << "\n";
  }
}

Buffer::state_t Buffer::get_state() const {
  std::vector<std::string> state;

  for (Request_ptr req : buffer_) {
    if (req == nullptr) {
      state.push_back("empty");
      continue;
    }

    auto src = std::to_string(req->get_priority() + 1);
    auto num = std::to_string(req->get_number());

    state.push_back(src + "." + num);
  }

  return state;
}
