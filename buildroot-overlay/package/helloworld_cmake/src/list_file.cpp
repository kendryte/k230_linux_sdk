#include <sys/stat.h>
#include <dirent.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "list_file.h"
#include <errno.h>
#include <string.h>

std::vector<std::string> get_input_list(const char *dataset_path) {
    std::vector<std::string> v;
    v.reserve(10000);

    DIR *d;
    struct dirent *dir;
    if ((d = opendir(dataset_path)) != NULL)
    {
        std::string base(dataset_path);
        while ((dir = readdir(d)) != NULL)
        {
            if (dir->d_type == DT_REG)
            {
                v.push_back(base + "/" + std::string(dir->d_name));
            }
        }
        if (errno != 0) {
			fprintf(stderr, "Error reading directory: %s\n", strerror(errno));
		}
        closedir(d);
    }
    else
    {
        printf("Unable to open %s\n", dataset_path);
    }

    return v;
}

void dump_string_list(const std::string &info, const std::vector<std::string> &v)
{
    std::cout << info << ", v.size = " << v.size() << std::endl;
    // for (size_t i = 0; i < v.size(); i++)
    //     std::cout << v[i] << std::endl;
}

int list_files(int argc, char *argv[])
{
    std::cout << "case " << argv[0] << " build " << __DATE__ << " " << __TIME__ << std::endl;
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <dataset_input>" << std::endl;
        return -1;
    }

    // dataset
    auto input_id_list = get_input_list(argv[1]);
    printf("befor l=%d\n",input_id_list.size());
    dump_string_list("before sort input id", input_id_list);
    std::sort(input_id_list.begin(), input_id_list.end());
    dump_string_list("after sort input id", input_id_list);
    printf("l=%d\n",input_id_list.size());
    return 0;
}
