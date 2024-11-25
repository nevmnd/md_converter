#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <format>
#include <functional>
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

    void write(const std::string& data) {
        if (!(mode_ & std::ios::out)) {
            throw std::ios_base::failure("File is not opened in write mode.");
        }
        file_ << data;
        if (!file_) {
            throw std::ios_base::failure("Failed to write to file: " + fileName_);
        }
    }

    std::string read() {
        if (!(mode_ & std::ios::in)) {
            throw std::ios_base::failure("File is not opened in read mode.");
        }

        std::ostringstream oss;
        oss << file_.rdbuf();
        return oss.str();
    }

    bool isEOF() const {
        return file_.eof();
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

int main(int argc, char* argv[]) {
    ArgParser parser(argc, argv);
    std::string css_file;
    int ret = -1;

    std::cout << "Start" << std::endl;
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

    std::string input_file = parser.positionalArgs()[0];
    std::cout << "Input file:  " << input_file << std::endl;
    FileHandler in = safeOpenFile(input_file, std::ios::in);
    std::string output_file = parser.positionalArgs()[1];
    std::cout << "Output file: " << output_file << std::endl;
    FileHandler out = safeOpenFile(output_file, std::ios::out);
    if (parser.has("c")) {
        std::string css_file = parser.get("c");
        std::cout << "CSS file:    " << css_file << std::endl;
        FileHandler css = safeOpenFile(css_file, std::ios::in);
    }
    std::string input_buffer = in.read();
    out.write("<!DOCTYPE html>\n");
    out.write("<html>\n");
    out.write("<head>\n");
    out.write("<title></title>\n");
    if (!css_file.empty()) {
        out.write(std::format("link rel=\"stylesheet\" href=\"{}\">", css_file));
    }
    out.write("</head>\n");
    out.write("<body>\n");
    // ret = md_html(input_buffer.c_str(), input_buffer.size(), process_output, &buf_out, MD_DIALECT_COMMONMARK, MD_HTML_FLAG_DEBUG);
    std::cout << "md_html returned" << ret;
    out.write(input_buffer);
    out.write("</body>\n");
    out.write("</html>\n");

    return 0;
}