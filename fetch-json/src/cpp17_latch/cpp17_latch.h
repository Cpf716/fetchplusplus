#include <atomic>
#include <condition_variable>
#include <mutex>

class cpp17_latch {
    // Member Fields

    std::atomic<int>        _count;
    std::condition_variable _cv;
    std::mutex              _mutex;
public:
    // Constructors

    cpp17_latch(const int count);

    // Member Functions

    void count_down();

    void wait();
};
