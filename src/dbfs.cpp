#include "dbfs.hpp"


namespace DBFS{
	
	std::mt19937 mt_rand(std::chrono::high_resolution_clock::now().time_since_epoch().count());
	
	string root = ".";
	string suffix = "";
	string prefix = "";
	pos_t ch_size = 4*1024;
	int filelength = 16;
	int max_try = 25;
	bool clear_folders = true;
	
}



DBFS::File::File()
{
	
}

DBFS::File::~File()
{
	
}

DBFS::File::File(string filename)
{
	open(filename);
}

bool DBFS::File::open()
{
	if(is_open() && fail())
		return true;
	st = create_stream(filename);
	return opened = !fail();
}

bool DBFS::File::open(string filename)
{
	if(is_open() && !fail())
		close();
	this->filename = filename;
	return open();
}

void DBFS::File::seek(pos_t p)
{
	pos = p;
}

DBFS::pos_t DBFS::File::tell()
{
	return pos;
}

DBFS::fstream& DBFS::File::stream()
{
	return st;
}


void DBFS::File::read(char* val, pos_t size)
{
	st.seekg(pos);
	st.read(val, size);
	pos = st.tellg();
}


DBFS::pos_t DBFS::File::write(char* val, pos_t size)
{
	st.seekp(pos);
	st.write(val, size);
	return pos = st.tellp();
}

DBFS::pos_t DBFS::File::write(fstream& val, pos_t size)
{
	st.seekp(pos);
	char* buf = new char[ch_size];
	while(size){
		pos_t sz = std::min(ch_size, size);
		val.read(buf, sz);
		st.write(buf, sz);
		size -= sz;
	}
	return pos = st.tellp();
}

DBFS::pos_t DBFS::File::size()
{
	st.seekg(0, st.end);
	pos_t p = st.tellg();
	return --p;
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
	opened = false;
	st.close();
}

DBFS::string DBFS::File::name()
{
	return filename;
}

bool DBFS::File::move(string newname)
{
	close();
	create_path(get_file_path(newname));
	bool r = DBFS::move(filename, newname);
	if(!r){
		open();
		return false;
	}
	DBFS::remove(filename);
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


DBFS::fstream DBFS::File::create_stream(string filename)
{
	string filepath = DBFS::get_file_path(filename);
	if(!exists(filename)){
		create_path(filepath);
		std::ofstream f(filepath);
	}
	return fstream(filepath);
}

DBFS::string DBFS::get_file_path(string filename)
{
	string f1 = filename.substr(0,2);
	string f2 = filename.substr(2,2);
	return root + "/" + f1 + "/" + f2 + "/" + prefix + filename + suffix;
}

void DBFS::File::create_path(string filepath)
{
	string curr = "";
	int filepath_size = filepath.size();
	for(int i=0;i<filepath_size;i++){
		if(filepath[i] == '/' && i > 0){
			if(curr == "" || curr == "." || curr == "..")
				continue;
			DBFS::mkdir(curr);
		}
		curr.push_back(filepath[i]);
	}
}

int DBFS::mkdir(string path)
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

int DBFS::rmdir(string path)
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
	int r = std::rename(get_file_path(oldname).c_str(), get_file_path(newname).c_str());
	
	#ifdef DEBUG
	if(r == -1){
		std::cout << "Error: " << strerror(errno) << std::endl;
	}
	#endif
	
	return !r;
}

bool DBFS::remove(string filename)
{
	string path = get_file_path(filename);
	int r = std::remove(path.c_str());
	char c = '\0';
	
	#ifdef DEBUG
	if(r == -1){
		std::cout << "Error: " << strerror(errno) << std::endl;
	}
	#endif
	
	if(!clear_folders)
		return !r;
		
	while(path != root){
		if(c == '/'){
			DBFS::rmdir(path.c_str());
		}
		c = path.back();
		path.pop_back();
	}
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

DBFS::File* DBFS::create(string filename)
{
	return new File(filename);
}

DBFS::string DBFS::random_filename()
{
	string ret = "";
	for(int i=0;i<filelength;i++){
		int rnd = mt_rand() % 36;
		char c = rnd < 10 ? rnd+'0' : rnd-10+'a';
		ret.push_back(c);
	}
	return ret;
}
