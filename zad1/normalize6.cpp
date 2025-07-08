#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <string>
#include <iostream>
#include <cctype>
#include <cstring>
#include <cmath>
#include <omp.h>

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

// fifth optimization - function that can be run concurrently
void transform_commas_and_whitespaces(const char* s, char* result, size_t size) {
	// to use in memcpy
	char* space = (char*)" ";
	char* comma = (char*)",";

	for(int i = 0; i < size; i++) {
		if(isspace(s[i])) {
			memcpy(result, space, 1); // 4th optimization
			result += 1;
		} else if(ispunct(s[i])) {
			memcpy(result, comma, 1); // 4th optimization
			result += 1;
		} else if(s[i] >= 0x20 && s[i] <= 0x7E) {
			memcpy(result, s+i, 1); // 4th optimization
			result += 1;
		}
	}

	*result = 0;
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
		if(s[i] == ' ') {
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
		} else if(s[i] == ',') {
			if(current.length() && current != last) {
				memcpy(result, current.data(), current.length()); // 4th optimization
				result += current.length();
				last = current;
			}

			memcpy(result, comma, 1); // 4th optimization
			result += 1;
			current.clear();
			flag = true;
		} else {
			current += (char)tolower(s[i]); // 2nd optimization
			flag = true;
		}
    }

    *result = 0;
}

int main(int argc, const char* argv[]) {
	int i, j, k, iret, num_threads, part_len;
	double dtime;
	std::string s, result, line, entry;
  
	std::cout << "Let's start processing the file\n";
  
	while(getline(std::cin, line))
    	s += line + "\n";

	// prepare for dividing data
	#pragma omp parallel
	{
		#pragma omp single
		num_threads = omp_get_num_threads();
	}

	part_len = s.length() / num_threads;
	char **results = (char**)malloc(sizeof(char*) * num_threads);
	char* results_ = (char*)malloc(sizeof(char) * s.length());

	for(int i = 0; i < num_threads; i++)
		results[i] = results_ + i * part_len;

	// we start the clock
	dtime = dclock();

	for(int i = 0; i < REPEATS; i++) {
		#pragma omp parallel 
		{
			int id = omp_get_thread_num();

			if(id < num_threads - 1) {
				transform_commas_and_whitespaces(s.data() + id * part_len, results[id], part_len);
			} else {
				transform_commas_and_whitespaces(
					s.data() + id * part_len, results[id], s.length() - (num_threads - 1) * part_len);
			}
		}

		for(int j = 0; j < num_threads; j++)
			entry += results[j];

		process_large_string(entry.data(), results_, entry.length());
    	result = results_;
		entry.clear();
	}
	
	// calculate the time taken
	dtime = dclock() - dtime;
  
	std::cout << result << "\n";  
	std::cout << "Time: " << dtime << "\n";
	fflush(stdout);
	free(results);
	free(results_);

	return iret;
}
