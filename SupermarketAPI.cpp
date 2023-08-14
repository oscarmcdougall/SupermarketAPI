#include "SupermarketAPI.h"
#include <iostream>

SupermarketAPI::SupermarketAPI():
	PAKNSAVE_BASE_URL("https://www.paknsave.co.nz"),
	COUNTDOWN_BASE_URL("https://www.countdown.co.nz"),
	NEWWORLD_BASE_URL("https://www.newworld.co.nz/"),
	ITEMS_PER_PAGE(60)
{
	// Initializing the clients with the cookies
	countdownClient = cpr::Session();
	countdownClient.SetHeader(countdownHeaders());

	paknsaveClient = cpr::Session();
	paknsaveClient.SetHeader(paknsaveHeaders());

	newWorldClient = cpr::Session();
	newWorldClient.SetHeader(newWorldHeaders());
}

cpr::Header SupermarketAPI::commonHeaders() const
{
	cpr::Header headers;
	headers["accept"] = "application/json, text/plain, */*";
	headers["accept-language"] = "en-GB,en-US;q=0.9,en;q=0.8";
	headers["sec-ch-ua"] = "\"Not/A)Brand\";v=\"99\", \"Google Chrome\";v=\"115\", \"Chromium\";v=\"115\"";
	headers["sec-ch-ua-mobile"] = "?0";
	headers["sec-ch-ua-platform"] = "\"macOS\"";
	headers["sec-fetch-dest"] = "empty";
	headers["sec-fetch-mode"] = "cors";
	headers["sec-fetch-site"] = "same-origin";
	return headers;
}

cpr::Header SupermarketAPI::countdownHeaders() const
{
	cpr::Header headers = commonHeaders();
	headers["cache-control"] = "no-cache";
	headers["content-type"] = "application/json";
	headers["expires"] = "Sat, 01 Jan 2000 00:00:00 GMT";
	headers["pragma"] = "no-cache";
	headers["x-requested-with"] = "OnlineShopping.WebApp";
	headers["x-ui-ver"] = "7.21.138";
	return headers;
}

cpr::Header SupermarketAPI::paknsaveHeaders() const
{
	cpr::Header headers = commonHeaders();
	headers["referrer"] = PAKNSAVE_BASE_URL;
	headers["referrerPolicy"] = "no-referrer-when-downgrade";
	headers["user-agent"] = "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/115.0.0.0 Safari/537.36";
	return headers;
}

cpr::Header SupermarketAPI::newWorldHeaders() const
{
	cpr::Header headers = commonHeaders();
	headers["referrer"] = NEWWORLD_BASE_URL;
	headers["referrerPolicy"] = "no-referrer-when-downgrade";
	headers["user-agent"] = "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/115.0.0.0 Safari/537.36";
	return headers;
}

std::vector<std::map<std::string, std::string>> SupermarketAPI::fetchSupermarkets() const
{
	std::vector<std::map<std::string, std::string>> supermarkets;

	// Fetching for Countdown
	auto countdownResponse = countdownClient.Get(cpr::Url { COUNTDOWN_BASE_URL + "/api/v1/addresses/pickup-addresses" });
	auto countdownData = nlohmann::json::parse(countdownResponse.text);
	for (const auto &store: countdownData["storeAreas"][0]["storeAddresses"])
	{
		supermarkets.push_back({ { "name", store["name"] }, { "type", "Countdown" } });
	}

	// Fetching for Pak'nSave
	auto paknsaveResponse = paknsaveClient.Post(cpr::Url { PAKNSAVE_BASE_URL + "/CommonApi/Store/GetStoreList" });
	auto paknsaveData = nlohmann::json::parse(paknsaveResponse.text);
	for (const auto &store: paknsaveData["stores"])
	{
		supermarkets.push_back({ { "name", store["name"] }, { "type", "Pak'nSave" } });
	}

	// Fetching for New World
	auto newWorldResponse = newWorldClient.Post(cpr::Url { NEWWORLD_BASE_URL + "/CommonApi/Store/GetStoreList" });
	auto newWorldData = nlohmann::json::parse(newWorldResponse.text);
	for (const auto &store: newWorldData["stores"])
	{
		supermarkets.push_back({ { "name", store["name"] }, { "type", "New World" } });
	}

	return supermarkets;
}

void SupermarketAPI::selectSupermarket(const std::map<std::string, std::string>& supermarket)
{
	if (supermarket.at("type") == "Countdown")
	{
		countdownClient.Put(cpr::Url { COUNTDOWN_BASE_URL + "/api/v1/fulfilment/my/pickup-addresses" }, cpr::Body { "addressId=" + supermarket.at("id") });
	}
	else if (supermarket.at("type") == "Pak'nSave")
	{
		paknsaveClient.Post(cpr::Url { PAKNSAVE_BASE_URL + "/CommonApi/Store/ChangeStore?storeId=" + supermarket.at("id") + "&clickSource=list" });
	}
	else if (supermarket.at("type") == "New World")
	{
		newWorldClient.Post(cpr::Url { NEWWORLD_BASE_URL + "/CommonApi/Store/ChangeStore?storeId=" + supermarket.at("id") + "&clickSource=list" });
	}
}

std::vector<std::map<std::string, std::string>> SupermarketAPI::searchProduct(const std::string &term, const std::map<std::string, std::string>& supermarket, int page) const
{
	std::vector<std::map<std::string, std::string>> products;

	if (supermarket.at("type") == "Countdown")
	{
		auto response = countdownClient.Get(cpr::Url { COUNTDOWN_BASE_URL + "/api/v1/products?target=search&search=" + term + "&page=" + std::to_string(page) + "&inStockProductsOnly=false&size=" + std::to_string(ITEMS_PER_PAGE) });
		auto productData = nlohmann::json::parse(response.text);
		for (const auto &product: productData["products"]["items"])
			products.push_back({ { "name", product["name"] }, { "price", product["price"]["salePrice"] } });
	}
	else if (supermarket.at("type") == "Pak'nSave")
	{
		auto response = paknsaveClient.Get(cpr::Url { PAKNSAVE_BASE_URL + "/next/api/products/search?q=" + term + "&s=popularity&pg=" + std::to_string(page) + "&storeId=" + supermarket.at("id") + "&publish=true&ps=" + std::to_string(ITEMS_PER_PAGE) });
		auto productData = nlohmann::json::parse(response.text);
		for (const auto &product: productData["data"]["products"])
			products.push_back({ { "name", product["name"] }, { "price", std::to_string(product["price"] / 100.0) } });
	}
	else if (supermarket.at("type") == "New World")
	{
		auto response = newWorldClient.Get(cpr::Url { NEWWORLD_BASE_URL + "/next/api/products/search?q=" + term + "&s=popularity&pg=" + std::to_string(page) + "&storeId=" + supermarket.at("id") + "&publish=true&ps=" + std::to_string(ITEMS_PER_PAGE) });
		auto productData = nlohmann::json::parse(response.text);
		for (const auto &product: productData["data"]["products"])
			products.push_back({ { "name", product["name"] }, { "price", std::to_string(product["price"] / 100.0) } });
	}

	return products;
}
