#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <openssl/md5.h>
#include "tools.h"

using namespace std;

string GetSTDTime(time_t timestamp) {
    if (timestamp == 0) {
        timestamp = time(NULL);
    }
    char buf[128] = {'\0'};
    struct tm tm_value;
    localtime_r(&timestamp, &tm_value);
    strftime(buf, sizeof(buf), "%Y/%m/%d %H:%M:%S", &tm_value);
    return string(buf);
}

string GetMinuteTime(time_t timestamp) {
    if (timestamp == 0) {
        timestamp = time(NULL);
    }
    char buf[128] = {'\0'};
    struct tm tm_value;
    localtime_r(&timestamp, &tm_value);
    strftime(buf, sizeof(buf), "%Y%m%d%H%M", &tm_value);
    return string(buf);
}

string GetHourTime(time_t timestamp) {
    if (timestamp == 0) {
        timestamp = time(NULL);
    }
    char buf[128] = {'\0'};
    struct tm tm_value;
    localtime_r(&timestamp, &tm_value);
    strftime(buf, sizeof(buf), "%Y%m%d%H", &tm_value);
    return string(buf);
}

string GetDayTime(time_t timestamp) {
    if (timestamp == 0) {
        timestamp = time(NULL);
    }
    char buf[128] = {'\0'};
    struct tm tm_value;
    localtime_r(&timestamp, &tm_value);
    strftime(buf, sizeof(buf), "%Y%m%d", &tm_value);
    return string(buf);
}

string GetMd5(const string& str) {
    if (str.empty()) {
        return "";
    }
    unsigned char buf[16] = {'\0'};
    MD5((const unsigned char *)str.data(), str.size(), buf);
    char result[33] = {'\0'};
    char tmp[3] = {'\0'};
    for (int i=0; i<16; i++ ){
        sprintf(tmp,"%02x",buf[i]);
        strcat(result, tmp);
    }
    return result;
}

void InitDaemon(void (*func)(int)) {
    pid_t pid;
    umask(0);
    if ((pid = fork()) < 0) {
        cout << "fork error 1" << endl;
        exit(1);
    }
    else if (pid != 0) {
        exit(0);
    }
    setsid();
    if(func != NULL) {
        signal(SIGTERM, func);
        signal(SIGINT, func);
    }
    signal(SIGHUP,  SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    if ((pid = fork()) < 0) {       
        cout << "fork error 2" << endl;
        exit(1);
    }
    else if (pid != 0) {
        exit(0);
    }
    /* close existing stdin, stdout, stderr */
    close(0);
    close(1);
    close(2);
    /*attach 0,1,2 to /dev/null*/
    int fd0,fd1,fd2;
    fd0 = open("/dev/null", O_RDWR);
    fd1 = dup(0);
    fd2 = dup(0);
    if(fd0 !=0 || fd1 != 1 || fd2 != 2) {
        cout << "attach 0,1,2 to /dev/null error" << endl;
        exit(1);
    }

    return;
}

void Sleep(int seconds) {
    struct timeval tm; 
    tm.tv_sec = seconds;
    tm.tv_usec = 0;
    select(0, NULL, NULL, NULL, &tm);
}

void Usleep(int microseconds) {
    struct timeval tm; 
    tm.tv_sec = microseconds / 1000000;
    tm.tv_usec = microseconds % 1000000;
    select(0, NULL, NULL, NULL, &tm);
}

string StringTrim(const string& str) {
    if (str.empty()) {
        return "";
    }
    int begin = 0;
    int end = str.size();
    while (begin < end && (str[begin] == '\n' || str[begin] == '\t' || str[begin] == ' ')) {
        begin++;
    }
    if (begin == end) {
        return "";
    }
    while (end > begin && (str[end-1] == '\n' || str[end-1] == '\t' || str[end-1] == ' ')) {
        end--;
    }
    return str.substr(begin, end-begin);
}
