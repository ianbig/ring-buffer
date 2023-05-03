#include <iostream>
#include <cassert>
#include <cstring>
#include <vector>
#include <memory>
#include <future>
#include <unordered_map>
#include <thread>

#include "CircularBuffer.hpp"

CircularBuffer gbuf(10);
std::unordered_map<std::string, int> writeCount;
std::mutex mapLck;

void write_data(const char * data) {
  std::this_thread::sleep_for (std::chrono::seconds(1));
  gbuf.putData(data, strlen(data));
  {
    std::lock_guard<std::mutex> lck(mapLck);
    writeCount[data]++;
  }
}

void write_data_concurrent() {
  const char* sentence[3] = {"Ian", "John", "hire me"};
  std::vector<std::future<void>> tasks;
  for (auto i = 0; i < 10; i++) {
    tasks.push_back(std::async(std::launch::async, write_data, sentence[i % 3]));
  }

  for (auto & task : tasks) {
    task.wait();
  }
}

void read_data() {
  const char * getData = gbuf.retrieveData();
  {
    std::lock_guard<std::mutex> lck(mapLck);
    writeCount[getData]--;
  }
}

void read_data_concurrent() {
  std::vector<std::future<void>> readers;
  for (auto i = 0; i < 10; i++) {
    readers.push_back(std::async(std::launch::async, read_data));
  }

  for (auto & reader : readers) {
    reader.wait();
  }
}

const char * read_test() {
  const char * getData = gbuf.retrieveData();
  {
    std::lock_guard<std::mutex> lck(mapLck);
    writeCount[getData]--;
  }

  return getData;
}

void overread() {
  const char* sentence[3] = {"Ian", "John", "hire me"};
  auto reader = std::async(read_test);
  auto writer = std::async(write_data, sentence[1]);
  writer.wait();

  std::string ans = reader.get();
  assert(ans == std::string(sentence[1]));
}

int main() {
  CircularBuffer buffer(10);
  // test initialization
  assert(buffer.getMax() == 10);
  assert(buffer.isEmpty());
  assert(!buffer.isFull());
  std::cout << "passed initialization" << std::endl;
  // test normal put
  for (int i = 0; i < 10; i++) {
    buffer.putData(std::to_string(i).c_str(), std::to_string(i).size());
  }

  assert(buffer.isFull());
  assert(!buffer.isEmpty());
  // test normal get, test FIFO property
  for (int i = 0; i < 10; i++) {
    std::string data = buffer.retrieveData();
    std::string expected = std::to_string(i);
    assert(data == expected);
  }
  assert(!buffer.isFull());
  assert(buffer.isEmpty());
  std::cout << "normal operation passed" << std::endl;
  // test override put
  for (int i = 0; i < 13; i++) {
    buffer.putData(std::to_string(i).c_str(), std::to_string(i).size());
  }
  std::string getData = buffer.retrieveData();
  assert(getData == "3");
  
  // test over read
  int ans = 4;
  for (int i = 0; i < 9; i++) {
    std::string data = buffer.retrieveData();
    std::string expected = std::to_string(ans++);
    assert(data == expected);
  }

  std::cout << "passed overwrite" << std::endl;
  // test concurrency
  write_data_concurrent();
  read_data_concurrent();
  for (auto it = writeCount.begin(); it != writeCount.end(); it++) {
    assert(it->second == 0);
  }
  
  overread();
  std::cout << "passed concurrency code" << std::endl;
}