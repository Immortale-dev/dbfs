#include "dbfs.hpp"

namespace DBFS{
	
	std::mt19937 mt_rand(std::chrono::high_resolution_clock::now().time_since_epoch().count());
	
	string root = ".";
	string suffix = "";
	string prefix = "";
	int filelength = 10;
	std::mutex mtx;
	std::mutex mtx_r;
	bool suffix_minutex = true;
	
	std::mutex mtxs[36*36];
}

DBFS::File::File()
{
	// ctor
}

DBFS::File::~File()
{
	close();
}

DBFS::File::File(string filename)
{
	open(filename);
}

DBFS::File::File(string filename, file_hook_fn onopen, file_hook_fn onclose)
{
	on_open(onopen);
	on_close(onclose);
	open(filename);
}

bool DBFS::File::open()
{
	if(is_open() && fail()){
		#ifdef DEBUG
		SHOW_ERROR;
		SHOW_FILENAME;
		#endif
		return true;
	}
	
	int trys = 5;
	int try_ms = 1;
	while(trys--){
		st = create_stream(filename);
		if(!fail()){
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(try_ms));
		try_ms *= 10;
	}
	p_updated = g_updated = false;
	
	#ifdef DEBUG
	if(fail()){ 
		SHOW_ERROR;
		SHOW_FILENAME;
	}
	#endif
	
	for(auto& it : on_open_fns){
		it(this);
	}
	
	return opened = !fail();
}

bool DBFS::File::open(string filename)
{
	if(is_open() && !fail())
		close();
	this->filename = filename;
	return open();
}

void DBFS::File::seekp(pos_t p)
{
	st.seekp(p);
	pos_p = p;
	p_updated = true;
}

void DBFS::File::seekg(pos_t p)
{
	if(g_updated && p == pos_g){
		return;
	}
	if(g_updated && p > pos_g && pos_g + st.rdbuf()->in_avail() >= p){
		st.ignore(p-pos_g);
		pos_g = p;
		return;
	}
	g_updated = true;
	pos_g = p;
	st.seekg(p);
}

DBFS::pos_t DBFS::File::tellp()
{
	if(p_updated){
		return pos_p;
	}
	p_updated = true;
	return pos_p = st.tellp();
}

DBFS::pos_t DBFS::File::tellg()
{
	if(g_updated){
		return pos_g;
	}
	g_updated = true;
	return pos_g = st.tellg();
}

DBFS::fstream& DBFS::File::stream()
{
	return st;
}

void DBFS::File::read(char* val, pos_t size)
{
	#ifdef DEBUG
	if(!is_open()){
		SHOW_ERROR;
		SHOW_FILENAME;
	}
	if(fail()){
		SHOW_ERROR;
		SHOW_FILENAME;
		assert(false);
	}
	#endif
	st.read(val, size);
	pos_g += size;
	#ifdef DEBUG
	if(fail()){
		SHOW_ERROR;
		SHOW_FILENAME;
		assert(false);
	}
	#endif
}

void DBFS::File::write(char* val, pos_t size)
{
	#ifdef DEBUG
	if(fail()){
		SHOW_ERROR;
		SHOW_FILENAME;
		assert(false);
	}
	#endif
	st.write(val, size);
	pos_p += size;
	#ifdef DEBUG
	if(fail()){
		SHOW_ERROR;
		SHOW_FILENAME;
		assert(false);
	}
	#endif
}

DBFS::pos_t DBFS::File::size()
{
	st.seekp(0, st.end);
	pos_p = st.tellp();
	p_updated = true;
	return pos_p;
}

bool DBFS::File::is_open()
{
	return opened;
}

bool DBFS::File::fail()
{
	return st.fail();
}

void DBFS::File::close()
{
	if(!opened)
		return;
	opened = false;
	st.close();
	for(auto& it : on_close_fns){
		it(this);
	}
}

DBFS::string DBFS::File::name()
{
	return filename;
}

bool DBFS::File::move(string newname)
{
	close();
	bool r = DBFS::move(filename, newname);
	if(!r){
		#ifdef DEBUG
		SHOW_ERROR;
		SHOW_FILENAME;
		#endif
		open();
		return false;
	}
	filename = newname;
	open();
	return r;
}

bool DBFS::File::remove()
{
	close();
	bool r = DBFS::remove(filename);
	if(!r){
		open();
		return false;
	}
	filename = "";
	return r;
}

std::mutex& DBFS::File::get_mutex()
{
	return rmtx; 
}

std::lock_guard<std::mutex> DBFS::File::get_lock()
{
	return std::lock_guard<std::mutex>(get_mutex());
}

void DBFS::File::on_close(file_hook_fn fn)
{
	on_close_fns.push_back(fn);
}

void DBFS::File::on_open(file_hook_fn fn)
{
	on_open_fns.push_back(fn);
}

DBFS::fstream DBFS::File::create_stream(string filename)
{
	string filepath = DBFS::get_file_path(filename);
	std::lock_guard<std::mutex> lock(mtx);
	if(!exists(filename)){
		details::create_path(filepath);
		std::ofstream f(filepath);
	}
	return fstream(filepath, std::fstream::binary | std::fstream::in | std::fstream::out);
}

DBFS::string DBFS::get_file_path(string filename)
{
	string f1 = filename.substr(0,2);
	string f2 = filename.substr(2,2);
	return root + "/" + f1 + "/" + f2 + "/" + prefix + filename + suffix;
}

void DBFS::details::create_path(string filepath)
{
	string curr = "";
	int filepath_size = filepath.size();
	for(int i=0;i<filepath_size;i++){
		if(filepath[i] == '/' && i > 0){
			if(curr == "" || curr == "." || curr == ".."){
				curr.push_back(filepath[i]);
				continue;
			}
			DBFS::details::mkdir(curr);
		}
		curr.push_back(filepath[i]);
	}
}

void DBFS::details::remove_path(string path)
{
	char c = '\0';
	while(path != root){
		if(c == '/'){
			DBFS::details::rmdir(path.c_str());
		}
		c = path.back();
		path.pop_back();
	}
}

int DBFS::details::mkdir(string path)
{
	int err = 0;
	#if defined(_WIN32)
		err = ::_mkdir(path.c_str()); // can be used on Windows
	#else 
		mode_t mode = 0733; // UNIX style permissions
		err = ::mkdir(path.c_str(),mode); // can be used on non-Windows
	#endif
	return err;
}

int DBFS::details::rmdir(string path)
{
	int err = 0;
	#if defined(_WIN32)
		err = ::_rmdir(path.c_str()); // can be used on Windows
	#else 
		err = ::rmdir(path.c_str()); // can be used on non-Windows
	#endif
	return err;
}

bool DBFS::exists(string filename)
{
	if(FILE *file = fopen(get_file_path(filename).c_str(), "r")){
		fclose(file);
		return true;
	}
	return false;
}

bool DBFS::move(string oldname, string newname)
{
	mtx.lock();
	DBFS::details::create_path(get_file_path(newname));
	int r = std::rename(get_file_path(oldname).c_str(), get_file_path(newname).c_str());
	#ifdef DEBUG
	if(r != 0){
		SHOW_ERROR;
	}
	#endif
	DBFS::details::remove_path(get_file_path(oldname));
	mtx.unlock();
	
	return !r;
}

bool DBFS::remove(string filename, bool rem_path)
{
	string path = get_file_path(filename);
	mtx.lock();
	int r = std::remove(path.c_str());
	
	#ifdef DEBUG
	if(r != 0){
		// Do not show error as it is possible and legal scenario
		//SHOW_ERROR; 
	}
	#endif
	
	if(!rem_path){
		mtx.unlock();
		return !r;
	}
		
	DBFS::details::remove_path(path);
	mtx.unlock();
	
	return !r;
}

DBFS::File* DBFS::create()
{
	string filename;
	do{
		filename = DBFS::random_filename();
	}while(DBFS::exists(filename));
	return create(filename);
}

DBFS::File* DBFS::create(file_hook_fn onopen, file_hook_fn onclose)
{
	string filename;
	do{
		filename = DBFS::random_filename();
	}while(DBFS::exists(filename));
	return new File(filename, onopen, onclose);
}

DBFS::File* DBFS::create(string filename)
{
	return new File(filename);
}

DBFS::string DBFS::random_filename()
{
	mtx_r.lock();
	string ret = "";
	for(int i=0;i<filelength;i++){
		int rnd = mt_rand() % 36;
		char c = rnd < 10 ? rnd+'0' : rnd-10+'a';
		ret.push_back(c);
	}
	
	if(suffix_minutex){
		unsigned int ctime = time(NULL)/60;
		while(ctime > 0){
			int rnd = ctime % 36;
			ctime /= 36;
			char c = rnd < 10 ? rnd+'0' : rnd-10+'a';
			ret.push_back(c);
		}
	}
	
	mtx_r.unlock();
	return ret;
}

void DBFS::set_root(string path)
{
	root = path;
}

void DBFS::set_prefix(string prefix)
{
	prefix = prefix;
}

void DBFS::set_suffix(string suffix)
{
	suffix = suffix;
}

void DBFS::set_filename_length(int length)
{
	filelength = length;
}

void DBFS::use_suffix_minutes(bool use)
{
	suffix_minutex = use;
}
