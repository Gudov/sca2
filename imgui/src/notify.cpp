#include "emscripten.h"
#include "notify.hpp"

namespace {

// clang-format off
EM_JS(void, notify, (
	const char* title_c, int title_len,
	const char* body_c, int body_len,
	const char* clip_c, int clip_len),
  { 
	var audio = new Audio('inugami-korone-beep-beep-beep.mp3');
	audio.play();

	title = UTF8ToString(title_c, title_len);
	body = UTF8ToString(body_c, body_len);
	clip = UTF8ToString(clip_c, clip_len);

	function notify(t, b, c) {
		const notification = new Notification(
			t, {
				tag: "test-tag",
				body: b
			}
		);

		notification.onclick = (event) => {
			event.preventDefault(); // prevent the browser from focusing the Notification's tab
			navigator.clipboard.writeText(c);
		};
	}

	if (!("Notification" in window)) {
		alert("This browser does not support desktop notification");
	} else if (Notification.permission === "granted") {
		notify(title, body, clip);
	} else if (Notification.permission !== "denied") {
		Notification.requestPermission().then((permission) => {
			if (permission === "granted") {
				notify(title, body, clip);
			}
		});
	}
	navigator.clipboard.writeText(clip);
  }
);
// clang-format on

}

void send_notify(const msg::Notify &notify) {
    send_notify(notify.label, notify.body, notify.clip);
}

void send_notify(const std::string &title, const std::string &body, const std::string &clip) {
    notify(title.c_str(), title.size(), body.c_str(), body.size(), clip.c_str(), clip.size());
}