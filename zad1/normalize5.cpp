#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <string>
#include <iostream>
#include <cctype>
#include <cstring>

#define REPEATS 50000

static double gtod_ref_time_sec = 0.0;

// Adapted from the bl2_clock() routine in the BLIS library
double dclock() {
	double the_time, norm_sec;
	struct timeval tv;
	gettimeofday(&tv, NULL);

	if(gtod_ref_time_sec == 0.0)
    	gtod_ref_time_sec = (double)tv.tv_sec;
  
	norm_sec = (double)tv.tv_sec - gtod_ref_time_sec;
	the_time = norm_sec + tv.tv_usec * 1.0e-6;

	return the_time;
}

// function to be optimized
// fourth optimization - char* instead of std::string
void process_large_string(const char* s, char* result, size_t size) {
	std::string last, current;
	bool flag;

	// to use in memcpy
	char* space = (char*)" ";
	char* comma = (char*)",";

    for(int i = 0; i < size; i++) {
		if(isspace(s[i])) {
			if(current != last) {
				if(current.length()) {
					memcpy(result, current.data(), current.length()); // 4th optimization
					result += current.length();
					last = current;
				}

				if(flag) {
					memcpy(result, space, 1); // 4th optimization
					result += 1;
				}
			}

			current.clear();
			flag = false;
		} else if(ispunct(s[i])) {
			if(current.length() && current != last) {
				memcpy(result, current.data(), current.length()); // 4th optimization
				result += current.length();
				last = current;
			}

			memcpy(result, comma, 1); // 4th optimization
			result += 1;
			current.clear();
			flag = true;
		} else if(s[i] >= 0x20 && s[i] <= 0x7E) {
			current += (char)tolower(s[i]); // 2nd optimization
			flag = true;
		}
    }

    *result = 0;
}

int main(int argc, const char* argv[]) {
	int i, j, k, iret;
	double dtime;
	std::string s, result, line;
  
	std::cout << "Let's start processing the file\n";
  
	while(getline(std::cin, line))
    	s += line + "\n";

	char* results = (char*)malloc(sizeof(char) * s.length());
  
	// we start the clock
	dtime = dclock();

	for(int i = 0; i < REPEATS; i++) {
		process_large_string(s.data(), results, s.length());
    	result = results;
	}
	
	// calculate the time taken
	dtime = dclock() - dtime;
  
	std::cout << result << "\n";  
	std::cout << "Time: " << dtime << "\n";
	fflush(stdout);
	free(results);

	return iret;
}
