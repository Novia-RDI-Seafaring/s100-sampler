#include "Curler.hpp"

#include <QTemporaryDir>
#include <cppcodec/base64_rfc4648.hpp>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace fs = std::filesystem;

static std::string readFile(const fs::path &path)
{
    std::ifstream input(path, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(input), {});
}

static std::string responseForXml(const std::string &xml)
{
    return nlohmann::json{
        {"dataResponseObject", nlohmann::json::array({
             {{"data", cppcodec::base64_rfc4648::encode(xml)}}})}}
        .dump();
}

static std::string responseFor(const std::string &identifier)
{
    return responseForXml("<Dataset xmlns:gml=\"http://www.opengis.net/gml/3.2\" gml:id=\"" +
                          identifier + "\"><marker>download</marker></Dataset>");
}

static void unsafeIdentifiers()
{
    QTemporaryDir temporary;
    if (!temporary.isValid())
        throw std::runtime_error("Cannot create disposable test directory");
    const fs::path root(temporary.path().toStdString());
    const auto datasets = root / "datasets";
    fs::create_directory(datasets);
    const auto outside = root / "outside.xml";
    { std::ofstream output(outside); output << "original"; }

    for (const std::string identifier : {
             "../outside", "..\\outside", "/outside", "C:\\outside", "C:outside",
             "\\\\server\\share\\outside", "nested/file", "nested\\file", ".", "..",
             "name..extension", "%2e%2e%2foutside", "name:stream", "name with spaces",
             "CON", "con.anything", "NUL", "PRN", "AUX", "COM1", "lpt9"})
    {
        bool rejected = false;
        try
        {
            Curler::saveAndDecodeRawJSONToFiles(responseFor(identifier), datasets.string());
        }
        catch (const std::runtime_error &)
        {
            rejected = true;
        }
        if (!rejected || !fs::is_empty(datasets) || readFile(outside) != "original")
            throw std::runtime_error("Unsafe dataset identifier was not rejected: " + identifier);
    }
}

static void normalDownloads()
{
    QTemporaryDir temporary;
    if (!temporary.isValid())
        throw std::runtime_error("Cannot create disposable test directory");
    const fs::path datasets(temporary.path().toStdString());
    for (const std::string identifier : {"124CA01M_1286_26", "dataset-01.2", "0a91-42b5"})
    {
        Curler::saveAndDecodeRawJSONToFiles(responseFor(identifier), datasets.string());
        if (readFile(datasets / (identifier + ".xml")).find("<marker>download</marker>") == std::string::npos)
            throw std::runtime_error("Normal dataset did not download");
    }
    // Updating the same safe identifier remains supported.
    Curler::saveAndDecodeRawJSONToFiles(responseFor("dataset-01.2"), datasets.string());
    if (std::distance(fs::directory_iterator(datasets), fs::directory_iterator{}) != 3)
        throw std::runtime_error("Updating a dataset changed its filename");

    Curler::saveAndDecodeRawJSONToFiles(responseForXml("<Dataset><marker>unnamed</marker></Dataset>"),
                                      datasets.string());
    if (readFile(datasets / "output_0.xml").find("unnamed") == std::string::npos)
        throw std::runtime_error("Missing identifier fallback no longer works");
}

static int symlinkDestination()
{
    QTemporaryDir temporary;
    if (!temporary.isValid())
        throw std::runtime_error("Cannot create disposable test directory");
    const fs::path root(temporary.path().toStdString());
    const auto datasets = root / "datasets";
    fs::create_directory(datasets);
    const auto outside = root / "outside.xml";
    { std::ofstream output(outside); output << "original"; }
    std::error_code error;
    fs::create_symlink(outside, datasets / "safe.xml", error);
    if (error)
    {
        std::cout << "SKIP: this account cannot create file symlinks: " << error.message() << '\n';
        return 77;
    }
    bool rejected = false;
    try
    {
        Curler::saveAndDecodeRawJSONToFiles(responseFor("safe"), datasets.string());
    }
    catch (const std::runtime_error &)
    {
        rejected = true;
    }
    if (!rejected || readFile(outside) != "original")
        throw std::runtime_error("Download followed an existing symlink outside the dataset directory");
    return 0;
}

static void parentTraversal()
{
    QTemporaryDir temporary;
    if (!temporary.isValid())
        throw std::runtime_error("Cannot create disposable test directory");
    const fs::path root(temporary.path().toStdString());
    const auto datasets = root / "datasets";
    fs::create_directory(datasets);
    const auto outside = root / "outside.xml";
    { std::ofstream output(outside); output << "original"; }

    bool rejected = false;
    try
    {
        Curler::saveAndDecodeRawJSONToFiles(responseFor("../outside"), datasets.generic_string() + "/");
    }
    catch (const std::runtime_error &)
    {
        rejected = true;
    }
    if (!rejected || readFile(outside) != "original" || !fs::is_empty(datasets))
        throw std::runtime_error("A downloaded identifier escaped the dataset directory");
}

int main(int argc, char **argv)
{
    try
    {
        if (argc != 2)
            throw std::runtime_error("Unknown test case");
        const std::string test(argv[1]);
        if (test == "parent-traversal")
            parentTraversal();
        else if (test == "unsafe-identifiers")
            unsafeIdentifiers();
        else if (test == "normal-downloads")
            normalDownloads();
        else if (test == "symlink-destination")
            return symlinkDestination();
        else
            throw std::runtime_error("Unknown test case");
        return 0;
    }
    catch (const std::exception &error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
