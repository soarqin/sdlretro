#pragma once

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
    }

    // Clear buffer
    void clear() {
        head = tail = 0;
    }

    // Add items to this circular buffer, overflow is ignored, and return count of pushed items.
    size_t push(const T *items, size_t count) {
        if (!count) return 0;
        size_t push_size = 0;
        if (tail >= head) {
            if (tail + count >= max_size) {
                if (head == 0) {
                    push_size = max_size - tail - 1;
                    if (!push_size) return 0;
                    memcpy(&buffer[tail], items, push_size * sizeof(T));
                    tail = max_size - 1;
                } else {
                    push_size = max_size - tail;
                    memcpy(&buffer[tail], items, push_size * sizeof(T));
                    tail = 0;
                }
                items += push_size;
                count -= push_size;
                if (head == 0) {
                    return push_size;
                }
            } else {
                memcpy(&buffer[tail], items, count * sizeof(T));
                tail = tail + count;
                return count;
            }
        }
        if (tail + count >= head) {
            size_t write_size = head - tail - 1;
            memcpy(&buffer[tail], items, write_size * sizeof(T));
            tail += write_size;
            push_size += write_size;
        } else {
            memcpy(&buffer[tail], items, count * sizeof(T));
            tail += count;
            push_size += count;
        }
        return push_size;
    }

    // Pop items from this circular buffer, return count of popped up items
    size_t pop(T *items, size_t count) {
        if (!count) return 0;
        size_t pop_size = 0;
        if (tail < head) {
            if (head + count > max_size) {
                pop_size = max_size - head;
                memcpy(items, &buffer[head], pop_size * sizeof(T));
                head = 0;
                items += pop_size;
                count -= pop_size;
            } else {
                memcpy(items, &buffer[head], count * sizeof(T));
                head = (head + count) % max_size;
                return count;
            }
        }
        if (head + count > tail) {
            size_t read_size = tail - head;
            memcpy(items, &buffer[head], read_size * sizeof(T));
            head += read_size;
            pop_size += read_size;
        } else {
            memcpy(items, &buffer[head], count * sizeof(T));
            head += count;
            pop_size += count;
        }
        return pop_size;
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
};
