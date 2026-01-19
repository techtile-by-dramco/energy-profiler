#ifndef RINGBUFFER_H_
#define RINGBUFFER_H_

#include <stdio.h>
#include <stdint.h>

#define BUFFER_SIZE 10 // Define the size of the ring buffer

typedef struct {
    int32_t buffer_voltage_mv; // Millivolts
    int32_t resistance;        // Ohms
    int32_t pwr_nw;            // NanoWatts
} Measurement;

typedef struct {
    // Measurement buffer[BUFFER_SIZE];
    // int head;
    // int tail;
    // int is_full;
    Measurement buffer[BUFFER_SIZE];
    int head;
    int tail;
    int count;
} RingBuffer;

// void init_ring_buffer(RingBuffer *rb);
// void add_measurement(RingBuffer *rb, int32_t buffer_voltage_mv, int32_t resistance, int32_t pwr_nw);
// int is_empty(RingBuffer *rb);
// int is_full(RingBuffer *rb);
// int get_measurement(RingBuffer *rb, Measurement *measurement);


void init_ring_buffer(RingBuffer *rb);
void add_measurement(RingBuffer *rb, uint16_t buffer_voltage_mv, uint16_t resistance, uint16_t pwr_nw);
int get_measurement(RingBuffer *rb, Measurement *measurement);
int buffer_count(RingBuffer *rb);
int buffer_space(RingBuffer *rb);

#endif