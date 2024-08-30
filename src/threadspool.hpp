#include <condition_variable>
#include <functional>
#include <mutex>
#include <sys/types.h>
#include <thread>
#include <vector>
class ThreadSpool {
public:
  ThreadSpool(uint threads) : thread_count(threads) {}
  ~ThreadSpool() {
    for (auto &thread : threads) {
      if (thread.joinable()) {
        thread.join();
      }
    }
  }

  void push_thread(std::function<void()> thread) {
    std::unique_lock<std::mutex> lock(thread_lock);
    while (used_threads >= thread_count) {
      cv.wait(lock);
    }

    used_threads++;
    threads.emplace_back(std::thread([this, thread]() {
      thread();
      thread_lock.lock();
      used_threads--;
      cv.notify_one();
      thread_lock.unlock();
    }));
    // (threads.end() - 1)->detach();
  }

private:
  uint thread_count;
  uint used_threads = 0;
  std::vector<std::thread> threads;
  std::mutex thread_lock;
  std::condition_variable cv;
};
