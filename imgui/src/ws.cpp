#include "ws.hpp"

#include <emscripten.h>
#include <emscripten/websocket.h>

#include "config.hpp"
#include <cereal/archives/json.hpp>

EMSCRIPTEN_WEBSOCKET_T ws;

std::mutex ws_queue_mutex;
std::queue<std::string> ws_queue;

static bool ws_connected;

EM_BOOL onopen(int eventType, const EmscriptenWebSocketOpenEvent *websocketEvent, void *userData) {
    puts("ws: onopen");
    ws_connected = true;
    return EM_TRUE;
}
EM_BOOL onerror(int eventType, const EmscriptenWebSocketErrorEvent *websocketEvent, void *userData) {
    puts("ws: onerror");
    return EM_TRUE;
}
EM_BOOL onclose(int eventType, const EmscriptenWebSocketCloseEvent *websocketEvent, void *userData) {
    puts("ws: onclose");
    return EM_TRUE;
}
EM_BOOL onmessage(int eventType, const EmscriptenWebSocketMessageEvent *event, void *userData) {
	std::string message((const char*)event->data, (size_t)event->numBytes);
	if (debug) {
		printf("message: %s\n", message.c_str());
	}
	{
		std::lock_guard guard(ws_queue_mutex);
		ws_queue.push(message);
	}
    return EM_TRUE;
}

void sendRequest(msg::Request &&request) {
	std::stringstream ss;
	{
		cereal::JSONOutputArchive archive(ss);
		archive(request);
	}
	std::string str = ss.str();
	if (debug) {
		printf("request: %s\n", str.c_str());
	}
	emscripten_websocket_send_binary(ws, (void*)str.c_str(), str.size());
}

void connect_to_ws(const std::string &url) {
	EmscriptenWebSocketCreateAttributes ws_attrs = {
		url.c_str(),
		NULL,
		EM_TRUE
	};

	ws = emscripten_websocket_new(&ws_attrs);
	emscripten_websocket_set_onopen_callback(ws, NULL, onopen);
	emscripten_websocket_set_onerror_callback(ws, NULL, onerror);
	emscripten_websocket_set_onclose_callback(ws, NULL, onclose);
	emscripten_websocket_set_onmessage_callback(ws, NULL, onmessage);
}

bool is_connected() {
    return ws_connected;
}