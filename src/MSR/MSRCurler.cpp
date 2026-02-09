#include "MSRCurler.hpp"

MSRCurler::MSRCurler()
{
}

json MSRCurler::getServiceList(const S100Provider &provider, int secomVersion = 1) const
{

    std::string msrUrl = provider.endpoint.toString().toStdString();
    if (secomVersion == 1)
    {
        msrUrl = msrUrl + "/v1/searchService";
    }
    else if (secomVersion == 2)
    {
        msrUrl = msrUrl + "/v2/ServiceRegistry";
    }
    else
    {
        std::cerr << "SECOM VERSION NOT SUPPORTED IN MSR" << '\n';
    }
    json payload = {{"query", json::object()}};

    HttpResponse response;
    try
    {
        response = Curler::postJson(msrUrl, payload.dump());
        if (response.statusCode != 200)
        {
            std::string errormessage = "Error in MSR: " + response.statusCode + ' ' + response.body;
            throw std::runtime_error(errormessage);
        }
    }
    catch (const std::exception &e)
    {
        // Log error if needed
        std::cerr << "Error fetching MSR list: " << e.what() << std::endl;
    }

    json parsed = json::parse(response.body);

    if (parsed.contains("searchServiceResult") && parsed["searchServiceResult"].is_array())
    {
        for (auto &item : parsed["searchServiceResult"])
        {
            if (item.is_object())
            {
                if (!item.contains("organizationId") || item["organizationId"].is_null() ||
                    (item["organizationId"].is_string() && item["organizationId"].get<std::string>().empty()))
                {
                    item["organizationId"] = "NO ORG ID";
                }
            }
        }
    }

    return parsed;
}

json MSRCurler::getServiceList(const S100Provider &provider)
{
    const auto &cfg = ConfigManager::getInstance().getConfig();
    const auto &certOpt = cfg.cert;
    const auto &keyOpt = cfg.key;

    auto response = Curler::getJson(provider.endpoint.toString().toStdString(), certOpt, keyOpt);

    if (response.statusCode == 200)
    {
        return response.body;
    }
    return "";
}

bool MSRCurler::pingEndpoint(const QUrl endpoint)
{
    std::string url = endpoint.toString().toStdString() + "/v1/ping";
    HttpResponse response;
    try
    {
        response = Curler::getJson(url);
        if (response.statusCode == 200)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    catch (...)
    {
        std::cerr << "Bruh" << '\n';
    }
    return false;
}
