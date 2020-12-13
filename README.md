# Database Filesystem

**Threading safe** library to simplify managing files for some specific database engines. This library provides relaxing files between the folders out of the box.

## Table of Context
* [Title](#database-filesystem)
* [Build](#build)
* [Docs](#docs)
	* [Usage](#usage)
	* [Public methods of DBFS namespace](#public-methods-of-dbfs-namespace)
		* [void DBFS::set_root(string path)](#void-dbfsset_rootstring-path)
		* [void DBFS::set_prefix(string prefix)](#void-dbfsset_prefixstring-prefix)
		* [void DBFS::set_suffix(string suffix)](#void-dbfsset_suffixstring-suffix)
		* [void DBFS::set_filename_length(int length)](#void-dbfsset_filename_lengthint-length)
		* [void DBFS::use_suffix_minutes(bool use)](#void-dbfsuse_suffix_minutesbool-use)
		* [std::string DBFS::random_filename()](#stdstring-dbfsrandom_filename)
		* [DBFS::File* DBFS::create()](#dbfsfile-dbfscreate)
		* [DBFS::File* DBFS::create(std::string name)](#dbfsfile-dbfscreatestdstring-name)
		* [DBFS::File* DBFS::create(DBFS::file_hook_fn on_open, DBFS::file_hook_fn of_close)](#dbfsfile-dbfscreatedbfsfile_hook_fn-on_open-dbfsfile_hook_fn-of_close)
		* [bool DBFS::move(std::string name, std::string new_name)](#bool-dbfsmovestdstring-name-stdstring-new_name)
		* [bool DBFS::remove(std::string name, bool remove_path)](#bool-dbfsremovestdstring-name-bool-remove_path)
		* [bool DBFS::exists(std::string name)](#bool-dbfsexistsstdstring-name)
	* [public methods of `DBFS::File` class](#public-methods-of-dbfsfile-class)
		* [DBFS::File()](#dbfsfile)
		* [DBFS::File(string name)](#dbfsfilestring-name)
		* [DBFS::File(DBFS::file_hook_fn on_open, DBFS::file_hook_fn on_close)](#dbfsfiledbfsfile_hook_fn-on_open-dbfsfile_hook_fn-on_close)
		* [std::string DBFS::File::name()](#stdstring-dbfsfilename)
		* [size_t DBFS::File::size()](#size_t-dbfsfilesize)
		* [bool DBFS::File::open()](#bool-dbfsfileopen)
		* [bool DBFS::File::open(std::string name)](#bool-dbfsfileopenstdstring-name)
		* [void DBFS::File::close()](#void-dbfsfileclose)
		* [bool DBFS::File::is_open()](#bool-dbfsfileis_open)
		* [bool DBFS::File::fail()](#bool-dbfsfilefail)
		* [bool DBFS::File::move(std::string new_name)](#bool-dbfsfilemovestdstring-new_name)
		* [bool DBFS::File::remove()](#bool-dbfsfileremove)
		* [template\<typename T\> void DBFS::File::read(T&amp; val)](#templatetypename-t-void-dbfsfilereadt-val)
		* [void DBFS::File::read(char* pos, size_t size)](#void-dbfsfilereadchar-pos-size_t-size)
		* [template\<typename T\> void DBFS::File::write(T val)](#templatetypename-t-void-dbfsfilewritet-val)
		* [void DBFS::File::write(char* pos, size_t size)](#void-dbfsfilewritechar-pos-size_t-size)
		* [void DBFS::File::seekg(size_t pos)](#void-dbfsfileseekgsize_t-pos)
		* [void DBFS::File::seekp(size_t pos)](#void-dbfsfileseekpsize_t-pos)
		* [size_t DBFS::File::tellg()](#size_t-dbfsfiletellg)
		* [size_t DBFS::File::tellp()](#size_t-dbfsfiletellp)
		* [void DBFS::File::on_open(DBFS::file_hook_fn on_open)](#void-dbfsfileon_opendbfsfile_hook_fn-on_open)
		* [void DBFS::File::on_close(DBFS::file_hook_fn on_close)](#void-dbfsfileon_closedbfsfile_hook_fn-on_close)
		* [std::fstream&amp; DBFS::File::stream()](#stdfstream-dbfsfilestream)
		* [std::mutex&amp; DBFS::File::get_mutex()](#stdmutex-dbfsfileget_mutex)
		* [std::lock_guard\<std::mutex\> get_lock()](#stdlock_guardstdmutex-get_lock)
* [License](#license)


## Build
Library was tested using **GNU G++** compiler with flag **-std=c++17**. So it is recommended to use C++ 17 or higher version of compiler. Compiling with another compilers might need code corrections.

## Docs

### Usage
First of all you have to define your files root folder. You can do it by calling the `DBFS::set_root(string)` method with path to the root folder as single argument. You can also set up preffix, suffix and other options. You can find all available options below. 

Then you can start using methods to create and manipulate files.

***Example:*** _Create file, write the data, and delete this file after it is closed._
```c++
DBFS::set_root("./tmp");
DBFS::File* file = DBFS::create_file();
file->on_close([](DBFS::File* f){
	f->remove();
});
file->write("Hello World!");
```

### Public methods of `DBFS` namespace
#### void DBFS::set_root(string path)
Methods for setting up the root folder for all files. By default `"."`

***Example:***
```c++
DBFS::set_root("./tmp");
```

#### void DBFS::set_prefix(string prefix)
Method for setting up the prefix for your files. This prefix used exclusively for storing files on OS filesystem, and not included to the filename returned by `string DBFS::File::name()` method. By default `""`

***Example:***
```c++
DBFS::set_prefix("f_");
auto f = new DBFS::File("somefilename"); // created as ${root_folder}/so/me/f_somefilename
std::cout << f->name() << std::endl; // somefilename
```

#### void DBFS::set_suffix(string suffix)
Method for setting up the suffix for your files. The rules the same as for prefix. You can use it for setting up file extension. By default `""`

***Example:***
```c++
DBFS::set_suffix(".txt");
auto f = new DBFS::File("somefilename"); // created as ${root_folder}/so/me/somefilename.txt
std::cout << f->name() << std::endl; // somefilename
```

#### void DBFS::set_filename_length(int length)
Method for setting up filename lengths for automatically created files _(not including prefix or suffix)_. By default `10`

***Example:***
```c++
DBFS::set_filename_length(8);
auto f = DBFS::create();
std::cout << f->name() << std::endl; // something like "a5ftc5g4"
```

#### void DBFS::use_suffix_minutes(bool use);
`true` by default. If active it will add ~ 5 symbols at the end of filename corresponding to the current time in minutes.

**Note:** _We strongly recommend leaving this feature on to avoid any filename collisions_

### std::string DBFS::get_file_path(string name);
Accepts one string parameter - name of the file and returns full path to the file including `prefix`, and `suffix`.

***Example:***
```c++
auto f = DBFS::create();
std::cout << DBFS::get_file_path(f->name()) << std::endl; // something like ${root}/a7/bc/a7bcfsa7wfhgawq8asm
```

#### std::string DBFS::random_filename();
Returns filename you can use to create new file with `DBFS::create(name)` construction.

***Example:***
```c++
DBFS::create(DBFS::random_filename());
```

#### DBFS::File* DBFS::create();
Creates and opens new file and returns its pointer. Uses `DBFS::random_filename()` to generate filename.

#### DBFS::File* DBFS::create(std::string name);
Creates or opens file with filename `name`.

#### DBFS::File* DBFS::create(DBFS::file_hook_fn on_open, DBFS::file_hook_fn of_close)
Creates and opens new file and set `on_open` and `on_close` hooks. 
**Note:** _`DBFS::file_hook_fn` is alias for `std::function<void(DBFS::File*)>`_

#### bool DBFS::move(std::string name, std::string new_name);
Moves file to new direction. 
**Note:** _File associated with `name` you want to move should be closed before moving_

#### bool DBFS::remove(std::string name, bool remove_path);
Deletes file with name `name`. if `remove_path` is set to `true` _(`true` by default)_ then if folders are empty, it will remove the folders as well.

#### bool DBFS::exists(std::string name);
Checks whenever file with `name` exists or not

### public methods of `DBFS::File` class
#### DBFS::File()
Default constructor. Creates instance of `DBFS::File` with no associated filename.
You can use `DBFS::File::open(std::string)` method to open specific file.

***Example:***
```c++
auto f = new DBFS::File();
f->open("jrhg31hj5h12bf");
```

#### DBFS::File(string name)
Creates or opens file with specific name. 
**Note:** It is recommended to use `new` keyword when creating or opening files with `DBFS::File` constructors.

#### DBFS::File(DBFS::file_hook_fn on_open, DBFS::file_hook_fn on_close)
Constructor. Match `DBFS::create(DBFS::file_hook_fn on_open, DBFS::file_hook_fn on_close)`. For more details, see corresponding [method](#dbfsfile-dbfscreatedbfsfile_hook_fn-on_open-dbfsfile_hook_fn-of_close).

#### std::string DBFS::File::name()
Returns name of associated with current instance file.

#### size_t DBFS::File::size()
Returns size of the associated associated with current instance file.

#### bool DBFS::File::open()
Opens associated with current instance file. Returns `true` if file was opened successfully.

#### bool DBFS::File::open(std::string name);
Opens file associated with `name`. Returns `true` if file was opened successfully.

#### void DBFS::File::close()
Closes associated with current instance file.

#### bool DBFS::File::is_open()
returns true if file is opened

#### bool DBFS::File::fail()
Returns true if corresponding to current instant `std::fstream` is in error stated.

#### bool DBFS::File::move(std::string new_name)
Moves associated with current instance file to move location. If current file is opened, it will be closed, moved and then reopened again. If file is closed, it will be just moved.

#### bool DBFS::File::remove()
Deletes associated with current instance file. If file is opened, it will be closed and then deleted.

#### template\<typename T\> void DBFS::File::read(T& val)
Reads file content to corresponding variable `val`. Methods works similar to `stringstream operator>>`

***Example:***
```c++
auto f = DBFS::create();
f->write("18 24\n");
int a1,a2;
f->read(a1);
f->read(a2);
std::cout << a1 + a2 << std::endl; // 42
```

#### void DBFS::File::read(char* pos, size_t size)
Reads `size` bytes from file and put them to char buffer starting at `pos`

***Example:***
```c++
auto f = DBFS::create();
f->write("123abc");
char* buf = new char[4];
f->read(buf,3);
buf[3] = '\0';
std::cout << buf << std::endl; // 123
```

#### template\<typename T\> void DBFS::File::write(T val)
Writes content of corresponding variable to file. Method works similar to `stringstream operator<<`.

#### void DBFS::File::write(char* pos, size_t size)
Writes `size` bytes to the file from buffer starting at position `pos`

#### void DBFS::File::seekg(size_t pos)
Moves read pointer to corresponding position.

***Example:***
```c++
auto f = DBFS::create();
f->write("abc42\n");
f->seekg(3);
int a;
f->read(a);
std::cout << a << std::endl; // 42
```

#### void DBFS::File::seekp(size_t pos)
Moves write pointer to corresponding position.

***Example:***
```c++
auto f = DBFS::create();
f->write("F**k this shit I'm out!");
f->seekp(1);
f->write("uc");
```

#### size_t DBFS::File::tellg()
Returns position of write pointer.

#### size_t DBFS::File::tellp()
Returns position of read pointer.

#### void DBFS::File::on_open(DBFS::file_hook_fn on_open)
Associate function with current instant. Every time file opens this function will run.

**Note:** _There is no way to remove function from hook list yet_

#### void DBFS::File::on_close(DBFS::file_hook_fn on_close)
Associate function with current file. Every time file closes this function will run.

***Example:***
```c++
auto f = DBFS::create();
f->on_close([](DBFS::File* f){
	f->remove();
});
```

#### std::fstream& DBFS::File::stream()
Returns reference to associated with current file `std::fstream`

#### std::mutex& DBFS::File::get_mutex()
Returns reference to associated with current file `std::mutex` so you can block file when reading it from 2 different threads. 

#### std::lock_guard\<std::mutex\> get_lock()
Returns `lock_guard` that uses `mutex` of current file.

***Example:***
```c++
auto f = DBFS::create("somefilename");
char* buf = new char[1024];
size_t sz = f->size();
auto lock = f->get_lock();
f->seekg(0)
while(sz){
	int read_size = std::min(sz,1024);
	f->read(buf,read_size);
	// Do something with buffer
	sz -= read_size;
}
```

## License
MIT

Have fun :)
