#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>

class RingBuffer {
public:
    RingBuffer(uint8_t* buffer, size_t capacity)
        : buf_(buffer), cap_(capacity), head_(0), tail_(0), count_(0) {}

    size_t capacity() const { return cap_; }
    size_t size() const { return count_; }
    
    // --- CORREÇÃO: Adicionado empty() para compatibilidade ---
    bool empty() const { return count_ == 0; }
    bool isEmpty() const { return empty(); }
    
    bool isFull() const { return count_ == cap_; }

    // Alias comum usado por drivers
    size_t available() const { return count_; }

    void clear() { head_ = tail_ = count_ = 0; }

    bool push(uint8_t v) {
        if (isFull()) return false;
        buf_[head_] = v;
        head_ = (head_ + 1) % cap_;
        count_++;
        return true;
    }

    bool pop(uint8_t& out) {
        if (isEmpty()) return false;
        out = buf_[tail_];
        tail_ = (tail_ + 1) % cap_;
        count_--;
        return true;
    }

    // Sobrecarga útil
    uint8_t pop() {
        uint8_t v = 0;
        pop(v);
        return v;
    }

    bool peek(uint8_t& out) const {
        if (isEmpty()) return false;
        out = buf_[tail_];
        return true;
    }

    std::vector<uint8_t> peekAll() const {
        std::vector<uint8_t> ret;
        ret.reserve(count_);
        size_t idx = tail_;
        for (size_t i = 0; i < count_; i++) {
            ret.push_back(buf_[idx]);
            idx = (idx + 1) % cap_;
        }
        return ret;
    }

    std::vector<uint8_t> popBytes(size_t n) {
        std::vector<uint8_t> ret;
        ret.reserve(n);
        for (size_t i = 0; i < n && !isEmpty(); i++) {
            ret.push_back(pop());
        }
        return ret;
    }

private:
    uint8_t* buf_;
    size_t cap_;
    size_t head_;
    size_t tail_;
    size_t count_;
};