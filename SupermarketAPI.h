#ifndef SUPERMARKETAPI_H
#define SUPERMARKETAPI_H

#include <cpr/cpr.h>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>

class SupermarketAPI {
private:
    const std::string PAKNSAVE_BASE_URL;
    const std::string COUNTDOWN_BASE_URL;
    const std::string NEWWORLD_BASE_URL;
    const int ITEMS_PER_PAGE;

    cpr::Session countdownClient, paknsaveClient, newWorldClient;

    cpr::Header commonHeaders();
    cpr::Header countdownHeaders();
    cpr::Header paknsaveHeaders();
    cpr::Header newWorldHeaders();

public:
    SupermarketAPI();

    std::vector<std::map<std::string, std::string>> fetchSupermarkets();
    void selectSupermarket(const std::map<std::string, std::string>& supermarket);
    std::vector<std::map<std::string, std::string>> searchProduct(const std::string& term, const std::map<std::string, std::string>& supermarket, int page = 1);
};

#endif // SUPERMARKETAPI_H
