#pragma once

#include <cstring>
#include <memory>

template <class T> class circular_buffer {
private:
    //---------------------------------------------------------------
    // circular_buffer - Private Member Variables
    //---------------------------------------------------------------

    std::unique_ptr<T[]> buffer; // using a smart pointer is safer (and we don't
    // have to implement a destructor)
    size_t head = 0;             // size_t is an unsigned long
    size_t tail = 0;
    size_t max_size = 0;
    T empty_item; // we will use this to clear data
public:
    //---------------------------------------------------------------
    // circular_buffer - Public Methods
    //---------------------------------------------------------------

    // Create a new circular_buffer.
    circular_buffer<T>() = default;

    // Create a new circular_buffer with size.
    explicit circular_buffer<T>(size_t size) {
        resize(size);
    }

    // Resize a new circular_buffer with size.
    void resize(size_t size) {
        max_size = size;
        buffer = std::make_unique<T[]>(size);
        memset(buffer.get(), size * sizeof(T), 0);
    }

    // Clear buffer
    void clear() {
        head = tail = 0;
    }

    // Add items to this circular buffer, overflow is ignored, and return count of pushed items.
    size_t push(const T *items, size_t count) {
        if (!count) return 0;
        if (tail >= head) {
            if (tail + count > max_size) {
                size_t part1 = max_size - tail;
                return push(items, part1) + push(items + part1, count - part1);
            }
            memcpy(&buffer[tail], items, count * sizeof(T));
            tail = (tail + count) % max_size;
            return count;
        }
        if (tail + count + 2 > head) {
            // align to 2
            count = head - tail - 2;
            if (!count) return 0;
        }
        memcpy(&buffer[tail], items, count * sizeof(T));
        tail += count;
        return count;
    }

    // Pop items from this circular buffer, return count of popped up items
    size_t pop(T *items, size_t count) {
        if (!count) return 0;
        if (tail < head) {
            if (head + count > max_size) {
                size_t part1 = max_size - head;
                return pop(items, part1) + pop(items + part1, count - part1);
            }
            memcpy(items, &buffer[head], count * sizeof(T));
            head = (head + count) % max_size;
            return count;
        }
        if (head + count > tail) {
            count = tail - head;
            if (!count) return 0;
        }
        memcpy(items, &buffer[head], count * sizeof(T));
        head += count;
        return count;
    }

    // Return the item at the front of this circular buffer.
    T front() { return buffer[head]; }

    // Return true if this circular buffer is empty, and false otherwise.
    bool is_empty() { return head == tail; }

    // Return true if this circular buffer is full, and false otherwise.
    bool is_full() { return head == (tail + 1) % max_size; }

    // Return the size of this circular buffer.
    size_t size() {
        if (tail >= head)
            return tail - head;
        return max_size - head - tail;
    }

    // Return total space of this circular buffer.
    size_t total() {
        return max_size;
    }
};
