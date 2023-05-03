#ifndef __circular_buffer_hpp__
#define __circular_buffer_hpp__

#include <iostream>
#include <memory>
#include <condition_variable>

class BufferItem {  
  public:
  std::mutex bmtx;
  uint64_t data;
  size_t sz;
};

class CircularBuffer {
  private:
  size_t max_items;
  size_t head;
  size_t tail;
  bool full;
  std::unique_ptr<BufferItem []> buffer;
  std::mutex cmtx;
  std::condition_variable cv;
  public:
  CircularBuffer(size_t _max_items);
  ~CircularBuffer() = default;
  void putData(const char * data, size_t sz);
  char * retrieveData();
  size_t getMax();
  bool isEmpty();
  bool isFull();
};

#endif