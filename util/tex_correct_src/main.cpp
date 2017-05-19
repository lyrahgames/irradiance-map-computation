#include <iostream>
#include <fstream>
#include <string>

int main(int argc, char *argv[]){
	using namespace std;

	string file_path;
	string file_data;

	if (argc < 2){
		cout << "usage: ./tex_correct [input file] [output file]\n";
		return -1;
	}else{
		file_path = argv[1];
	}

	fstream fs;
	fs.open(file_path, ios::in);

	string first_line;
	getline(fs, first_line);

	file_data += "\\documentclass{standalone}\n";

	uint dollar_count = 0;
	char tmp = fs.get();

	while(fs){
		if (tmp == '$'){
			dollar_count++;
		}else{
			dollar_count = 0;
		}

		if (dollar_count < 2)
			file_data += tmp;

		tmp = fs.get();
	}


	fs.close();
	fs.open(file_path, ios::out | ios::trunc);

	fs << file_data;

	fs.close();

	// cout << file_data << endl;

	return 0;
}