#include <iostream>
#include <chrono>
#include "repeat.h"
#include "timer.h"

//using namespace std;



int main(){
	{
		Timer t;
		for(int i = 0; i < 1000000; i++)
			std::cout << "";
	}

	{
		Timer t;
		for(int i = 1000000; i--;)
			std::cout << "";
	}


	{
		Timer t;
		repeat(1000000, i)
			std::cout << "";
	}

	return 0;
}
