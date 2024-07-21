title = UTF8ToString(title_c, title_len)
body = UTF8ToString(body_c, body_len)
clip = UTF8ToString(clip_c, clip_len)

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