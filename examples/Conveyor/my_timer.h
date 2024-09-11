#ifndef MY_TIMER_H
#define MY_TIMER_H

#include <signal.h>
#include <time.h>

// Define a timer handle type
typedef struct {
    timer_t timer_id;                    // POSIX timer ID
    struct sigevent sev;                 // Signal event configuration
    struct itimerspec its;               // Timer specification
    struct sigaction sa;                 // Signal action for callback
    void (*callback)(int, void*);        // User-defined callback function with argument
    void *callback_args;                 // Arguments to pass to the callback function
} my_timer_t;

// Function to create a timer instance
my_timer_t* create_timer_instance();

// Function to set the callback function for the timer
void set_timer_callback(my_timer_t *timer, void (*callback)(int, void*), void *args);

// Function to set the timeout duration for the timer (seconds)
void set_timer_timeout(my_timer_t *timer, int seconds);

// Function to start the timer
void start_timer(my_timer_t *timer);

// Function to delete the timer instance
void delete_timer_instance(my_timer_t *timer);

#endif // MY_TIMER_H
