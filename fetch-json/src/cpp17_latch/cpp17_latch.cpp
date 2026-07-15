//
//  cpp17_latch.cpp
//  fetch-json
//
//  Created by Corey Ferguson on 7/9/26.
//

#include "cpp17_latch.h"

// Constructors

cpp17_latch::cpp17_latch(const int count) {
    this->_count = count;
}

// Member Functions

void cpp17_latch::count_down() {
    if (this->_count.fetch_sub(1) == 1) {
        std::unique_lock<std::mutex> lock(this->_mutex);
        
        this->_cv.notify_all();
    }
}

void cpp17_latch::wait() {
    std::unique_lock<std::mutex> lock(this->_mutex);

    this->_cv.wait(lock, [this] {
        return this->_count.load() == 0;
    });
}

