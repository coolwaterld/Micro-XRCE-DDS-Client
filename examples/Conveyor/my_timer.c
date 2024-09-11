#include "my_timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Internal function to handle the signal and call the user-defined callback
void internal_timer_handler(int signum, siginfo_t *si, void *uc) {
    my_timer_t *timer = si->si_value.sival_ptr;
    if (timer && timer->callback) {
        timer->callback(signum, timer->callback_args);  // Call the user-defined callback with arguments
    }
}

// Create a timer instance
my_timer_t* create_timer_instance() {
    my_timer_t* timer = (my_timer_t*)malloc(sizeof(my_timer_t));
    if (!timer) {
        perror("Failed to allocate memory for timer instance");
        exit(EXIT_FAILURE);
    }
    memset(timer, 0, sizeof(my_timer_t));

    // Configure the signal action to use the internal handler
    timer->sa.sa_flags = SA_SIGINFO;
    timer->sa.sa_sigaction = internal_timer_handler; // Use sigaction for passing additional data
    sigemptyset(&timer->sa.sa_mask);

    if (sigaction(SIGRTMIN, &timer->sa, NULL) == -1) {
        perror("sigaction");
        free(timer);
        exit(EXIT_FAILURE);
    }

    // Configure the timer to send SIGRTMIN upon expiration
    timer->sev.sigev_notify = SIGEV_SIGNAL;
    timer->sev.sigev_signo = SIGRTMIN;
    timer->sev.sigev_value.sival_ptr = timer;

    if (timer_create(CLOCK_REALTIME, &timer->sev, &timer->timer_id) == -1) {
        perror("timer_create");
        free(timer);
        exit(EXIT_FAILURE);
    }

    return timer;
}

// Set the callback function and its arguments for the timer
void set_timer_callback(my_timer_t *timer, void (*callback)(int, void*), void *args) {
    timer->callback = callback;     // Set the user-defined callback function
    timer->callback_args = args;    // Set the user-defined arguments
}

// Set the timeout duration for the timer (seconds)
void set_timer_timeout(my_timer_t *timer, int seconds) {
    timer->its.it_value.tv_sec = seconds;   // Initial expiration time in seconds
    timer->its.it_value.tv_nsec = 0;        // Initial expiration time in nanoseconds
    timer->its.it_interval.tv_sec = 0;      // Interval of 0 seconds (one-shot)
    timer->its.it_interval.tv_nsec = 0;     // Interval of 0 nanoseconds
}

// Start the timer
void start_timer(my_timer_t *timer) {
    if (timer_settime(timer->timer_id, 0, &timer->its, NULL) == -1) {
        perror("timer_settime");
        exit(EXIT_FAILURE);
    }

    // printf("Timer started for %ld seconds...\n", timer->its.it_value.tv_sec);
}

// Delete the timer instance
void delete_timer_instance(my_timer_t *timer) {
    if (timer_delete(timer->timer_id) == -1) {
        perror("timer_delete");
    }
    free(timer);
}
