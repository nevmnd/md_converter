#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <format>
#include <functional>
#include <filesystem>
#include <md4c-html.h>

class ArgParser {
public:
    ArgParser(int argc, char* argv[]) {
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg.starts_with("-") && arg.size() > 1) {
                std::string key = arg.substr(1);
                if (i + 1 < argc && !std::string(argv[i + 1]).starts_with('-')) {
                    args_[key] = argv[++i];
                } else {
                    args_[key] = "true"; // Default value for flags
                }
            } else {
                positionalArgs_.push_back(arg);
            }
        }
    }

    std::string get(const std::string& key, const std::string& defaultValue = "") const {
        auto it = args_.find(key);
        return it != args_.end() ? it->second : defaultValue;
    }

    bool has(const std::string& key) const {
        return args_.find(key) != args_.end();
    }

    const std::vector<std::string>& positionalArgs() const {
        return positionalArgs_;
    }

private:
    std::unordered_map<std::string, std::string> args_;
    std::vector<std::string> positionalArgs_;
};

class FileHandler {
public:
    FileHandler(const std::string& fileName, std::ios::openmode mode) 
        : file_(fileName, mode), fileName_(fileName), mode_(mode) {
        if (!file_.is_open()) {
            throw std::ios_base::failure("Failed to open file: " + fileName);
        }
    }

    ~FileHandler() {
        if (file_.is_open()) {
            file_.close();
        }
    }

    void writeLine(const std::string& data) {
        file_ << data << std::endl;
        if (!file_) {
            throw std::ios_base::failure("Failed to write to file: " + fileName_);
        }
    }
    void writeBytes(const std::string& data, size_t size) {
        file_.write(data.c_str(), size);
        if (!file_) {
            throw std::ios_base::failure("Failed to write to file: " + fileName_);
        }
    }
    std::string readFile() {
        if (!(mode_ & std::ios::in)) {
            throw std::ios_base::failure("File is not opened in read mode.");
        }

        std::ostringstream oss;
        oss << file_.rdbuf();
        return oss.str();
    }

private:
    std::fstream file_;
    std::string fileName_;
    std::ios::openmode mode_;
};

inline FileHandler safeOpenFile(const std::string& fileName, std::ios::openmode mode) {
    try {
        return FileHandler(fileName, mode);
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        std::exit(1);
    }
}

FileHandler* outPtr = nullptr;

static void process_output(const MD_CHAR* text, MD_SIZE size, void* userdata)
{
    outPtr->writeBytes(text, size);
}

int main(int argc, char* argv[]) {
    ArgParser parser(argc, argv);
    std::string css_file;
    int ret = -1;

    if (parser.has("help") || parser.positionalArgs().size() != 2) {
        std::cout << "Usage: program [options] INPUT OUTPUT\n"
                  << "Options:\n"
                  << "  -h                  Show this help message\n"
                  << "  -c                  CSS file\n"
                  << "  INPUT               Input .md file\n"
                  << "  OUTPUT              Output .html file\n"
                  << std::endl;
        return 0;
    }

    std::string md_file = parser.positionalArgs()[0];
    std::cout << "Input file:  " << md_file << std::endl;
    if (!std::filesystem::exists(md_file)) {
        std::cout << "Input file doesn't exist. Exiting..." << std::endl;
        std::exit(1);
    }
    FileHandler in = safeOpenFile(md_file, std::ios::in);
    std::string html_file = parser.positionalArgs()[1];
    std::cout << "Output file: " << html_file << std::endl;
    // Remove the file as output will be appended by pieces
    if (std::filesystem::exists(html_file)) {
        std::cout << "Output file exists. Removing it..." << std::endl;
        std::filesystem::remove(html_file);
    }
    FileHandler out = safeOpenFile(html_file, std::ios::app);
    outPtr = &out;
    if (parser.has("c")) {
        css_file = parser.get("c");
        std::cout << "CSS file:    " << css_file << std::endl;
        if (!std::filesystem::exists(css_file)) {
            std::cout << "CSS file doesn't exist. Exiting..." << std::endl;
            std::exit(1);
        }
    }
    std::string md_buffer = in.readFile();
    out.writeLine("<!DOCTYPE html>");
    out.writeLine("<html>");
    out.writeLine("<head>");
    out.writeLine("<title></title>");
    out.writeLine("<meta charset=\"UTF-8\" />");
    if (!css_file.empty()) {
        out.writeLine(std::format("<link rel=\"stylesheet\" href=\"{}\">", css_file));
    }
    out.writeLine("</head>");
    out.writeLine("<body>");
    ret = md_html(md_buffer.c_str(), md_buffer.size(), process_output, NULL, MD_DIALECT_COMMONMARK, MD_HTML_FLAG_DEBUG | MD_HTML_FLAG_SKIP_UTF8_BOM);
    out.writeLine("</body>");
    out.writeLine("</html>");
    std::cout << "Done!" << std::endl;
    std::cout << "Output file is " << html_file << std::endl;

    return 0;
}