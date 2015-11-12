/*
 * Copyright (C) 2015 deipi.com LLC and contributors. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "discovery.h"

#include <assert.h>


Discovery::Discovery(const std::shared_ptr<XapiandManager>& manager_, ev::loop_ref *loop_, int port_, const std::string &group_)
	: BaseUDP(manager_, loop_, port_, "Discovery", group_),
	  heartbeat(*loop)
{
	heartbeat.set<Discovery, &Discovery::heartbeat_cb>(this);
	heartbeat.repeat = random_real(HEARTBEAT_MIN, HEARTBEAT_MAX);
	LOG_DISCOVERY(this, "\tSet heartbeat timeout event %f\n", heartbeat.repeat);

	LOG_OBJ(this, "CREATED DISCOVERY\n");
}


Discovery::~Discovery()
{
	heartbeat.stop();

	LOG_OBJ(this, "DELETED DISCOVERY\n");
}


void
Discovery::heartbeat_cb(ev::timer &, int)
{
	LOG_EV(this, "Discovery::heartbeat_cb:BEGIN\n");
	switch (manager->state) {
		case XapiandManager::State::RESET:
			if (!local_node.name.empty()) {
				manager->drop_node(local_node.name);
			}
			if (manager->node_name.empty()) {
				local_node.name = name_generator();
			} else {
				local_node.name = manager->node_name;
			}
			INFO(this, "Advertising as %s (id: %016llX)...\n", local_node.name.c_str(), local_node.id);
			send_message(Message::HELLO, local_node.serialise());
			manager->state = XapiandManager::State::WAITING;
			break;

		case XapiandManager::State::WAITING:
			manager->state = XapiandManager::State::WAITING_;
			break;

		case XapiandManager::State::WAITING_:
			manager->state = XapiandManager::State::SETUP;
			break;

		case XapiandManager::State::SETUP:
			manager->setup_node();
			break;

		case XapiandManager::State::READY:
			send_message(Message::HEARTBEAT, local_node.serialise());
			break;

		case XapiandManager::State::BAD:
			LOG_ERR(this, "ERROR: Manager is in BAD state!!\n");
			break;
	}
	LOG_EV(this, "Discovery::heartbeat_cb:END\n");
}


void
Discovery::send_message(Message type, const std::string &content)
{
	if (!content.empty()) {
		std::string message(1, toUType(type));
		message.append(std::string((const char *)&XAPIAND_DISCOVERY_PROTOCOL_VERSION, sizeof(uint16_t)));
		message.append(serialise_string(manager->cluster_name));
		message.append(content);
		sending_message(message);
	}
}


std::string
Discovery::getDescription() const noexcept
{
	return "UDP:" + std::to_string(port) + " (" + description + " v" + std::to_string(XAPIAND_DISCOVERY_PROTOCOL_MAJOR_VERSION) + "." + std::to_string(XAPIAND_DISCOVERY_PROTOCOL_MINOR_VERSION) + ")";
}
