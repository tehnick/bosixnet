/*****************************************************************************
 *                                                                           *
 *  Copyright (c) 2013-2016 Boris Pek <tehnick-8@mail.ru>                    *
 *                                                                           *
 *  Permission is hereby granted, free of charge, to any person obtaining    *
 *  a copy of this software and associated documentation files (the          *
 *  "Software"), to deal in the Software without restriction, including      *
 *  without limitation the rights to use, copy, modify, merge, publish,      *
 *  distribute, sublicense, and/or sell copies of the Software, and to       *
 *  permit persons to whom the Software is furnished to do so, subject to    *
 *  the following conditions:                                                *
 *                                                                           *
 *  The above copyright notice and this permission notice shall be included  *
 *  in all copies or substantial portions of the Software.                   *
 *                                                                           *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,          *
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF       *
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.   *
 *  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY     *
 *  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,     *
 *  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE        *
 *  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                   *
 *                                                                           *
 *****************************************************************************/

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>

#include <sys/types.h>
#include <sys/stat.h>
#include <cstdlib>
#include <cstring>

#include <fcgi_stdio.h>

#include "Version.h"

using namespace std;

string basic_str = "/bosixnet/";
string log_dir = "/var/tmp/bosixnet";
string conf_file = "/etc/bosixnet/bosixnet-webui.conf";

map<string, string> hosts_map;

bool check_options(int, char **);
void read_options(int, char **);
void read_config();

void show_help();
void show_version();
void show_html(const string &);
void show_hosts();
void read_hosts();
void write_hosts();

bool ends_with(const string &, const string &);
bool starts_with(const string &, const string &);
bool is_valid_ipv6_address(const string &);

string get_env_var(const string &);
string get_param(const string &, const string &);
string get_conf_var(const string &, const string &);
string remove_extra_symbols(const string &, const string &);

int main(int argc, char **argv)
{
    if (check_options(argc, argv))
        return 0;

    // Settings in config file have lower priority than command line options.
    string conf_file_default = conf_file;
    read_config();
    read_options(argc, argv);
    if (conf_file != conf_file_default) {
        read_config();
        read_options(argc, argv);
    }

    read_hosts();

    int counter = 0;
    while (FCGI_Accept() >= 0) {
        ++counter;

        string content;
        string request_method = get_env_var("REQUEST_METHOD");
        if (request_method.empty()) {
            continue;
        }
        if (request_method == "GET") {
            content = get_env_var("QUERY_STRING");
        }
        else {
            show_html("<center><h2>Only GET request method is allowed!</h2></center>\n");
            continue;
        };

        string full_path = get_env_var("SCRIPT_NAME");
        string path = get_env_var("SCRIPT_FULL_PATHNAME");

        if (ends_with(full_path, basic_str)) {
            show_hosts();
        }
        else if (ends_with(full_path, basic_str + "hosts")) {
            show_hosts();
        }
        else if (ends_with(full_path, basic_str + "counter")) {
            stringstream counter_str;
            counter_str << counter;
            show_html("Counter: " + counter_str.str());
        }
        else {
            string host_name = full_path.substr(full_path.rfind("/") + 1);
            string new_address = get_param(content, "update=");
            if (new_address.empty()) { 
                map<string, string>::iterator it = hosts_map.find(host_name);
                if (it != hosts_map.end()) {
                    show_html(it->second);
                }
                else {
                    show_html("");
                }
            }
            else if (is_valid_ipv6_address(new_address)) {
                if (hosts_map[host_name] != new_address) {
                    hosts_map[host_name] = new_address;
                    write_hosts();
                }
                show_html(new_address);
            }
            else {
                show_html(new_address + " is not a valid IPv6 address!");
            }
        }
    }

    return 0;
}

bool check_options(int argc, char **argv)
{
    string arg;
    for (int idx = 1 ; idx < argc ; ++idx) {
        arg = argv[idx];
        if (arg == "-h" || arg == "--help") {
            show_help();
            return true;
        }
        else if (arg == "-v" || arg == "--version") {
            show_version();
            return true;
        }
    }
    return false;
}

void read_options(int argc, char **argv)
{
    if (argc > 2) {
        string arg;
        string arg_next;
        for (int idx = 1 ; idx < argc - 1 ; ++idx) {
            arg = argv[idx];
            arg_next = argv[idx + 1];
            if (arg == "-b" || arg == "--basic-str") {
                basic_str = arg_next;
            }
            else if (arg == "-l" || arg == "--log-dir") {
                log_dir = arg_next;
            }
            else if (arg == "-c" || arg == "--conf-file") {
                conf_file = arg_next;
            }
        }
    }
}

void read_config()
{
    ifstream file;
    file.open(conf_file.c_str(), ios::in);
    if (file.is_open()) {
       string buff, var, tmp;
       while (!file.eof()) {
            getline(file, buff);
            buff = remove_extra_symbols(buff, " \t");
            if (buff.size() >= 3 && buff.at(0) != '#') {
                var = "BASIC_STR";
                tmp = get_conf_var(buff, var);
                if (!tmp.empty()) {
                    basic_str = tmp;
                }
                var = "LOG_DIR";
                tmp = get_conf_var(buff, var);
                if (!tmp.empty()) {
                    log_dir = tmp;
                }
            }
        }
        file.close();
    }
}

void show_help()
{
    cout << "Usage: bosixnet_webui [options]\n"
            "\n"
            "FastCGI program which passively listens for incoming connections and\n"
            "generates list of hosts in your IPv6 network. This daemon prepares data\n"
            "which may be put directly into /etc/hosts.\n"
            "\n"
            "Generic options:\n"
            "  -h, --help     show help\n"
            "  -v, --version  show version\n"
            "\n"
            "Options:\n"
            "  -b <string>, --basic-str <string>  set basic url (default: " + basic_str + ")\n"
            "  -l <dir>, --log-dir <dir>          set log directory (default: " + log_dir + ")\n"
            "  -c <file>, --conf-file <file>      set config file (default: " + conf_file + ")\n"
            "\n"
            "Settings in config file have lower priority than command line options.\n";
}

void show_version()
{
    cout << "Version: " << string(VERSION) << "\n";
}

void show_html(const string &str)
{
    printf("Content-type: text/html\n\n");
    printf("%s", str.c_str());
}

void show_hosts()
{
    string log_file = log_dir + "/hosts";
    string out = "File " + log_file + " is empty!";
    ifstream file;
    file.open(log_file.c_str(), ios::in);
    if (file.is_open()) {
        out.clear();
        string buff;
        while (!file.eof()) {
            getline(file, buff);
            out += buff + "<br>\n";
        }
        file.close();
    }
    show_html(out);
}

void read_hosts()
{
    string log_file = log_dir + "/hosts";
    ifstream file;
    file.open(log_file.c_str(), ios::in);
    if (file.is_open()) {
       string buff, host, addr;
       stringstream str;
       while (!file.eof()) {
            getline(file, buff);
            str.clear();
            str << buff;
            str >> addr;
            str >> host;
            if (!addr.empty() && !host.empty()){
                if (is_valid_ipv6_address(addr)){
                    hosts_map[host] = addr;
                }
            }
        }
        file.close();
    }
}

void write_hosts()
{
    struct stat info;
    if (stat(log_dir.c_str(), &info)) {
        if (mkdir(log_dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)) {
            cout << "Directory " << log_dir << " was not created!\n";
            return;
        }
    }
    string log_file = log_dir + "/hosts";
    ofstream file;
    file.open(log_file.c_str(), ios::out);
    if (file.is_open()) {
        for (map<string, string>::iterator it = hosts_map.begin(); it != hosts_map.end(); ++it) {
            file <<  it->second << " " << it->first << "\n";
        }
        file.close();
    }
}

bool ends_with(const string &str, const string &sfx)
{
    if (sfx.size() > str.size())
        return false;

    return equal(str.begin() + str.size() - sfx.size(), str.end(), sfx.begin());
}

bool starts_with(const string &str, const string &pfx)
{
    if (pfx.size() > str.size())
        return false;

    return equal(str.begin(), str.begin() + pfx.size(), pfx.begin());
}

bool is_valid_ipv6_address(const string &str)
{
    size_t len = str.size();
    if (len < 8)
        return false;
    else if (len > 39)
        return false;

    unsigned int counter = 0;
    const char *p = str.c_str();
    while (strstr(p,":")) {
        ++counter;
        ++p;
    }

    if (counter < 3)
        return false;
    else if (str.at(4) != ':')
        return false;

    if (str.find_first_not_of(":0123456789abcdefABCDEF") != string::npos)
        return false;

    return true;
}

string get_env_var(const string &var)
{
    char *ptr = getenv(var.c_str());
    return (ptr ? string(ptr) : "");
}

string get_param(const string &buff, const string &name)
{
    if (buff.empty() || name.empty())
        return "";

    size_t param_begin = buff.find(name);
    if (param_begin == string::npos)
        return "";

    param_begin += name.size();
    if (param_begin >= buff.size())
        return "";

    size_t param_end = buff.find("&", param_begin);
    if (param_end == string::npos)
        param_end = buff.size();

    string out = buff.substr(param_begin, param_end - param_begin);
    return out;
}

string get_conf_var(const string &buff, const string &var)
{
    if (!starts_with(buff, var))
        return "";

    string out = buff.substr(var.size());
    out = remove_extra_symbols(out, "= \t\"");
    return out;
}

string remove_extra_symbols(const string &str, const string &symbols)
{
    string out = str;
    size_t pos = out.find_first_not_of(symbols);

    if (pos != string::npos) {
        out = out.substr(pos);
    }

    pos = out.find_last_not_of(symbols);

    if (pos != string::npos && pos != out.size() - 1) {
        out = out.substr(0, pos + 1);
    }

    return out;
}

