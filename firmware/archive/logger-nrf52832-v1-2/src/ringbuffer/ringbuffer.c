#include "ringbuffer.h"


void init_ring_buffer(RingBuffer *rb) {
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;
}

void add_measurement(RingBuffer *rb, uint16_t buffer_voltage_mv, uint16_t resistance, uint16_t pwr_nw) {
    rb->buffer[rb->head].buffer_voltage_mv = buffer_voltage_mv;
    rb->buffer[rb->head].resistance = resistance;
    rb->buffer[rb->head].pwr_nw = pwr_nw;

    if(rb->count == BUFFER_SIZE) {
        rb->tail = (rb->tail + 1) % BUFFER_SIZE; // Overwrite the oldest data
    } else {
        rb->count++;
    }

    rb->head = (rb->head + 1) % BUFFER_SIZE;
}

int get_measurement(RingBuffer *rb, Measurement *measurement) {
    if (rb->count == 0) {
        return -1; // Buffer is empty
    }

    *measurement = rb->buffer[rb->tail];
    rb->tail = (rb->tail + 1) % BUFFER_SIZE;
    rb->count--;

    return 0; // Success
}

int buffer_count(RingBuffer *rb) {
    return rb->count;
}

int buffer_space(RingBuffer *rb) {
    return BUFFER_SIZE - rb->count;
}
// void init_ring_buffer(RingBuffer *rb) {
//     rb->head = 0;
//     rb->tail = 0;
//     rb->is_full = 0;
// }

// void add_measurement(RingBuffer *rb, int32_t buffer_voltage_mv, int32_t resistance, int32_t pwr_nw) {
//     rb->buffer[rb->head].buffer_voltage_mv = buffer_voltage_mv;
//     rb->buffer[rb->head].resistance = resistance;
//     rb->buffer[rb->head].pwr_nw = pwr_nw;

//     if(rb->is_full) {
//         rb->tail = (rb->tail + 1) % BUFFER_SIZE;
//     }

//     rb->head = (rb->head + 1) % BUFFER_SIZE;
//     rb->is_full = (rb->head == rb->tail);
// }

// int is_empty(RingBuffer *rb) {
//     return (!rb->is_full && (rb->head == rb->tail));
// }

// int is_full(RingBuffer *rb) {
//     return rb->is_full;
// }

// int get_measurement(RingBuffer *rb, Measurement *measurement) {
//     if (is_empty(rb)) {
//         return -1; // Buffer is empty
//     }

//     *measurement = rb->buffer[rb->tail];
//     rb->is_full = 0;
//     rb->tail = (rb->tail + 1) % BUFFER_SIZE;

//     return 0; // Success
// }