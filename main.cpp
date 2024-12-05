#include<iostream>
#include<time.h>
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/time.h>
#include <math.h>
#endif
#include "my_serial.hpp"
#include <sstream>  

template<class T> std::string to_string(const T& v)
{
    std::ostringstream ss;
    ss << v;
    return ss.str();
}

void csleep(double timeout) {
#if defined (WIN32)
	if (timeout <= 0.0)
        ::Sleep(INFINITE);
    else
        ::Sleep((DWORD)(timeout * 1e3));
#else
    if (timeout <= 0.0)
        pause();
    else {
        struct timespec t;
        t.tv_sec = (int)timeout;
        t.tv_nsec = (int)((timeout - t.tv_sec)*1e9);
        nanosleep(&t, NULL);
    }
#endif
}



int main(int argc, char** argv){
 cplib::SerialPort smport(std::string(argv[1]),cplib::SerialPort::BAUDRATE_115200);
 if (!smport.IsOpen()) {
		std::cout << "Failed to open port '" << argv[1] << "'! Terminating..." << std::endl;
		return -2;
	}
 for (int i = 0;;i++) {
  smport << to_string(rand()%30);
  //std::cout << std::to_string(rand()%30) + '\n';
  csleep(1.0);
	}
 return 0;
}