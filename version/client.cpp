#include "httplib.h"
#include <sstream>
#include <stdio.h>
#include <fstream>

#include "md5.h"

std::vector<std::string> split(const std::string& s, char seperator)
{
    std::vector<std::string> output;
    std::string::size_type prev_pos = 0, pos = 0;

    while((pos = s.find(seperator, pos)) != std::string::npos) {
        std::string substring( s.substr(prev_pos, pos-prev_pos) );
        output.push_back(substring);
        prev_pos = ++pos;
    }

    output.push_back(s.substr(prev_pos, pos-prev_pos)); // Last word
    return output;
}

std::string getFileHash(const std::string &file) {
    std::string cont;
    {
        std::fstream f(file);
        std::stringstream ss;
        ss << f.rdbuf();
        cont = ss.str();
    }
    Chocobo1::MD5 md5;
    md5.addData(cont.c_str(), cont.size());
    md5.finalize();
    return md5.toString();
}

void writeVersion(const std::string &path, const std::string &version, const std::string &build_number) {
    std::string msg_file = path;
    msg_file += "/shared/messages.hpp";
    std::string msg_hash = getFileHash(msg_file);
    printf("version: %s %s %s\n", version.c_str(), build_number.c_str(), msg_hash.c_str());
    std::string version_file = path;
    version_file += "/shared/version.hpp";
    FILE *f = fopen(version_file.c_str(), "w");
    fprintf(f, "#define BUILD_NUMBER ");
    fwrite(build_number.c_str(), build_number.size(), 1, f);

    fprintf(f, "\n#define BUILD_VERSION \"");
    fwrite(version.c_str(), version.size(), 1, f);
    fprintf(f, "\"\n");

    fprintf(f, "#define MSG_HASH \"%s\"\n", msg_hash.c_str());

    fclose(f);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        return 1;
    }
    httplib::Client cli("http://gudov.info:26901");

    std::string path = "/version/";
    path += std::string(argv[1]);
    auto res = cli.Get(path);
    fflush(stdout);
    if (res->status != 200) {
        writeVersion(argv[2], "unk_non_200", "0");
    } else {
        auto versions = split(res->body, ' ');
        if (versions.size() != 2) {
            writeVersion(argv[2], "unk_wrong_response", "0");
        } else {
            writeVersion(argv[2], versions[1], versions[0]);
        }
    }

    return 0;
}