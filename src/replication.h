/*
 * Copyright (C) 2015-2018 Dubalu LLC. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#pragma once

#include "xapiand.h"

#ifdef XAPIAND_CLUSTERING

#include <string>

#include "utils.h"

#define SWITCH_TO_REPL '\xfe'

enum class ReplicationMessageType {
	MSG_GET_CHANGESETS,
	MSG_MAX,
};


static inline const std::string& ReplicationMessageTypeNames(ReplicationMessageType type) {
	static const std::string ReplicationMessageTypeNames[] = {
		"MSG_GET_CHANGESETS",
	};
	auto type_int = static_cast<int>(type);
	if (type_int >= 0 || type_int < toUType(ReplicationMessageType::MSG_MAX)) {
		return ReplicationMessageTypeNames[type_int];
	}
	static const std::string UNKNOWN = "ReplicationMessageType::UNKNOWN";
	return UNKNOWN;
}


enum class ReplicationReplyType {
	REPLY_WELCOME,              // Welcome message (same as Remote Protocol's REPLY_UPDATE)
	REPLY_END_OF_CHANGES,       // No more changes to transfer
	REPLY_FAIL,                 // Couldn't generate full set of changes
	REPLY_DB_HEADER,            // The start of a whole DB copy
	REPLY_DB_FILENAME,          // The name of a file in a DB copy
	REPLY_DB_FILEDATA,          // Contents of a file in a DB copy
	REPLY_DB_FOOTER,            // End of a whole DB copy
	REPLY_CHANGESET,            // A changeset file is being sent
	REPLY_MAX,
};


static inline const std::string& ReplicationReplyTypeNames(ReplicationReplyType type) {
	static const std::string ReplicationReplyTypeNames[] = {
		"REPLY_WELCOME",
		"REPLY_END_OF_CHANGES", "REPLY_FAIL", "REPLY_DB_HEADER", "REPLY_DB_FILENAME",
		"REPLY_DB_FILEDATA", "REPLY_DB_FOOTER", "REPLY_CHANGESET",
	};
	auto type_int = static_cast<int>(type);
	if (type_int == SWITCH_TO_REPL) {
		static const std::string SWITCH_TO_REPL_NAME = "SWITCH_TO_REPL";
		return SWITCH_TO_REPL_NAME;
	} else if (type_int >= 0 || type_int < toUType(ReplicationReplyType::REPLY_MAX)) {
		return ReplicationReplyTypeNames[type_int];
	}
	static const std::string UNKNOWN = "ReplicationReplyType::UNKNOWN";
	return UNKNOWN;
}


class BinaryClient;


class Replication {
	BinaryClient* client;

public:
	explicit Replication(BinaryClient* client_);
	~Replication();

	void send_message(ReplicationReplyType type, const std::string& message, double end_time=0.0);

	void replication_server(ReplicationMessageType type, const std::string& message);
	void replication_client(ReplicationReplyType type, const std::string& message);
	void replication_client_file_done();

	void msg_get_changesets(const std::string& message);
	void reply_welcome(const std::string& message);
	void reply_end_of_changes(const std::string& message);
	void reply_fail(const std::string& message);
	void reply_db_header(const std::string& message);
	void reply_db_filename(const std::string& message);
	void reply_db_filedata(const std::string& message);
	void reply_db_footer(const std::string& message);
	void reply_changeset(const std::string& message);
};


#endif  /* XAPIAND_CLUSTERING */
