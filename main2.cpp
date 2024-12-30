#include "my_serial.hpp"
#include <sstream>  
#include<iostream>
#include<time.h>
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/time.h>
#include <math.h>
#endif


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



class Time{
	public:

#ifdef WIN32

	long getTime(){
		union{
			long long ns100;
			FILETIME ft;
		} now;
		GetSystemTimeAsFileTime(&(now.ft));
		return (long)((now.ns100 / 10LL));
	}

	std::string getLocalTime(){
		SYSTEMTIME st;
 	GetLocalTime(&st);
		return (std::to_string(st.wYear) + " " + std::to_string(st.wMonth) + " " + std::to_string(st.wDay) + " " + std::to_string(st.wHour) + ":" + std::to_string(st.wMinute) + ":" + std::to_string(st.wSecond) + ":" + std::to_string(st.wMilliseconds) + "\n").data(); 
	}

int getMonth(){
 SYSTEMTIME st;
 GetLocalTime(&st);
 return st.wMonth;
}

int getDay(){
 SYSTEMTIME st;
 GetLocalTime(&st);
 return st.wDay;
}

int getHour(){
 SYSTEMTIME st;
 GetLocalTime(&st);
 return st.wHour;
}

int getYear(){
 SYSTEMTIME st;
 GetLocalTime(&st);
 return st.wYear;
}

#else
	unsigned long getTime(){
		struct timeval tv;
		gettimeofday(&tv,NULL);
		return 1000000 * tv.tv_sec + tv.tv_usec;
	}

	std::string getLocalTime(){
  char buffer[26];
  int millisec;
  struct tm* tm_info;
  struct timeval tv;

  gettimeofday(&tv, NULL);

  millisec = lrint(tv.tv_usec/1000.0); 
  if (millisec>=1000) { 
    millisec -=1000;
    tv.tv_sec++;
  }

  tm_info = localtime(&tv.tv_sec);

  strftime(buffer, 26, "%Y %m %d %H:%M:%S", tm_info);
		std::string tmp1223 = (std::string)buffer+":"+std::to_string(millisec) + "\n";
		return tmp1223;
	}

int getMonth(){
 time_t theTime = time(NULL);
 struct tm *aTime = localtime(&theTime);
 return  aTime->tm_mon + 1;
}

int getDay(){
 time_t theTime = time(NULL);
 struct tm *aTime = localtime(&theTime);
 return  aTime->tm_mday;
}

int getHour(){
 time_t theTime = time(NULL);
 struct tm *aTime = localtime(&theTime);
 return  aTime->tm_hour + 1;
}

int getYear(){
 time_t theTime = time(NULL);
 struct tm *aTime = localtime(&theTime);
 return  aTime->tm_year + 1900;
}

#endif

};

bool isNumber(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

int main(int argc, char** argv){

 Time t;
 std::string mystr;
 int sumInHour = 0;
 int sumInDay = 0;

 int countInHour = 0;
 int countInDay = 0;

 FILE* allNoticeFile = fopen("temp.txt", "a");
 FILE* hourNoticeFile = fopen("tempInHour.txt", "a");
 FILE* dayNoticeFile = fopen("tempInDay.txt", "a");

 int lastDay = t.getDay();
 int lastMonth = t.getMonth();
 int lastHour = t.getHour();
 int lastYear = t.getYear();

 cplib::SerialPort smport(std::string(argv[1]),cplib::SerialPort::BAUDRATE_115200);
 if (!smport.IsOpen()) {
		std::cout << "Failed to open port '" << argv[1] << "'! Terminating..." << std::endl;
		return -2;
	}
 smport.SetTimeout(1.0);

 for (;;) {

  smport >> mystr;
  if (!isNumber(mystr)){
   continue;
  }
  std::cout << "Got: "<< (mystr.empty()?"nothing": mystr) << std::endl;
  if (!mystr.empty()){
   fprintf(allNoticeFile, "time: %s\n", t.getLocalTime().c_str());
   fprintf(allNoticeFile, "Temperature: %d\n\n", std::stoi(mystr));
   fflush(allNoticeFile);

   sumInHour += std::stoi(mystr);
   sumInDay += std::stoi(mystr);

   countInHour++;
   countInDay++;

   if (t.getHour() - lastHour){
    fprintf(hourNoticeFile, "time: %s\n", t.getLocalTime().c_str());
    fprintf(hourNoticeFile, "Temperature: %d\n\n", sumInHour/countInHour);
    fflush(hourNoticeFile);
    countInHour = 0;
    sumInHour = 0;
    lastHour = t.getHour();
   }
   
   if (t.getDay() - lastDay){
    fclose(allNoticeFile);
    allNoticeFile = fopen("temp.txt", "w");


    fprintf(dayNoticeFile, "time: %s\n", t.getLocalTime().c_str());
    fprintf(dayNoticeFile, "Temperature: %d\n\n", sumInDay/countInDay);
    fflush(dayNoticeFile);
    countInDay = 0;
    sumInDay = 0;
    lastDay = t.getDay();
   }
   if (t.getMonth() - lastMonth){
    fclose(hourNoticeFile);
    hourNoticeFile = fopen("tempInHour.txt", "w");
    lastMonth = t.getMonth();
   }
   if (t.getYear() - lastYear){
    fclose(dayNoticeFile);
    dayNoticeFile = fopen("tempInDay.txt", "w");
    lastYear = t.getYear();
  }

  }
 }
 fclose(hourNoticeFile);
 fclose(dayNoticeFile);
 fclose(dayNoticeFile);

 return 0;
}