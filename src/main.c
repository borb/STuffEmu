/*
 * This file is part of the STuffEmu (https://github.com/BigWhale/STuffEmu)
 * Copyright (c) 2018 David Klasinc
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/time.h>

#include "main.h"
#include "input.h"
#include "output.h"
#include "hw_defs.h"

pthread_mutex_t mouse_mutex = PTHREAD_MUTEX_INITIALIZER;

void sig_handler(int signum) {
    if (signum == SIGUSR1) {
        printf("Got signal USR1, bailing out!\n");
        exit(0);
    }
}

int main( int argc, char *argv[]) {
    int input_dev;
    input_arg thread_arg;
    bool joy = false;
    pthread_t t_input;
    pthread_t t_output;

    signal(SIGINT, sig_handler);

    if (argc == 2) {
        if (strncmp(argv[1], "-j", 2) == 0) {
            joy = true;
        }
    }

    if (joy) {
        if ((input_dev = open(JOY_DEV, O_RDONLY)) == -1) {
            printf("Unable to open joystick.\n");
            exit(EXIT_FAILURE);
        }
    } else {
        if ((input_dev = open(MOUSE_DEV, O_RDONLY)) == -1) {
            printf("Unable to open mouse.\n");
            exit(EXIT_FAILURE);
        }
    }

    thread_arg.device = input_dev;
    thread_arg.joystick = joy;

    /* All is well, prepare GPIO */
    start_gpio();
    printf("Starting emulation.\n");
    pthread_create(&t_input, NULL, input_thread, &thread_arg);
    pthread_create(&t_output, NULL, output_thread, NULL);
    printf("Waiting for threads.\n");

    /* Wait for the input thread to stop */
    pthread_join(t_input, NULL);
    pthread_join(t_output, NULL);

    return 0;
}