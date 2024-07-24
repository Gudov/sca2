#pragma once

#include <mutex>
#include <queue>

#include "messages.hpp"

extern std::mutex ws_queue_mutex;
extern std::queue<std::string> ws_queue;

void sendRequest(msg::Request&& request);
void connect_to_ws(const std::string& url);
bool is_connected();

enum WsStatus {
	connecting,
	connected,
	error,
	closed,
	empty
};

WsStatus get_ws_status();
