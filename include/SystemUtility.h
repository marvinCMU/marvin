/* 
 * Project: Multi Modal First Person Sensing (MMFPS)
 * Author: Qifan Hu (qhu@andrew.cmu.edu)
 * Affiliation: Carnegie Mellon University, ISTC-EC
 * Date: 11/5/2012
 */

#ifndef SYSTEMUTILITY_H_
#define SYSTEMUTILITY_H_

#include <string>

// replace specific char to specific new string
std::string replaceChar (std::string str, char c, std::string newstr) {
	std::string tmp = str;
	size_t index = tmp.find(c, 0);
	while (index != std::string::npos) {
		tmp.replace(index, 1, newstr);
		index = tmp.find(c, ++index);
	}
	return tmp;
}

// replace specific substring to specific new string
std::string replaceStr (std::string str, std::string s, std::string newstr) {
	std::string tmp = str;
	size_t index = tmp.find(s, 0);
	while (index != std::string::npos) {
		tmp.replace(index, s.length(), newstr);
		index = tmp.find(s, index);
	}
	return tmp;
}

// change string case; -1: lower case, 0: normal case; 1: upper case
// change only the case of the first letter
void changeWordCase(std::string &str, int n){
	const int length = str.length();
	if (n==-1)
		for(int i=0; i < length; ++i)
			str[i] = std::tolower(str[i]);
	else if (n==0)
		str[0] = std::toupper(str[0]);
	else if (n==1)
		for(int i=0; i < length; ++i)
			str[i] = std::toupper(str[i]);
}

// parse sentense string and change case to normal case
// e.g "hi, im XXX" to "Hi, Im Xxx"
void changeStrCase(std::string &str, int n){
	std::string cmd;
	std::string obj;
	size_t tokenPos = str.find(" ");
	cmd = str.substr(0, tokenPos);
	str.erase(0, tokenPos+1);
	obj = str;
	changeWordCase (cmd, n);
	changeWordCase (obj, n);
	if (cmd.compare("Information") == 0) {
		cmd = "Info";
	}
	str = cmd + " " + obj;
}

#endif
