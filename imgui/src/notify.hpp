#pragma once

#include <string>

#include "messages.hpp"

void send_notify(const std::string& title, const std::string& body, const std::string& clip);
void send_notify(const msg::Notify& notify);

