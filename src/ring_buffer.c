#include <string.h>
#include <stddef.h>

#include "ring_buffer.h"

// ########################################################################
// defines

#ifndef RB_ENTER_CRITICAL
  #define RB_ENTER_CRITICAL() ((void)0)
  #define RB_EXIT_CRITICAL()  ((void)0)
#endif

// ########################################################################
// private functions

static inline bool is_pow2(uint32_t n)
{
    return n && ((n & (n - 1)) == 0);
}

/* Safe offset calculation to prevent integer overflow */
static inline size_t ring_buffer_offset(uint32_t pos, uint32_t elem_size)
{
    return (size_t)pos * (size_t)elem_size;
}

static inline uint32_t ring_buffer_next_pos(const ring_buffer_t* rb, uint32_t pos)
{
    if ( rb->flags & RB_F_POW2_HINT )
    {
        return (pos + 1) & rb->mask;
    }
    else
    {
        return (pos + 1) % rb->capacity;
    }
}

static inline uint32_t ring_buffer_read_pos(const ring_buffer_t* rb)
{
    // Read position = write_pos - count (with wraparound)
    if ( rb->write_pos >= rb->count )
    {
        return rb->write_pos - rb->count;
    }
    else
    {
        return rb->capacity - (rb->count - rb->write_pos);
    }
}

// ########################################################################
// public functions

// ########## initialization & cleanup ##########

void ring_buffer_init(ring_buffer_t* rb, void* buffer, uint32_t capacity, uint32_t elem_size)
{
    if ( rb == NULL || buffer == NULL || capacity == 0 || elem_size == 0 )
    {
        return; // Invalid parameters
    }

    RB_ENTER_CRITICAL();

    rb->buffer = (uint8_t*)buffer;
    rb->capacity = capacity;
    rb->elem_size = elem_size;
    rb->write_pos = 0;
    rb->count = 0;
    rb->overflow_count = 0;
    bool p2 = is_pow2(capacity);
    rb->flags = p2 ? RB_F_POW2_HINT : 0;
    rb->mask = p2 ? capacity - 1 : 0;

    RB_EXIT_CRITICAL();
}

void ring_buffer_clear(ring_buffer_t* rb)
{
    if ( rb == NULL )
    {
        return;
    }

    RB_ENTER_CRITICAL();
    rb->write_pos = 0;
    rb->count = 0;
    RB_EXIT_CRITICAL();
}

// ########## data operations ##########

bool ring_buffer_push(ring_buffer_t* rb, const void* data, bool overwrite)
{
    if ( rb == NULL || data == NULL )
    {
        return false; // Invalid parameters
    }

    // These should be guaranteed by ring_buffer_init
    assert(rb->buffer != NULL);
    assert(rb->capacity > 0);
    assert(rb->elem_size > 0);

    RB_ENTER_CRITICAL();

    // Check if buffer is full
    if ( rb->count >= rb->capacity )
    {
        if ( !overwrite )
        {
            rb->overflow_count++;
            RB_EXIT_CRITICAL();
            return false; // Element dropped
        }
        // When overwriting, count stays at capacity
    }
    else
    {
        rb->count++;
    }

    size_t offset = ring_buffer_offset(rb->write_pos, rb->elem_size);
    memcpy(rb->buffer + offset, data, rb->elem_size);
    rb->write_pos = ring_buffer_next_pos(rb, rb->write_pos);

    RB_EXIT_CRITICAL();
    return true; // Element successfully pushed
}

bool ring_buffer_pop(ring_buffer_t* rb, void* out_data)
{
    if ( rb == NULL || out_data == NULL )
    {
        return false;
    }

    assert(rb->buffer != NULL);

    RB_ENTER_CRITICAL();

    if ( rb->count == 0 )
    {
        RB_EXIT_CRITICAL();
        return false; // buffer is empty
    }

    uint32_t read_pos = ring_buffer_read_pos(rb);
    size_t offset = ring_buffer_offset(read_pos, rb->elem_size);
    memcpy(out_data, rb->buffer + offset, rb->elem_size);
    rb->count--;

    RB_EXIT_CRITICAL();
    return true;
}

bool ring_buffer_peek(const ring_buffer_t* rb, void* out_data)
{
    if ( rb == NULL || out_data == NULL )
    {
        return false;
    }

    assert(rb->buffer != NULL);

    RB_ENTER_CRITICAL();

    if ( rb->count == 0 )
    {
        RB_EXIT_CRITICAL();
        return false; // buffer is empty
    }

    uint32_t read_pos = ring_buffer_read_pos(rb);
    size_t offset = ring_buffer_offset(read_pos, rb->elem_size);
    memcpy(out_data, rb->buffer + offset, rb->elem_size);

    RB_EXIT_CRITICAL();
    return true;
}

bool ring_buffer_peek_at(const ring_buffer_t* rb, uint32_t index, void* out_data)
{
    if ( rb == NULL || out_data == NULL )
    {
        return false;
    }

    assert(rb->buffer != NULL);

    RB_ENTER_CRITICAL();

    // Check if index is within bounds
    if ( index >= rb->count )
    {
        RB_EXIT_CRITICAL();
        return false;
    }

    // Calculate position: read_pos + index
    uint32_t read_pos = ring_buffer_read_pos(rb);
    uint32_t peek_pos = read_pos;

    // Advance peek_pos by index positions
    for ( uint32_t i = 0; i < index; i++ )
    {
        peek_pos = ring_buffer_next_pos(rb, peek_pos);
    }

    size_t offset = ring_buffer_offset(peek_pos, rb->elem_size);
    memcpy(out_data, rb->buffer + offset, rb->elem_size);

    RB_EXIT_CRITICAL();
    return true;
}

// ########## bulk operations ##########

uint32_t ring_buffer_push_n(ring_buffer_t* rb, const void* data, uint32_t count, bool overwrite)
{
    if ( rb == NULL || data == NULL || count == 0 )
    {
        return 0;
    }

    assert(rb->buffer != NULL);
    assert(rb->capacity > 0);
    assert(rb->elem_size > 0);

    uint32_t pushed = 0;
    const uint8_t* src = (const uint8_t*)data;

    RB_ENTER_CRITICAL();

    for ( uint32_t i = 0; i < count; i++ )
    {
        // Check if buffer is full
        if ( rb->count >= rb->capacity )
        {
            if ( !overwrite )
            {
                rb->overflow_count += (count - i); // Track all dropped elements
                break;                             // Stop if not allowed to overwrite
            }
            // When overwriting, count stays at capacity
        }
        else
        {
            rb->count++;
        }

        size_t offset = ring_buffer_offset(rb->write_pos, rb->elem_size);
        memcpy(rb->buffer + offset, src + i * rb->elem_size, rb->elem_size);
        rb->write_pos = ring_buffer_next_pos(rb, rb->write_pos);
        pushed++;
    }

    RB_EXIT_CRITICAL();
    return pushed;
}

uint32_t ring_buffer_pop_n(ring_buffer_t* rb, void* out_data, uint32_t count)
{
    if ( rb == NULL || out_data == NULL || count == 0 )
    {
        return 0;
    }

    assert(rb->buffer != NULL);

    uint32_t popped = 0;
    uint8_t* dst = (uint8_t*)out_data;

    RB_ENTER_CRITICAL();

    for ( uint32_t i = 0; i < count; i++ )
    {
        if ( rb->count == 0 )
        {
            break; // Buffer is empty
        }

        uint32_t read_pos = ring_buffer_read_pos(rb);
        size_t offset = ring_buffer_offset(read_pos, rb->elem_size);
        memcpy(dst + i * rb->elem_size, rb->buffer + offset, rb->elem_size);
        rb->count--;
        popped++;
    }

    RB_EXIT_CRITICAL();
    return popped;
}

// ########## status & query ##########

bool ring_buffer_empty(const ring_buffer_t* rb)
{
    if ( rb == NULL )
    {
        return true;
    }
    return rb->count == 0;
}

bool ring_buffer_full(const ring_buffer_t* rb)
{
    if ( rb == NULL )
    {
        return false; // NULL buffer is not full, it's invalid
    }
    return rb->count >= rb->capacity;
}

bool ring_buffer_is_initialized(const ring_buffer_t* rb)
{
    if ( rb == NULL )
    {
        return false;
    }
    // A properly initialized buffer has non-NULL buffer pointer and non-zero capacity
    return rb->buffer != NULL && rb->capacity > 0 && rb->elem_size > 0;
}

uint32_t ring_buffer_count(const ring_buffer_t* rb)
{
    if ( rb == NULL )
    {
        return 0;
    }
    return rb->count;
}

uint32_t ring_buffer_available(const ring_buffer_t* rb)
{
    if ( rb == NULL )
    {
        return 0;
    }
    return rb->capacity - rb->count;
}

uint32_t ring_buffer_get_capacity(const ring_buffer_t* rb)
{
    if ( rb == NULL )
    {
        return 0;
    }
    return rb->capacity;
}

uint32_t ring_buffer_bytes_used(const ring_buffer_t* rb)
{
    if ( rb == NULL )
    {
        return 0;
    }
    return rb->count * rb->elem_size;
}

uint32_t ring_buffer_bytes_available(const ring_buffer_t* rb)
{
    if ( rb == NULL )
    {
        return 0;
    }
    return (rb->capacity - rb->count) * rb->elem_size;
}

// ########## overflow tracking ##########

uint32_t ring_buffer_get_overflow_count(const ring_buffer_t* rb)
{
    if ( rb == NULL )
    {
        return 0;
    }
    return rb->overflow_count;
}

void ring_buffer_reset_overflow_count(ring_buffer_t* rb)
{
    if ( rb == NULL )
    {
        return;
    }

    RB_ENTER_CRITICAL();
    rb->overflow_count = 0;
    RB_EXIT_CRITICAL();
}
