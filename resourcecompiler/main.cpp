/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
#include <cstdint>
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <pugixml.hpp>

namespace {

struct InputFile {
    std::filesystem::path source;
    std::string resourceName;
    std::uintmax_t size;
};

std::filesystem::path fromUtf8(const char *value) {
    return std::filesystem::u8path(value);
}

std::string trim(const std::string &value) {
    const size_t first = value.find_first_not_of(" \t\r\n");
    if(first == std::string::npos) {
        return std::string();
    }
    const size_t last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

bool normalizeResourceName(const std::string &value, std::string &result) {
    std::string path(value);
    std::replace(path.begin(), path.end(), '\\', '/');

    std::stringstream stream(path);
    std::string component;
    result.clear();
    while(std::getline(stream, component, '/')) {
        if(component.empty() || component == ".") {
            continue;
        }
        if(component == "..") {
            return false;
        }
        if(!result.empty()) {
            result += '/';
        }
        result += component;
    }
    return !result.empty();
}

bool writeString(std::ostream &output, const std::string &value) {
    output.put('"');
    for(unsigned char ch : value) {
        if(ch == '\\' || ch == '"') {
            output.put('\\');
            output.put(static_cast<char>(ch));
        } else if(ch >= 0x20 && ch <= 0x7e) {
            output.put(static_cast<char>(ch));
        } else {
            output.put('\\');
            output.put(static_cast<char>('0' + ((ch >> 6) & 0x07)));
            output.put(static_cast<char>('0' + ((ch >> 3) & 0x07)));
            output.put(static_cast<char>('0' + (ch & 0x07)));
        }
    }
    output.put('"');
    return output.good();
}

void showUsage(const char *program) {
    std::cout << "Usage: " << program << " --output <file.cpp> <manifest.xml>\n"
              << "       " << program << " --output <file.cpp> --manifest <manifest.xml>\n"
              << "       " << program << " --output <file.cpp> [--root <directory>] <files...>\n"
              << "Embed arbitrary files in a C++ source file.\n"
              << "The XML manifest contains resource groups with optional prefixes and file aliases.\n"
              << "Paths in the manifest are relative to the manifest file.\n";
}

bool resourceName(const std::filesystem::path &source,
                  const std::filesystem::path &root,
                  std::string &name) {
    std::filesystem::path relative = root.empty() ?
                source.filename() : source.lexically_relative(root);
    if(relative.empty() || relative.is_absolute()) {
        return false;
    }
    for(const auto &part : relative) {
        if(part == "..") {
            return false;
        }
    }
    return normalizeResourceName(relative.generic_u8string(), name);
}

bool loadManifest(const std::filesystem::path &manifestPath,
                  std::vector<InputFile> &files,
                  std::set<std::string> &resourceNames) {
    std::ifstream manifestFile(manifestPath, std::ios::binary);
    if(!manifestFile) {
        std::cerr << "Unable to open resource manifest: " << manifestPath.u8string() << '\n';
        return false;
    }
    std::string xml((std::istreambuf_iterator<char>(manifestFile)), std::istreambuf_iterator<char>());

    pugi::xml_document document;
    pugi::xml_parse_result parseResult = document.load_buffer(xml.data(), xml.size());
    if(!parseResult) {
        std::cerr << "Unable to parse resource manifest " << manifestPath.u8string()
                  << ": " << parseResult.description() << " at offset "
                  << parseResult.offset << '\n';
        return false;
    }

    pugi::xml_node root = document.document_element();
    if(!root) {
        std::cerr << "Resource manifest has no root element: "
                  << manifestPath.u8string() << '\n';
        return false;
    }

    bool foundGroup = false;
    for(pugi::xml_node child : root.children()) {
        if(child.type() != pugi::node_element) {
            continue;
        }
        if(std::string(child.name()) != "qresource") {
            std::cerr << "Unexpected element in resource manifest: " << child.name() << '\n';
            return false;
        }
        foundGroup = true;

        const std::string prefixValue = child.attribute("prefix").as_string();
        std::string prefix;
        const bool rootPrefix = prefixValue.find_first_not_of("/\\") == std::string::npos;
        if(!prefixValue.empty() && !rootPrefix &&
           !normalizeResourceName(prefixValue, prefix)) {
            std::cerr << "Invalid resource group prefix: " << child.attribute("prefix").as_string() << '\n';
            return false;
        }

        for(pugi::xml_node fileNode : child.children()) {
            if(fileNode.type() != pugi::node_element) {
                continue;
            }
            if(std::string(fileNode.name()) != "file") {
                std::cerr << "Unexpected element in resource group: " << fileNode.name() << '\n';
                return false;
            }

            const std::string filePath = trim(fileNode.text().as_string());
            if(filePath.empty()) {
                std::cerr << "Empty file path in resource manifest.\n";
                return false;
            }
            std::filesystem::path relativePath = fromUtf8(filePath.c_str());
            if(relativePath.is_absolute()) {
                std::cerr << "Resource manifest paths must be relative: " << filePath << '\n';
                return false;
            }

            const std::string alias = fileNode.attribute("alias") ?
                        fileNode.attribute("alias").as_string() : filePath;
            std::string resourcePath;
            if(!normalizeResourceName(alias, resourcePath)) {
                std::cerr << "Invalid resource alias or file path: " << alias << '\n';
                return false;
            }
            if(!prefix.empty()) {
                resourcePath = prefix + "/" + resourcePath;
            }
            if(!normalizeResourceName(resourcePath, resourcePath)) {
                std::cerr << "Invalid resource path after applying prefix: " << resourcePath << '\n';
                return false;
            }
            if(!resourceNames.insert(resourcePath).second) {
                std::cerr << "Duplicate embedded resource name: " << resourcePath << '\n';
                return false;
            }

            std::filesystem::path source = std::filesystem::absolute(
                        manifestPath.parent_path() / relativePath).lexically_normal();
            std::error_code error;
            if(!std::filesystem::is_regular_file(source, error) || error) {
                std::cerr << "Resource file does not exist or is not a file: "
                          << source.u8string() << '\n';
                return false;
            }
            const std::uintmax_t size = std::filesystem::file_size(source, error);
            if(error) {
                std::cerr << "Unable to get resource file size: " << source.u8string() << '\n';
                return false;
            }
            files.push_back({source, resourcePath, size});
        }
    }
    if(!foundGroup) {
        std::cerr << "Resource manifest contains no resource groups.\n";
        return false;
    }
    return true;
}

}

int main(int argc, char *argv[]) {
    std::filesystem::path outputPath;
    std::filesystem::path root;
    std::filesystem::path manifestPath;
    std::vector<std::filesystem::path> inputPaths;

    for(int i = 1; i < argc; ++i) {
        std::string argument(argv[i]);
        if(argument == "-h" || argument == "--help") {
            showUsage(argv[0]);
            return 0;
        }
        if(argument == "-o" || argument == "--output" ||
           argument == "--root" || argument == "--manifest") {
            if(i + 1 >= argc) {
                std::cerr << "Missing value for " << argument << ".\n";
                return 1;
            }
            std::filesystem::path value = fromUtf8(argv[++i]);
            if(argument == "--root") {
                root = std::filesystem::absolute(value).lexically_normal();
            } else if(argument == "--manifest") {
                manifestPath = std::filesystem::absolute(value).lexically_normal();
            } else {
                outputPath = value;
            }
            continue;
        }
        inputPaths.push_back(fromUtf8(argv[i]));
    }

    if(manifestPath.empty() && inputPaths.size() == 1) {
        std::string extension = inputPaths.front().extension().u8string();
        std::transform(extension.begin(), extension.end(), extension.begin(),
                       [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
        if(extension == ".qrc") {
            manifestPath = std::filesystem::absolute(inputPaths.front()).lexically_normal();
            inputPaths.clear();
        }
    }

    if(outputPath.empty() || (manifestPath.empty() == inputPaths.empty()) ||
       (!manifestPath.empty() && !root.empty())) {
        showUsage(argv[0]);
        return 1;
    }

    std::filesystem::path absoluteOutputPath =
                std::filesystem::absolute(outputPath).lexically_normal();
    std::filesystem::path absoluteTemporaryPath = absoluteOutputPath;
    absoluteTemporaryPath += ".tmp";
    std::vector<InputFile> files;
    std::set<std::string> resourceNames;
    if(!manifestPath.empty()) {
        if(!loadManifest(manifestPath, files, resourceNames)) {
            return 1;
        }
    }
    if(!root.empty() && !std::filesystem::is_directory(root)) {
        std::cerr << "Root path is not a directory: " << root.u8string() << '\n';
        return 1;
    }

    for(const InputFile &file : files) {
        if(file.source == absoluteOutputPath || file.source == absoluteTemporaryPath) {
            std::cerr << "Output source or its temporary file cannot also be an input file: "
                      << file.source.u8string() << '\n';
            return 1;
        }
    }

    for(const std::filesystem::path &input : inputPaths) {
        std::filesystem::path source = std::filesystem::absolute(input).lexically_normal();
        std::error_code error;
        if(!std::filesystem::is_regular_file(source, error) || error) {
            std::cerr << "Input is not a readable file: " << source.u8string() << '\n';
            return 1;
        }
        if(source == absoluteOutputPath || source == absoluteTemporaryPath) {
            std::cerr << "Output source or its temporary file cannot also be an input file: "
                      << source.u8string() << '\n';
            return 1;
        }

        std::string name;
        if(!resourceName(source, root, name)) {
            std::cerr << "Input file is outside the specified root directory: "
                      << source.u8string() << '\n';
            return 1;
        }
        if(!resourceNames.insert(name).second) {
            std::cerr << "Duplicate embedded resource name: " << name << '\n';
            return 1;
        }

        std::uintmax_t size = std::filesystem::file_size(source, error);
        if(error) {
            std::cerr << "Unable to get input file size: " << source.u8string() << '\n';
            return 1;
        }
        files.push_back({source, name, size});
    }

    if(files.empty()) {
        std::cerr << "No resource files were specified.\n";
        return 1;
    }

    std::filesystem::path temporaryPath = outputPath;
    temporaryPath += ".tmp";
    if(!outputPath.parent_path().empty()) {
        std::error_code error;
        std::filesystem::create_directories(outputPath.parent_path(), error);
        if(error) {
            std::cerr << "Unable to create output directory: " << error.message() << '\n';
            return 1;
        }
    }
    std::ofstream output(temporaryPath, std::ios::binary | std::ios::trunc);
    if(!output) {
        std::cerr << "Unable to create output source: " << temporaryPath.u8string() << '\n';
        return 1;
    }

    output << "#include <adapters/handlers/embeddedfilehandler.h>\n\nnamespace {\n";
    for(size_t fileIndex = 0; fileIndex < files.size(); ++fileIndex) {
        std::ifstream input(files[fileIndex].source, std::ios::binary);
        if(!input) {
            std::cerr << "Unable to open input file: " << files[fileIndex].source.u8string() << '\n';
            output.close();
            std::filesystem::remove(temporaryPath);
            return 1;
        }

        output << "static const uint8_t resource" << fileIndex << "[] = {\n";
        size_t byteIndex = 0;
        char byte = 0;
        while(input.get(byte)) {
            if(byteIndex % 16 == 0) {
                output << "    ";
            }
            output << "0x";
            const char digits[] = "0123456789abcdef";
            unsigned char value = static_cast<unsigned char>(byte);
            output.put(digits[value >> 4]);
            output.put(digits[value & 0x0f]);
            output << ',';
            if(byteIndex % 16 == 15) {
                output << '\n';
            } else {
                output << ' ';
            }
            ++byteIndex;
        }
        if(!input.eof() || byteIndex != files[fileIndex].size) {
            std::cerr << "Failed while reading input file: " << files[fileIndex].source.u8string() << '\n';
            output.close();
            std::filesystem::remove(temporaryPath);
            return 1;
        }
        if(byteIndex % 16 != 0) {
            output << '\n';
        }
        if(byteIndex == 0) {
            output << "    0x00,\n";
        }
        output << "};\n";
    }

    output << "\nstatic const EmbeddedFileHandler::EmbeddedFile embeddedFiles[] = {\n";
    for(size_t i = 0; i < files.size(); ++i) {
        output << "    {";
        if(!writeString(output, files[i].resourceName)) {
            break;
        }
        output << ", resource" << i << ", " << files[i].size << "},\n";
    }
    output << "};\n"
              "static EmbeddedFileRegistration registration(embeddedFiles, "
           << files.size() << ");\n"
              "}\n";
    output.close();
    if(!output) {
        std::cerr << "Failed to write output source: " << temporaryPath.u8string() << '\n';
        std::filesystem::remove(temporaryPath);
        return 1;
    }

    std::error_code error;
    std::filesystem::remove(outputPath, error);
    if(error) {
        std::filesystem::remove(temporaryPath);
        std::cerr << "Unable to replace output source " << outputPath.u8string()
                  << ": " << error.message() << '\n';
        return 1;
    }
    std::filesystem::rename(temporaryPath, outputPath, error);
    if(error) {
        std::filesystem::remove(temporaryPath);
        std::cerr << "Unable to move generated source to " << outputPath.u8string()
                  << ": " << error.message() << '\n';
        return 1;
    }

    std::cout << "Generated " << outputPath.u8string() << " with " << files.size()
              << " embedded files.\n";
    return 0;
}
