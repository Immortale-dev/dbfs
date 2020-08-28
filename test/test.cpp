
#define DEBUG

#include <iostream>
#include <string>
#include <mutex>
#include <thread>
#include <unordered_set>
#include "qtest.hpp"
#include "dbfs.hpp"

using namespace std;

DESCRIBE("DBFS", {
	
	srand(time(NULL));
	DBFS::root = "tmp";
	
	IT("should create file called `abcdef` in ./tmp/ab/cd dir", {
		DBFS::create("abcdef")->close();
		EXPECT(DBFS::exists("abcdef")).toBe(true);
		DBFS::remove("abcdef");
	});
	
	DESCRIBE("Random name file is created", {
		
		DBFS::File* f;
		BEFORE_ALL({
			f = DBFS::create();
		});
		
		AFTER_ALL({
			delete f;
		});
		
		IT("File length should be greater or equal to " + to_string(DBFS::filelength), {
			EXPECT(f->name().size()).toBeGreaterThanOrEqual(DBFS::filelength);
		});
		
		IT("File name should contain only alphabetic letters and numbers", {
			string name = f->name();
			for(int i=0;i<name.size();i++){
				char c = name[i];
				if( (c < '0' || c > '9') && (c < 'a' || c > 'z') )
					TEST_FAILED();
			}
			TEST_SUCCEED();
		});
		
		IT("File should exists", {
			DBFS::exists(f->name());
		});
		
		DESCRIBE("Move file to new location", {
			string oldname = f->name();
			string newname = DBFS::random_filename();
			
			BEFORE_ALL({				
				f->move(newname);
			});
			
			IT("File old location should not exists", {
				EXPECT(DBFS::exists(oldname)).toBe(false);
			});
			
			IT("File new location should exists", {
				EXPECT(DBFS::exists(newname)).toBe(true);
			});
		});
		
		DESCRIBE("Remove file", {
			
			string name;
			
			BEFORE_ALL({
				name = f->name();
				f->remove();
			});
			
			IT("file should be closed", {
				EXPECT(f->is_open()).toBe(false);
			});
			
			IT("File location should not exists", {
				EXPECT(DBFS::exists(name)).toBe(false);
			});
		});
	});
	
	DESCRIBE("File Created", {
		DBFS::File* f;
		
		BEFORE_ALL({
			f = DBFS::create();
		});
		
		AFTER_ALL({
			f->remove();
			delete f;
		});
		
		DESCRIBE("Write 10 numbers to file", {
			BEFORE_ALL({
				for(int i=0;i<10;i++){
					f->write(i);
				}
				f->write('\n');
			});
			
			IT("file size should be equal 11", {
				EXPECT(f->size()).toBe(11);
				INFO_PRINT("File size: " + to_string(f->size()));
			});
			
			DESCRIBE("Seek for 4th bit", {
				BEFORE_ALL({
					f->seekg(4);
				});
				
				IT("current char should be equal `4`", {
					char c;
					f->read(c);
					
					EXPECT(c).toBe('4');
				});
			});
			
			DESCRIBE("Seekp for 2th bit and put 3 numbers", {
				
				BEFORE_ALL({				
					f->seekp(1);
					for(int i=1;i<=3;i++){
						f->write(i);
					}
				});
				
				IT("tellp should be equal to `4`", {
					EXPECT(f->tellp()).toBe(4);
				});
				
				IT("next char should be equal `4", {
					char c;
					f->read(c);
					EXPECT(c).toBe('4');
				});
			});
			
			DESCRIBE("Seek for 6th bit and read int", {
				int a;
				string str;
				BEFORE_ALL({
					f->seekg(6);
					f->read(a);
				});
				
				IT("`a` should be equal to `6789`", {
					EXPECT(a).toBe(6789);
					//INFO_PRINT("Read number: " + str);
				});
			});
			
			DESCRIBE("Seek for 0th bit and read char buffer of size()", {
				char* buf = new char[50];
				BEFORE_ALL({
					int sz = f->size();
					f->seekg(0);
					f->read(buf, --sz);
					buf[sz] = '\0';
				});
				
				IT("buf should be equal to 0123456789", {
					EXPECT(string(buf)).toBe("0123456789");
					INFO_PRINT(string(buf));
				});
				
				IT("File should not be fail", {
					EXPECT(f->fail()).toBe(false);
				});
			});
		});
	});
	
	DESCRIBE("File hooks test", {
		int open_count, close_count;
		open_count = close_count = 0;
		DBFS::File* f;
		
		IT("Create file with on_opne, on_close assigned", {
			
			f = new DBFS::File(DBFS::random_filename(), [&open_count](auto* file){ open_count++; }, [&close_count](auto* file){ close_count++; });
			
			EXPECT(open_count).toBe(1);
			EXPECT(close_count).toBe(0);
		});
		
		IT("Move file to new destination", {
			f->move(DBFS::random_filename());
			
			EXPECT(open_count).toBe(2);
			EXPECT(close_count).toBe(1);
		});
		
		IT("Close file", {
			f->close();
			
			EXPECT(open_count).toBe(2);
			EXPECT(close_count).toBe(2);
			
		});
		
		IT("Reopen file",{
			
			f->open();
			
			EXPECT(open_count).toBe(3);
			EXPECT(close_count).toBe(2);
		});
		
		IT("Assign delete on close and free file memory",{
			
			string name = f->name();
			
			f->on_close([](auto* file){
				DBFS::remove(file->name());
			});
			
			delete f;
			
			EXPECT(open_count).toBe(3);
			EXPECT(close_count).toBe(3);
			EXPECT(!DBFS::exists(name));
		});
	});
	
	DESCRIBE("File::on_close", {
		IT("should delete file after it is closed", {
			DBFS::File* f = DBFS::create();
			string name = f->name();
			bool called = false;
			f->on_close([&called](DBFS::File* file){
				called = true;
			});
			f->on_close([](DBFS::File* file){
				file->remove();
			});
			f->close();
			EXPECT(!DBFS::exists(name));
			EXPECT(called).toBe(true);
		});
	});
	
	DESCRIBE("Multithreading test", {
		std::mutex mtx;
		std::unordered_set<string> files;
		
		DESCRIBE("Proceed with 1000 create/delete operations", {
			IT("Should succeed", {
				vector<thread> v;
				for(int i=0;i<1000;i++){
					std::thread t([&mtx,&files](int ind){
						if(ind == 0){
							DBFS::File* f = DBFS::create();
							string name = f->name();
							f->close();
							if(!DBFS::exists(name)){
								TEST_FAILED();
							}
							mtx.lock();
							files.insert(name);
							mtx.unlock();
						}
						else if(ind == 1){
							mtx.lock();
							if(files.begin() == files.end()){
								mtx.unlock();
								return;
							}
							string name = *(files.begin());
							files.erase(name);
							mtx.unlock();
							string nname = DBFS::random_filename();
							DBFS::move(name,nname);
							if(!DBFS::exists(nname)){
								//cout << "HEREF" << endl;
								TEST_FAILED();
							}
							mtx.lock();
							files.insert(nname);
							mtx.unlock();
						}
						else{
							mtx.lock();
							if(files.begin() == files.end()){
								mtx.unlock();
								return;
							}
							string name = *(files.begin());
							files.erase(name);
							mtx.unlock();
							DBFS::remove(name);
							if(DBFS::exists(name)){
								TEST_FAILED();
							}
						}
					}, rand()%3);
					v.push_back(move(t));
				}
				for(auto &it : v){
					it.join();
				}
				for(auto &it : files){
					if(!DBFS::exists(it)){
						TEST_FAILED();
					}
					DBFS::remove(it);
				}
				
				TEST_SUCCEED();
			});
		});
		
		DESCRIBE("get_lock should work and block file from being writed from second thread", {
			DBFS::File* f = DBFS::create();
			BEFORE_ALL({
				thread t1([&f](){
					auto lock = f->get_lock();
					for(int i=0;i<10;i++){
						f->write("0");
						this_thread::sleep_for(chrono::milliseconds(10));
					}
				});
				this_thread::sleep_for(chrono::milliseconds(10));
				thread t2([&f](){
					auto lock = f->get_lock();
					for(int i=0;i<10;i++){
						f->write("1");
						this_thread::sleep_for(chrono::milliseconds(10));
					}
				});
				t1.join();
				t2.join();
			});
			IT("content of file should equal to `00000000001111111111`",{
				f->seekg(0);
				char* buf = new char[20];
				f->read(buf,20);
				EXPECT(string(buf,20)).toBe("00000000001111111111");
			});
		});
	});
});


int main(){
	
	return 0;
}
