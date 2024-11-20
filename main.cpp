#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

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

    // Check if a key exists
    bool has(const std::string& key) const {
        return args_.find(key) != args_.end();
    }

    // Get all positional arguments
    const std::vector<std::string>& positionalArgs() const {
        return positionalArgs_;
    }

private:
    std::unordered_map<std::string, std::string> args_;
    std::vector<std::string> positionalArgs_;
};

int main(int argc, char* argv[]) {
    ArgParser parser(argc, argv);

    // Example usage
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

    std::cout << "Input file:  " << parser.positionalArgs()[0] << std::endl;
    std::cout << "Output file: " << parser.positionalArgs()[1] << std::endl;
    if (parser.has("c")) {
        std::cout << "CSS file:    " << parser.get("c") << std::endl;
    }

    return 0;
}