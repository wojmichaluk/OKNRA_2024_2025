#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <string>
#include <iostream>
#include <cctype>
#include <algorithm>
#include <vector>
#include <sstream>

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

// helper function to detect ASCII characters out of given codes scope
bool nonprintable(char & ch) {
	return ch < 0x20 || ch > 0x7E;
}

// transformations regards to task contents
char transform_character(char & ch) {
	if(isspace(ch)) {
		return ' ';
	} else if(ispunct(ch)) {
		return ',';
	}

	return tolower(ch);
}

// helper function to split string, based on: 
// https://medium.com/@ryan_forrester_/splitting-strings-in-c-a-complete-guide-cf162837f4ba
void split_string(std::vector<std::string> & words, const std::string & str) {
    std::istringstream iss(str);
    std::string word;
    
    while (iss >> word)
        words.emplace_back(word);
}

// helper function so that DRY rule is respected
void fill_between_words(const std::string & s, std::string & result, int pos1, int pos2) {
	bool flag = true;

	for(pos1; pos1 < pos2; pos1++) {
		if(isspace(s[pos1]) && flag) {
			result += " "; // 2nd optimization
			flag = false;
		} else if(ispunct(s[pos1])) {
			result += ","; // 2nd optimization
			flag = true;
		}
	}
}

// function to be optimized
std::string process_large_string(std::string s) {
    std::string result, copy;
	std::vector<std::string> split;
	int old_pos = 0, new_pos;

	// first optimization
	result.reserve(s.length());

	// third optimization - std library algorithms
	std::transform(s.begin(), s.end(), s.begin(), transform_character);
	auto end = std::remove_if(s.begin(), s.end(), nonprintable);
	s.erase(end, s.end());

	// preparing & saving a copy at this stage
	copy.resize(s.length());
	std::copy(s.begin(), s.end(), copy.begin());

	// trick to help splitting words
	std::transform(
		copy.begin(), copy.end(), copy.begin(), [](char ch){ if(ch == ',') return ' '; return ch; });

	// split string into tokens (words separated by spaces)
	split_string(split, copy);

	// eliminating word duplicates
  	auto words_end = std::unique(split.begin(), split.end());

	// joining words back together, including "lost" commas and single whitespaces
	for(auto it = split.begin(); it != words_end; ) {
		new_pos = s.find(*it, old_pos);
		fill_between_words(s, result, old_pos, new_pos);
		old_pos = new_pos + (*it).length();
		result += *it++; // 2nd optimization
	}

	// commas or spaces at the end (after last word)
	fill_between_words(s, result, old_pos, s.length());

    return result;
}

int main(int argc, const char* argv[]) {
	int i, j, k, iret;
	double dtime;
	std::string s, result, line;
  
	std::cout << "Let's start processing the file\n";
  
	while(getline(std::cin, line))
    	s += line + "\n";
  
	// we start the clock
	dtime = dclock();

	for(int i = 0; i < REPEATS; i++)
    	result = process_large_string(s);
	
	// calculate the time taken
	dtime = dclock() - dtime;
  
	std::cout << result << "\n";  
	std::cout << "Time: " << dtime << "\n";
	fflush(stdout);

	return iret;
}
