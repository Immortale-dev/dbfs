#ifndef DBFS_H
#define DBFS_H

#define DEBUG

#ifdef DEBUG
	#include <errno.h>
	#include <iostream>
	#define SHOW_ERROR std::cout<<"Error on L"<<__LINE__<<": "<<std::strerror(errno)<<std::endl
	#define SHOW_FILENAME std::cout<<"Filename: "<<name()<<std::endl
#endif

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <string>
#include <fstream>
#include <algorithm>
#include <random>
#include <chrono>
#include <mutex>
#include <cassert>
#include <functional>
#include <thread>
#include <list>

#ifdef _WIN32
	#include <direct.h>
#else
	#include <sys/stat.h>
	#include <unistd.h>
#endif

namespace DBFS{
	
	class File;
		
	using string = std::string;
	using pos_t = long int;
	using fstream = std::fstream;
	using file_hook_fn = std::function<void(File*)>;
	
	extern int filelength;
	
	class File{
		public:
			File();
			File(string filename);
			File(string filename, file_hook_fn onopen, file_hook_fn onclose);
			virtual ~File();
			
			template<typename T>
			void write(T val);
			
			template<typename T>
			void read(T& val);
			
			void write(char* val, pos_t size);
			void read(char* val, pos_t size);
			
			bool open();
			bool open(string filename);
			void close();
			
			void seekp(pos_t p);
			void seekg(pos_t p);
			
			pos_t tellp();
			pos_t tellg();
			
			pos_t size();
			
			string name();
			
			bool move(string newname);
			bool remove();
			
			bool is_open();
			bool fail();
			fstream& stream();
			
			void on_close(file_hook_fn fn);
			void on_open(file_hook_fn fn);
			
			std::mutex& get_mutex();
			std::lock_guard<std::mutex> get_lock();
			
		private:
			pos_t pos_p = 0, pos_g = 0;
			bool p_updated = false, g_updated = false;
			fstream st;
			bool opened = false;
			string filename = "";
			std::mutex mtx, rmtx;
			std::list<file_hook_fn> on_close_fns, on_open_fns;
			
			fstream create_stream(string filename);
	};
	
	string get_file_path(string filename);
	string random_filename();
	File* create();
	File* create(string filename);
	File* create(file_hook_fn onopen, file_hook_fn onclose);
	bool move(string oldname, string newname);
	bool remove(string filename, bool remove_path = true);
	bool exists(string filename);

	void set_root(string path);
	void set_prefix(string prefix);
	void set_suffix(string suffix);
	void set_filename_length(int length);
	void use_suffix_minutes(bool use);
	
	namespace details{	
		void create_path(string filename);
		void remove_path(string filepath);
		int mkdir(string path);
		int rmdir(string path);
	}
}



template<typename T>
void DBFS::File::read(T& val)
{
	#ifdef DEBUG
	if(st.fail()){
		SHOW_ERROR;
		SHOW_FILENAME;
	}
	#endif
	st >> val;
	#ifdef DEBUG
	if(st.fail()){
		SHOW_ERROR;
		SHOW_FILENAME;
	}
	#endif
	pos_g += st.gcount();
	g_updated = false;
}

template<typename T>
void DBFS::File::write(T val)
{
	st << val;
	#ifdef DEBUG
	if(st.fail()){
		SHOW_ERROR;
		SHOW_FILENAME;
	}
	#endif
	p_updated = false;
}

#endif // DBFS_H
