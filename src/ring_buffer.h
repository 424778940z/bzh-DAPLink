/**
 * @file ring_buffer.h
 * @brief Generic ring buffer implementation for embedded systems
 *
 * OVERVIEW:
 * ---------
 * A generic, cache-friendly ring buffer supporting arbitrary element sizes.
 * Optimized for power-of-2 capacities using bitmask operations.
 *
 * MEMORY MODEL:
 * -------------
 * - Uses caller-provided buffer (no dynamic allocation)
 * - Buffer must remain valid for lifetime of ring_buffer_t
 * - No alignment requirements enforced (caller's responsibility)
 *
 * THREAD SAFETY:
 * --------------
 * This implementation is NOT thread-safe by default. For concurrent access:
 * 1. Define RB_ENTER_CRITICAL() and RB_EXIT_CRITICAL() macros before including this header
 * 2. Or protect all operations with external synchronization (mutex, etc.)
 * 3. For single-producer/single-consumer without contention, it may work but is not guaranteed
 *
 * Example thread-safe setup:
 *   #define RB_ENTER_CRITICAL() __disable_irq()
 *   #define RB_EXIT_CRITICAL()  __enable_irq()
 *   #include "ring_buffer.h"
 *
 * CAPACITY LIMITS:
 * ----------------
 * - Maximum capacity: UINT32_MAX elements
 * - Maximum elem_size: Limited by (UINT32_MAX / capacity) to prevent overflow
 * - Total buffer size must not exceed SIZE_MAX
 *
 * ERROR HANDLING:
 * ---------------
 * - Functions returning bool: false indicates failure (empty/full/null params)
 * - Functions returning uint32_t: 0 indicates error or empty state
 * - void functions: silently ignore invalid parameters (check with assertions if enabled)
 *
 * PERFORMANCE:
 * ------------
 * - Power-of-2 capacities: O(1) index calculation using bitmask
 * - Non-power-of-2: O(1) with modulo operation
 * - All operations are O(1) except bulk operations which are O(n)
 *
 * @note Intended for single-threaded use or with external synchronization
 * @note Does not handle buffer memory lifetime (caller's responsibility)
 */

#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>

// ########################################################################
// defines

/* Optional thread-safety: define before including to override (0 = no IRQ save/restore) */
#ifndef RB_THREAD_SAFE
  #define RB_THREAD_SAFE 1
#endif

#if RB_THREAD_SAFE
  #include "stm32f4xx.h"
  #ifndef RB_ENTER_CRITICAL
    #define RB_ENTER_CRITICAL() __disable_irq()
    #define RB_EXIT_CRITICAL()  __enable_irq()
  #endif
#endif

/* Flag bits for ring_buffer_t.flags field */
#define RB_F_POW2_HINT (1u << 0) /* Internal: capacity is power-of-two (auto-detected, enables bitmask optimization) */
/* Bits 1-31 reserved for future use */

// ########################################################################
// variables

typedef struct
{
    uint8_t* buffer;         // pointer to data storage
    uint32_t elem_size;      // size of one element in bytes
    uint32_t capacity;       // number of elements in the buffer
    uint32_t write_pos;      // next write position
    uint32_t count;          // current number of stored elements
    uint32_t mask;           // valid iff capacity is power-of-two
    uint32_t flags;          // flags
    uint32_t overflow_count; // number of elements dropped due to overflow (when overwrite=false)
} ring_buffer_t;

// ########################################################################
// private functions

// ########################################################################
// public functions

// ########## initialization & cleanup ##########

/**
 * Initialize the ring buffer.
 * @param rb         pointer to ring buffer object
 * @param buffer     memory storage for elements
 * @param capacity   maximum number of elements
 * @param elem_size  size of one element in bytes
 */
void ring_buffer_init(ring_buffer_t* rb, void* buffer, uint32_t capacity, uint32_t elem_size);

/**
 * Clear the buffer (reset write_pos/count but keep memory).
 * @param rb         pointer to ring buffer object
 */
void ring_buffer_clear(ring_buffer_t* rb);

// ########## data operations ##########

/**
 * Push an element into the ring buffer.
 * @param rb         pointer to ring buffer object
 * @param data       pointer to element to be copied
 * @param overwrite  if true, overwrite the oldest element when full
 * @return true if element was pushed, false if dropped (full and overwrite=false)
 */
bool ring_buffer_push(ring_buffer_t* rb, const void* data, bool overwrite);

/**
 * Pop an element from the ring buffer.
 * @param rb         pointer to ring buffer object
 * @param out_data   pointer to store popped element
 * @return true if successful, false if buffer is empty
 */
bool ring_buffer_pop(ring_buffer_t* rb, void* out_data);

/**
 * Peek at the next element without removing it.
 * @param rb         pointer to ring buffer object
 * @param out_data   pointer to store peeked element
 * @return true if successful, false if buffer is empty
 */
bool ring_buffer_peek(const ring_buffer_t* rb, void* out_data);

/**
 * Peek at element at specific offset from read position.
 * @param rb         pointer to ring buffer object
 * @param index      offset from current read position (0 = next element to be popped)
 * @param out_data   pointer to store peeked element
 * @return true if successful, false if index out of range or buffer error
 */
bool ring_buffer_peek_at(const ring_buffer_t* rb, uint32_t index, void* out_data);

// ########## bulk operations ##########

/**
 * Push multiple elements into the ring buffer.
 * @param rb         pointer to ring buffer object
 * @param data       pointer to array of elements to be copied
 * @param count      number of elements to push
 * @param overwrite  if true, overwrite oldest elements when full
 * @return number of elements actually pushed
 */
uint32_t ring_buffer_push_n(ring_buffer_t* rb, const void* data, uint32_t count, bool overwrite);

/**
 * Pop multiple elements from the ring buffer.
 * @param rb         pointer to ring buffer object
 * @param out_data   pointer to buffer to store popped elements
 * @param count      maximum number of elements to pop
 * @return number of elements actually popped (may be less than count if buffer empties)
 */
uint32_t ring_buffer_pop_n(ring_buffer_t* rb, void* out_data, uint32_t count);

// ########## status & query ##########

/**
 * Check if buffer is empty.
 * @param rb         pointer to ring buffer object
 * @return true if empty, false otherwise
 */
bool ring_buffer_empty(const ring_buffer_t* rb);

/**
 * Check if buffer is full.
 * @param rb         pointer to ring buffer object
 * @return true if full, false otherwise
 */
bool ring_buffer_full(const ring_buffer_t* rb);

/**
 * Check if ring buffer is properly initialized.
 * @param rb         pointer to ring buffer object
 * @return true if initialized and valid, false otherwise
 */
bool ring_buffer_is_initialized(const ring_buffer_t* rb);

/**
 * Get number of elements currently in buffer.
 * @param rb         pointer to ring buffer object
 * @return number of elements stored
 */
uint32_t ring_buffer_count(const ring_buffer_t* rb);

/**
 * Get number of available slots for new elements.
 * @param rb         pointer to ring buffer object
 * @return number of free slots
 */
uint32_t ring_buffer_available(const ring_buffer_t* rb);

/**
 * Get the capacity of the ring buffer.
 * @param rb         pointer to ring buffer object
 * @return maximum number of elements, or 0 if rb is NULL
 */
uint32_t ring_buffer_get_capacity(const ring_buffer_t* rb);

/**
 * Get total bytes used by stored elements.
 * @param rb         pointer to ring buffer object
 * @return number of bytes in use
 */
uint32_t ring_buffer_bytes_used(const ring_buffer_t* rb);

/**
 * Get available space in bytes.
 * @param rb         pointer to ring buffer object
 * @return number of bytes available
 */
uint32_t ring_buffer_bytes_available(const ring_buffer_t* rb);

// ########## overflow tracking ##########

/**
 * Get the overflow counter (number of elements dropped).
 * @param rb         pointer to ring buffer object
 * @return number of elements that were dropped due to overflow
 */
uint32_t ring_buffer_get_overflow_count(const ring_buffer_t* rb);

/**
 * Reset the overflow counter to zero.
 * @param rb         pointer to ring buffer object
 */
void ring_buffer_reset_overflow_count(ring_buffer_t* rb);
