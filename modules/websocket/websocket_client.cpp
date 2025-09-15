/*************************************************************************/
/*  websocket_client.cpp                                                 */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "websocket_client.h"

GDCINULL(WebSocketClient);

WebSocketClient::WebSocketClient() {
	verify_ssl = true;
}

WebSocketClient::~WebSocketClient() {
}

Error WebSocketClient::connect_to_url(String p_url, const Vector<String> p_protocols, bool gd_mp_api, const Vector<String> p_custom_headers) {
	_is_multiplayer = gd_mp_api;

	String host = p_url;
	String path;
	String scheme;
	int port = 0;
	Error err = p_url.parse_url(scheme, host, port, path);
	ERR_FAIL_COND_V_MSG(err != OK, err, "Invalid URL: " + p_url);

	bool ssl = false;
	if (scheme == "wss://") {
		ssl = true;
	}
	if (port == 0) {
		port = ssl ? 443 : 80;
	}
	if (path.empty()) {
		path = "/";
	}
	return connect_to_host(host, path, port, ssl, p_protocols, p_custom_headers);
}

void WebSocketClient::set_verify_ssl_enabled(bool p_verify_ssl) {
	verify_ssl = p_verify_ssl;
}

bool WebSocketClient::is_verify_ssl_enabled() const {
	return verify_ssl;
}

Ref<X509Certificate> WebSocketClient::get_trusted_ssl_certificate() const {
	return ssl_cert;
}

void WebSocketClient::set_trusted_ssl_certificate(Ref<X509Certificate> p_cert) {
	ERR_FAIL_COND(get_connection_status() != CONNECTION_DISCONNECTED);
	ssl_cert = p_cert;
}

bool WebSocketClient::is_server() const {
	return false;
}

void WebSocketClient::set_compression_enabled(bool p_compression_enabled) {
	compression_enabled = p_compression_enabled;
}

bool WebSocketClient::is_compression_enabled() const {
	return compression_enabled;
}

void WebSocketClient::set_server_no_context_takeover(bool p_server_no_context_takeover) {
	server_no_context_takeover = p_server_no_context_takeover;
}

bool WebSocketClient::is_server_no_context_takeover() const {
	return server_no_context_takeover;
}

void WebSocketClient::set_client_no_context_takeover(bool p_client_no_context_takeover) {
	client_no_context_takeover = p_client_no_context_takeover;
}

bool WebSocketClient::is_client_no_context_takeover() const {
	return client_no_context_takeover;
}

void WebSocketClient::set_server_max_window_bits(int p_server_max_window_bits) {
	server_max_window_bits = p_server_max_window_bits;
}

int WebSocketClient::get_server_max_window_bits() const {
	return server_max_window_bits;
}

void WebSocketClient::set_client_max_window_bits(int p_client_max_window_bits) {
	client_max_window_bits = p_client_max_window_bits;
}

int WebSocketClient::get_client_max_window_bits() const {
	return client_max_window_bits;
}

void WebSocketClient::_on_peer_packet() {
	if (_is_multiplayer) {
		_process_multiplayer(get_peer(1), 1);
	} else {
		emit_signal("data_received");
	}
}

void WebSocketClient::_on_connect(String p_protocol) {
	if (_is_multiplayer) {
		// need to wait for ID confirmation...
	} else {
		emit_signal("connection_established", p_protocol);
	}
}

void WebSocketClient::_on_close_request(int p_code, String p_reason) {
	emit_signal("server_close_request", p_code, p_reason);
}

void WebSocketClient::_on_disconnect(bool p_was_clean) {
	if (_is_multiplayer) {
		emit_signal("connection_failed");
	} else {
		emit_signal("connection_closed", p_was_clean);
	}
}

void WebSocketClient::_on_error() {
	if (_is_multiplayer) {
		emit_signal("connection_failed");
	} else {
		emit_signal("connection_error");
	}
}

void WebSocketClient::_bind_methods() {
	ClassDB::bind_method(D_METHOD("connect_to_url", "url", "protocols", "gd_mp_api", "custom_headers"), &WebSocketClient::connect_to_url, DEFVAL(Vector<String>()), DEFVAL(false), DEFVAL(Vector<String>()));
	ClassDB::bind_method(D_METHOD("disconnect_from_host", "code", "reason"), &WebSocketClient::disconnect_from_host, DEFVAL(1000), DEFVAL(""));
	ClassDB::bind_method(D_METHOD("get_connected_host"), &WebSocketClient::get_connected_host);
	ClassDB::bind_method(D_METHOD("get_connected_port"), &WebSocketClient::get_connected_port);
	ClassDB::bind_method(D_METHOD("set_verify_ssl_enabled", "enabled"), &WebSocketClient::set_verify_ssl_enabled);
	ClassDB::bind_method(D_METHOD("is_verify_ssl_enabled"), &WebSocketClient::is_verify_ssl_enabled);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "verify_ssl", PROPERTY_HINT_NONE, "", 0), "set_verify_ssl_enabled", "is_verify_ssl_enabled");

	ClassDB::bind_method(D_METHOD("get_trusted_ssl_certificate"), &WebSocketClient::get_trusted_ssl_certificate);
	ClassDB::bind_method(D_METHOD("set_trusted_ssl_certificate", "certificate"), &WebSocketClient::set_trusted_ssl_certificate);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "trusted_ssl_certificate", PROPERTY_HINT_RESOURCE_TYPE, "X509Certificate", 0), "set_trusted_ssl_certificate", "get_trusted_ssl_certificate");

	ClassDB::bind_method(D_METHOD("set_compression_enabled", "enabled"), &WebSocketClient::set_compression_enabled);
	ClassDB::bind_method(D_METHOD("is_compression_enabled"), &WebSocketClient::is_compression_enabled);
	ClassDB::bind_method(D_METHOD("set_server_no_context_takeover", "enabled"), &WebSocketClient::set_server_no_context_takeover);
	ClassDB::bind_method(D_METHOD("is_server_no_context_takeover"), &WebSocketClient::is_server_no_context_takeover);
	ClassDB::bind_method(D_METHOD("set_client_no_context_takeover", "enabled"), &WebSocketClient::set_client_no_context_takeover);
	ClassDB::bind_method(D_METHOD("is_client_no_context_takeover"), &WebSocketClient::is_client_no_context_takeover);
	ClassDB::bind_method(D_METHOD("set_server_max_window_bits", "max_window_bits"), &WebSocketClient::set_server_max_window_bits);
	ClassDB::bind_method(D_METHOD("get_server_max_window_bits"), &WebSocketClient::get_server_max_window_bits);
	ClassDB::bind_method(D_METHOD("set_client_max_window_bits", "max_window_bits"), &WebSocketClient::set_client_max_window_bits);
	ClassDB::bind_method(D_METHOD("get_client_max_window_bits"), &WebSocketClient::get_client_max_window_bits);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "compression_enabled"), "set_compression_enabled", "is_compression_enabled");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "server_no_context_takeover"), "set_server_no_context_takeover", "is_server_no_context_takeover");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "client_no_context_takeover"), "set_client_no_context_takeover", "is_client_no_context_takeover");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "server_max_window_bits"), "set_server_max_window_bits", "get_server_max_window_bits");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "client_max_window_bits"), "set_client_max_window_bits", "get_client_max_window_bits");

	ADD_SIGNAL(MethodInfo("data_received"));
	ADD_SIGNAL(MethodInfo("connection_established", PropertyInfo(Variant::STRING, "protocol")));
	ADD_SIGNAL(MethodInfo("server_close_request", PropertyInfo(Variant::INT, "code"), PropertyInfo(Variant::STRING, "reason")));
	ADD_SIGNAL(MethodInfo("connection_closed", PropertyInfo(Variant::BOOL, "was_clean_close")));
	ADD_SIGNAL(MethodInfo("connection_error"));
}
