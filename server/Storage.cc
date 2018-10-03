
#include "Storage.h"

void Storage::push(uint64_t session_id,
                   uint64_t first_byte_num,
                   std::vector<char> audio_data) {
    {
        std::unique_lock<std::mutex> lock(mutex);
        packages_to_send.push_back({htonll(session_id), htonll(first_byte_num), audio_data});
    }
    c_v.notify_one();
}

package Storage::pop() {
    std::unique_lock<std::mutex> lock(mutex);
    c_v.wait(lock, [this] { return packages_to_send.size() > 0; });
    auto s = packages_to_send.front();
    packages_to_send.pop_front();
    return s;
}

void Storage::notify() {
    push(0, 0, std::vector<char>());
}

void Storage::push_front(std::list<package> &lp) {
    {
        std::unique_lock<std::mutex> lock(mutex);
        for (auto iter = lp.rbegin(); iter != lp.rend(); ++iter) {
            if (iter->audio_data.size() != (size_t)size)syserr("Size of vector2 %d", iter->audio_data.size());
            packages_to_send.push_front({htonll(iter->session_id), htonll(iter->first_byte_num), iter->audio_data});
        }
    }
    c_v.notify_one();
}

size_t Storage::size_of_queue() {
    std::unique_lock<std::mutex> lock(mutex);
    return packages_to_send.size();
}

