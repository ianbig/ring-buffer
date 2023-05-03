#include <cstdlib>
#include <cstring>
#include <mutex>

#include "CircularBuffer.hpp"

CircularBuffer::CircularBuffer(size_t _max_items) : max_items(_max_items), head(0), tail(0), full(false), buffer(std::make_unique<BufferItem[]>(max_items)) {
}

void CircularBuffer::putData(const char * data, size_t sz) {
  {
    std::lock_guard<std::mutex> itemlck(buffer[head].bmtx);
    strncpy((char*)(&buffer[head].data), data, sz);
    buffer[head].sz = sz;
  }

  {
    std::unique_lock<std::mutex> buffLck(cmtx);
    if (isFull()) {
      tail = (tail + 1) % max_items;
    }
    head = (head + 1) % max_items;
    full = (head == tail);
    cv.notify_all();
  }
}

char * CircularBuffer::retrieveData() {
  {
    std::unique_lock<std::mutex> gLck(cmtx);
    if (isEmpty()) {
      cv.wait(gLck, [&]() { return !isEmpty(); });
    }
  }

  char * ret = nullptr;
  {
    std::lock_guard<std::mutex> itemLck(buffer[tail].bmtx);
    ret = (char*)(&buffer[tail].data);
    ret[buffer[tail].sz] = '\0';
  }
  
  {
    std::lock_guard<std::mutex> gLck(cmtx);
    tail = (tail + 1) % max_items;
    full = false;
  }
  
  return ret;
}


size_t CircularBuffer::getMax() {
  return this->max_items;
}


bool CircularBuffer::isEmpty() {
  return head == tail && !full;
}

bool CircularBuffer::isFull() {
  return head == tail && full;
}