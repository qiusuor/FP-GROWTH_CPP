#include "±êÍ·.h"


int main(void) {
	for (;;)
	{
		std::string file_name;
		int sup;
		std::cout << "This is the fp-growth algorithm. Before use it, make sure that the input file has the same form with Groceries.csv in the project.\n\n\n";
		std::cout << "Please input file name£º";
		std::cin >> file_name;
		std::cout << "Please input min suport£º";
		std::cin >> sup;
		double confidence;
		std::cout << "Please input confidence constraint, note that if you don't need confidence constraint, just enter 0£º";
		std::cin >> confidence;
		fp_growth fp(file_name, sup, confidence);
		clock_t start = clock();
		fp.fp();
		clock_t end = clock();
		std::cout << "\n\n";
		std::cout << "time used: " << end - start << " ms\n";
		std::cout << "All finish!\n\n\n";

	}
	

	std::cin.get();
	std::cin.get();
}