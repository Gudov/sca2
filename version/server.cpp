#include "httplib.h"
#include <stdio.h>
#include <filesystem>

int main() {
    httplib::Server svr;

    svr.Get("/version/:project", [](const httplib::Request &req, httplib::Response &res) {
        auto project = req.path_params.at("project");
        std::string path = "v/";
        path += project;
        std::string build_number, version;
        if (std::filesystem::exists(path)) {
            {
                std::fstream file(path, std::ios_base::in);
                file >> build_number >> version;
            }
        } else {
            build_number = "0";
            version = "unk";
        }

        {
            std::fstream file(path, std::ios_base::out);
            int b = std::stoi(build_number);
            file << std::to_string(b + 1) << " " << version;
        }

        printf("return version %s build_number %s for %s\n", version.c_str(), build_number.c_str(), project.c_str());
        std::string result;
        result += build_number;
        result += " ";
        result += version;
        res.set_content(result, "text/plain");
    });

    svr.listen("0.0.0.0", 26901);

    return 0;
}
