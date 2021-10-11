#pragma once
#include <iostream>

// COLORS 
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

// DEBUGGING
#define DEBUG_VERBOSE 0

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#if DEBUG_VERBOSE == 1 
#define DEBUG_MESSAGE(message, ...) { \
    printf(ANSI_COLOR_YELLOW "[DEBUG: %s:%i] " #message ANSI_COLOR_RESET "\n" , __FILENAME__, __LINE__, ##__VA_ARGS__); \
}
#else
#define DEBUG_MESSAGE(message, ...) 
#endif

#define ERROR_MESSAGE(message, ...) { \
    fprintf(stderr, ANSI_COLOR_RED "[ERROR: %s:%i] " #message  ANSI_COLOR_RESET "\n", __FILENAME__, __LINE__, ##__VA_ARGS__); \
}

// MKL SWITCH
#define ORIGINAL_CONV 0
#define MKL_MATRIX_CONV 1
#define MKL_DOTPRODUCT_CONV 2
#define ENABLE_MKL 0

// Number of layers
#define LAYERS_PER_FRAMEWORK 8

//#define FRAMES 20
#define NUM_REPETITIONS 1000
//define the size of each layer

#define CH0 1

//Layer0 Input
#define INPUT_LAYER_ID 0

//Layer1 Conv
#define FIRST_CONV_LAYER_ID 1
#define INPUT1 32
#define STRIDE1 1
#define KERNEL1 5
#define IC1 4
#define JC1 1
#define CH1 6
#define OUTPUT1 28

//Layer2 Pooling
#define FIRST_POOLING_LAYER_ID 2
#define INPUT2 OUTPUT1
#define KERNEL2 2
#define CH2 6
#define OUTPUT2 14

//Layer3 Conv
#define SECOND_CONV_LAYER_ID 3
#define INPUT3 6
#define STRIDE3 1
#define KERNEL3 5
#define IC3 2
#define JC3 1
#define CH3 16
#define OUTPUT3 10

//Layer4 Pooling
#define SECOND_POOLING_LAYER_ID 4
#define INPUT4 OUTPUT3
#define KERNEL4 2
#define CH4 16
#define OUTPUT4 5

//Layer5 Conv2Full
#define CONV2FULL_LAYER_ID 5
#define INPUT5 OUTPUT4
#define KERNEL5 5
#define CUT5 10
#define OUTPUT5 120

//Layer6 Full
#define FULL_LAYER_ID 6
#define INPUT6 OUTPUT5
#define OUTPUT6 10
#define CUT6 2

#define OUTPUT OUTPUT6
//Layer7 Gauss
#define FINISH_LAYER_ID 7

