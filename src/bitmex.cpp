#include "bitmex.h"
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <sstream>

#include <iostream>



bitmex::bitmex(std::string key, std::string secret):auth(key, secret)
{
	apikey = key;
	apisecret = secret;

	blank_vec = {{""}};
	url = "https://www.bitmex.com/api/v1";

}

size_t bitmex::writecallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;   
}

// ------------------------------------------------------------------------------------------------------------------------------ Private Functions

std::string numstr(double x)
{
	std::ostringstream ss;
	ss << x;
	std::string result = ss.str();
	return result;
}

std::string intstr(int x)
{
	std::ostringstream ss;
	ss << x;
	std::string result = ss.str();
	return result;
}

std::string build_msg(std::vector<std::vector<std::string>> params)
{
	std::string result = "{";
	for(unsigned i = 0; i < params.size(); ++i)
	{
		if(params[i][0] == "price" || params[i][0] == "orderQty" || params[i][0] == "leverage" || params[i][0] == "amount")
		{
			result += "\"" + params[i][0] + "\":" + params[i][1] + ",";
		} else {
			result += "\"" + params[i][0] + "\":\"" + params[i][1] + "\",";
		}
	}
	result.pop_back();
	result += "}";

	return result;
}

std::string bitmex::urlComp(std::string base, std::vector<std::vector<std::string>> params)
{
	std::string items = build_msg(params);
	items = auth.url_encode(items);
	return base + "?filter=" + items;
}

// --------------------------------------------------------------------------------------------------------------------------- API Functions


std::string bitmex::Request(std::string VERB, std::string path, std::string body, std::vector<std::vector<std::string>> query)
{
	std::string result;
	std::string timestamp = auth.nonce();

	if(query != blank_vec)
	{
		path = urlComp(path, query);
	}

	std::string payload = VERB + "/api/v1" + path + timestamp + body;
	std::string signature = auth.sha256_hmac(payload);
	
	
	struct curl_slist *httpHeaders = NULL;
	httpHeaders = curl_slist_append(httpHeaders, ("api-nonce: " + timestamp).c_str());
	httpHeaders = curl_slist_append(httpHeaders, ("api-key: " + apikey).c_str());
	httpHeaders = curl_slist_append(httpHeaders, ("api-signature: " + signature).c_str());
	httpHeaders = curl_slist_append(httpHeaders, "User-Agent:api");
	httpHeaders = curl_slist_append(httpHeaders, "Content-Type: Application/JSON");
	httpHeaders = curl_slist_append(httpHeaders, ("Content-Length: " + intstr(body.size())).c_str());

	std::string send_url = url + path;

	if(VERB=="DELETE"){
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
	}

	curl_easy_setopt(curl, CURLOPT_URL, send_url.c_str());

	if(VERB=="POST")
	{
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, NULL);
		curl_easy_setopt(curl, CURLOPT_POST, 1L);
		curl_easy_setopt(curl, CURLOPT_HTTPGET, 0);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
	}
	else if (VERB=="GET")
	{
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, NULL);
		curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
		curl_easy_setopt(curl, CURLOPT_POST, 0);
	}

	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, httpHeaders);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writecallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);

	res = curl_easy_perform(curl);
	curl_slist_free_all(httpHeaders);
	
	return result;
}

std::string bitmex::limit_buy(std::string symbol, double price, int quantity)
{
	std::string result;
	std::string body = build_msg({{"symbol",symbol},{"price",numstr(price)},{"orderQty",intstr(quantity)},{"side","Buy"}});
	result = Request("POST","/order",body,blank_vec);
	return result;
}

std::string bitmex::limit_sell(std::string symbol, double price, int quantity)
{
	std::string result;
	std::string body = build_msg({{"symbol",symbol},{"price",numstr(price)},{"orderQty",intstr(-quantity)},{"side","Sell"}});
	result = Request("POST","/order",body,blank_vec);
	return result;
}

std::string bitmex::bulk_order(std::string symbol, std::vector<double> prices, std::vector<int> qty, std::string side)
{
	std::string result;
	std::string orders = "[";
	for(unsigned i = 0; i < prices.size(); ++i)
	{
		if(side == "Buy"){
			orders += build_msg({{"symbol",symbol},{"price",numstr(prices[i])},{"orderQty",intstr(qty[i])},{"side","Buy"}});
		} else {
			orders += build_msg({{"symbol",symbol},{"price",numstr(prices[i])},{"orderQty",intstr(-qty[i])},{"side","Sell"}});
		}
		orders += ",";	
	}
	orders.pop_back();
	orders += "]";
	orders = "{\"orders\":" + orders + "}";
	
	result = Request("POST","/order/bulk",orders,blank_vec);

	return result;
}

std::string bitmex::cancel_order(std::string orderId)
{
	std::string result;
	std::string body = build_msg({{"orderID",orderId}});
	result = Request("DELETE","/order",body,blank_vec);	
	return result;
}

std::string bitmex::cancel_all(std::string symbol, std::string side)
{
	std::string result;
	std::string body = build_msg({{"symbol",symbol}});	

	if(side != "")
	{
		result = Request("DELETE","/order/all",body,{{"side",side}});
	} else {
		result = Request("DELETE","/order/all",body,blank_vec);
	}

	return result;
}

std::string bitmex::leverage(std::string symbol, double leverage)
{
	std::string result;
	std::string body = build_msg({{"symbol",symbol},{"leverage",numstr(leverage)}});
	result = Request("POST","/position/leverage",body,blank_vec);
	return result;
}

std::string bitmex::chat(std::string message)
{
	std::string result;
	std::string body = build_msg({{"message",message}});
	result = Request("POST","/chat",body,blank_vec);
	return result;
}

std::string bitmex::account(std::string symbol)
{
	std::string result;
	std::string path = "/user/margin?currency=" + symbol;
	result = Request("GET",path,"",blank_vec);
	return result;
}

std::string bitmex::withdraw(std::string symbol, std::string address, double amount)
{
	std::string result;
	std::string body = build_msg({{"currency",symbol},{"amount",numstr(amount)},{"address",address}});
	result = Request("POST","/user/requestWithdrawal",body,blank_vec);
	return result;
}

