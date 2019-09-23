/* -----------------------------------------------------------------------------
    SYNTAX EXAMPLE
----------------------------------------------------------------------------- */
/*
    repeat(x){
        // code here
    }

    Will repeat the provided code x times. (A counter "i" storing the index will
    always be present)

    Ex:
    repeat(5)
        std::cout << "This line will be repeated 5 times" << std::endl;
*/

/*
    repeat(x, index){
        // code here
    }

    Will repeat the provided code x times, storing the current index in a
    variable with the name of the second input.

    Ex:
    repeat(5, index)
        std::cout << "Current index: " << index << std::endl;

    These loops are optimized with reverse for, meaning that the index will
    start at x and work its way down to zero. The optimization can be disabled
    in applications where having the unreversed order is critical by defining:
        #define REPEAT_DISABLE_OPTIMIZATIONS
    Before including this header.
*/

/*
    repeat(x, type, index){
        // code here
    }

    Will repeat the provided code x times, storing the current index an a
    variable type dictated by the second input and a name provided by the third

    This option shouldn't be necessary for 99% of applications.
*/
#ifndef _REPEAT_H_
#define _REPEAT_H_

// Macros defining Repeat Syntax
#ifndef REPEAT_DISABLE_OPTIMIZATIONS
#define _REPEAT_TIMES_TYPE_(x, type, y) for(type y = x; y--;)
#elif
#define _REPEAT_TIMES_TYPE_(x, type, y) for(type y = 0; y < x; y++)
#endif // REPEAT_DISABLE_OPTIMIZATIONS
#define _REPEAT_TIMES_(x, y) _REPEAT_TIMES_TYPE_(x, size_t, y)
#define _REPEAT_(x) _REPEAT_TIMES_(x, i)
// Macro overloading from: https://stackoverflow.com/questions/11761703/overloading-macro-on-number-of-arguments
#define _GET_REPEAT_(_1,_2,_3, NAME,...) NAME
#define repeat(...) _GET_REPEAT_(__VA_ARGS__, _REPEAT_TIMES_TYPE_, _REPEAT_TIMES_, _REPEAT_)(__VA_ARGS__)
#endif // _REPEAT_H_
