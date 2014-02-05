/*****************************************************************************
 *                                                                           *
 *  Copyright (C)  2013-2014 Boris Pek <tehnick-8@mail.ru>                   *
 *                                                                           *
 *  This program is free software; you can redistribute it and/or modify     *
 *  it under the terms of the GNU General Public License as published by     *
 *  the Free Software Foundation; either version 2 of the License, or        *
 *  (at your option) any later version.                                      *
 *                                                                           *
 *  This program is distributed in the hope that it will be useful,          *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU General Public License for more details.                             *
 *                                                                           *
 *  You should have received a copy of the GNU General Public License        *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 *****************************************************************************/

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>

#include <sys/types.h>
#include <sys/stat.h>
#include <cstdlib>

#include <fcgi_stdio.h>

#include "Version.h"

using namespace std;

string basic_url = "/bosixnet/";
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

string get_env_var(const string &);
string get_post_request();
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
        else if (request_method == "POST") {

            content = get_post_request();
        }
        else {
            show_html("<center><h2>Unknown REQUEST_METHOD. Use only GET or POST!</h2></center>\r\n");
            continue;
        };

        string full_path = get_env_var("SCRIPT_NAME");
        string path = get_env_var("SCRIPT_FULL_PATHNAME");

        if (ends_with(full_path, basic_url)) {
            show_hosts();
        }
        else if(ends_with(full_path, basic_url + "hosts")) {
            show_hosts();
        }
        else if(ends_with(full_path, basic_url + "counter")) {
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
            else {
                hosts_map[host_name] = new_address;
                show_html(new_address);
                write_hosts();
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
            if (arg == "-b" || arg == "--basic-url") {
                basic_url = arg_next;
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
                var = "BASIC_URL";
                tmp = get_conf_var(buff, var);
                if (!tmp.empty()) {
                    basic_url = tmp;
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
    cout << "Usage: bosixnet-webui [options]\n"
            "\n"
            "FastCGI program which passively listens for incoming connections and\n"
            "generates list of hosts in your IPv6 network. This daemon prepares data\n"
            "which may be put directly into /etc/hosts.\n"
            "\n"
            "Generic options:\n"
            "  -h, --help    show help\n"
            "  -v, --vesion  show vesion\n"
            "\n"
            "Options:\n"
            "  -b <string>, --basic-url <string>  set basic url (default: " + basic_url + ")\n"
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
    printf("Content-type: text/html\r\n\r\n");
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
                hosts_map[host] = addr;
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

string get_env_var(const string &var)
{
    char *ptr = getenv(var.c_str());
    return (ptr ? string(ptr) : "");
}

string get_post_request()
{
    char *content = NULL;
    char *endptr = NULL;
    unsigned int contentlength;
    const char *clen = getenv("CONTENT_LENGTH");
    contentlength = strtol(clen, &endptr, 10);
    if (contentlength) {
        content = new char[contentlength];
        fread(content, contentlength, 1, stdin);
        content[contentlength] = '\0';
    }
    return (content ? string(content) : "");
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

