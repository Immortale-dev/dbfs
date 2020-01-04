#ifndef DBFS_H
#define DBFS_H

#ifdef DEBUG
	#include <errno.h>
	#include <iostream>
#endif

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <string>
#include <fstream>
#include <algorithm>
#include <random>
#include <chrono>

#ifdef _WIN32
	#include <direct.h>
#else
	#include <sys/stat.h>
#endif

namespace DBFS{
	
		
	using string = std::string;
	using pos_t = long int;
	using fstream = std::fstream;

	extern std::mt19937 mt_rand;
	extern string root;
	extern string suffix;
	extern string prefix;
	extern pos_t ch_size;
	extern int filelength;
	extern int max_try;
	extern bool clear_folders;
	
	class File{
		public:
			File();
			File(string filename);
			virtual ~File();
			
			template<typename T>
			pos_t write(T val);
			
			template<typename T>
			void read(T& val);
			
			pos_t write(char* val, pos_t size);
			void read(char* val, pos_t size);
			pos_t write(fstream& val, pos_t size);
			
			bool open();
			bool open(string filename);
			void close();
			void seek(pos_t p);
			pos_t tell();
			pos_t size();
			
			string name();
			
			bool move(string newname);
			bool remove();
			
			bool is_open();
			bool fail();
			fstream& stream();
			
		private:
			pos_t pos = 0;
			fstream st;
			bool opened = false;
			string filename = "";
			
			void create_path(string filename);
			fstream create_stream(string filename);
	};
	
	string get_file_path(string filename);
	string random_filename();
	File* create();
	File* create(string filename);
	bool move(string oldname, string newname);
	bool remove(string filename, bool remove_path = true);
	bool exists(string filename);
	
	int mkdir(string path);
	int rmdir(string path);
}



template<typename T>
void DBFS::File::read(T& val)
{
	st.seekg(pos);
	st >> val;
	pos = st.tellg();
}

template<typename T>
DBFS::pos_t DBFS::File::write(T val)
{
	st.seekp(pos);
	st << val;
	return pos = st.tellp();
}



#endif // DBFS_H
