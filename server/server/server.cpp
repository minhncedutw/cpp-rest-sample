// server.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>

#include <boost/format.hpp>
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <cpprest/base_uri.h>
//#pragma comment(lib, "cpprest_2_10")

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

#include <iostream>
#include <map>
#include <set>
#include <string>
using namespace std;

#define TRACE(msg)            wcout << msg
#define TRACE_ACTION(a, k, v) wcout << a << L" (" << k << L", " << v << L")\n"

map<utility::string_t, utility::string_t> dictionary;

void display_json(
	json::value const & jvalue,
	utility::string_t const & prefix)
{
	wcout << prefix << jvalue.serialize() << endl;
}

void handle_get(http_request request)
{
	TRACE(L"\nhandle GET\n");

	auto paths = http::uri::split_path(http::uri::decode(request.relative_uri().path()));

	switch (paths.size())
	{
	case 1:
		if (paths[0] == L"restdemo")
		{
			auto answer = json::value::object();

			for (auto const & p : dictionary)
			{
				answer[p.first] = json::value::string(p.second);
			}

			display_json(json::value::null(), L"R: ");
			display_json(answer, L"S: ");

			request.reply(status_codes::OK, answer);
		}
		else
		{
			request.reply(status_codes::NotFound);
		}
		break;

	default:
		request.reply(status_codes::NotFound);
		break;
	}
}

void handle_request(
	http_request request,
	function<void(json::value const &, json::value &)> action)
{
	auto answer = json::value::object();

	request
		.extract_json()
		.then([&answer, &action](pplx::task<json::value> task) {
		try
		{
			auto const & jvalue = task.get();
			display_json(jvalue, L"R: ");

			if (!jvalue.is_null())
			{
				action(jvalue, answer);
			}
		}
		catch (http_exception const & e)
		{
			wcout << e.what() << endl;
		}
	})
		.wait();


	display_json(answer, L"S: ");

	request.reply(status_codes::OK, answer);
}

void handle_post(http_request request)
{
	TRACE("\nhandle POST\n");

	auto paths = http::uri::split_path(http::uri::decode(request.relative_uri().path()));

	switch (paths.size())
	{
	case 1:
		if (paths[0] == L"restdemo")
		{
			handle_request(
				request,
				[](json::value const & jvalue, json::value & answer)
			{
				for (auto const & e : jvalue.as_array())
				{
					if (e.is_string())
					{
						auto key = e.as_string();
						auto pos = dictionary.find(key);

						if (pos == dictionary.end())
						{
							answer[key] = json::value::string(L"<nil>");
						}
						else
						{
							answer[pos->first] = json::value::string(pos->second);
						}
					}
				}
			});
		}
		else
		{
			request.reply(status_codes::NotFound);
		}
		break;

	default:
		request.reply(status_codes::NotFound);
		break;
	}
}

void handle_put(http_request request)
{
	TRACE("\nhandle PUT\n");

	//ucout << endl << "******************************" << endl;
	////ucout << "aaa " << request.to_string() << endl;
	auto paths = http::uri::split_path(http::uri::decode(request.relative_uri().path()));
	//for (auto const & path : paths)
	//{
	//	ucout << path << " | ";
	//}
	//ucout << endl << "******************************" << endl;
	switch (paths.size())
	{
	case 1:
		if (paths[0] == L"restdemo")
		{
			handle_request(
				request,
				[](json::value const & jvalue, json::value & answer)
			{
				for (auto const & e : jvalue.as_object())
				{
					if (e.second.is_string())
					{
						auto key = e.first;
						auto value = e.second.as_string();

						if (dictionary.find(key) == dictionary.end())
						{
							TRACE_ACTION(L"added", key, value);
							answer[key] = json::value::string(L"<put>");
						}
						else
						{
							TRACE_ACTION(L"updated", key, value);
							answer[key] = json::value::string(L"<updated>");
						}

						dictionary[key] = value;
					}
				}
			});
		}
		else
		{
			request.reply(status_codes::NotFound);
		}
		break;

	default:
		request.reply(status_codes::NotFound);
		break;
	}
}

void handle_del(http_request request)
{
	TRACE("\nhandle DEL\n");

	auto paths = http::uri::split_path(http::uri::decode(request.relative_uri().path()));

	switch (paths.size())
	{
	case 1:
		if (paths[0] == L"restdemo")
		{
			handle_request(
				request,
				[](json::value const & jvalue, json::value & answer)
			{
				set<utility::string_t> keys;
				for (auto const & e : jvalue.as_array())
				{
					if (e.is_string())
					{
						auto key = e.as_string();

						auto pos = dictionary.find(key);
						if (pos == dictionary.end())
						{
							answer[key] = json::value::string(L"<failed>");
						}
						else
						{
							TRACE_ACTION(L"deleted", pos->first, pos->second);
							answer[key] = json::value::string(L"<deleted>");
							keys.insert(key);
						}
					}
				}

				for (auto const & key : keys)
					dictionary.erase(key);
			});
		}
		else
		{
			request.reply(status_codes::NotFound);
		}
		break;

	default:
		request.reply(status_codes::NotFound);
		break;
	}
}

int main()
{
    std::cout << "Hello World! \nThis is server."; 

	utility::string_t url = L"http://localhost:5000/v1";
	http_listener listener(url);

	listener.support(methods::GET, handle_get);
	listener.support(methods::POST, handle_post);
	listener.support(methods::PUT, handle_put);
	listener.support(methods::DEL, handle_del);

	try
	{
		listener
			.open()
			.then([&listener]() {TRACE(L"\nstarting to listen\n"); })
			.wait();

		while (true);
	}
	catch (exception const & e)
	{
		wcout << e.what() << endl;
	}

	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
