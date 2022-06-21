#include <uwebsockets/App.h>
#include <iostream>
#include <ranges>

#include <nlohmann/json.hpp>
using json = nlohmann::json;


/* ws->getUserData returns one of these */
struct PerSocketData {
	bool in_match = false;
	int match_id;
	bool color;
	static inline int match_id_gen = 0;
};

using WS = uWS::WebSocket<false, true, PerSocketData>;

int main()
{


	/* Keep in mind that uWS::SSLApp({options}) is the same as uWS::App() when compiled without SSL support.
	 * You may swap to using uWS:App() if you don't need SSL */
	auto app = new uWS::App();
	app->ws<PerSocketData>("/*", {
		/* Settings */
		//.compression = uWS::CompressOptions(uWS::DEDICATED_COMPRESSOR_4KB | uWS::DEDICATED_DECOMPRESSOR),
		.maxPayloadLength = 100 * 1024 * 1024,
		.idleTimeout = 16,
		.maxBackpressure = 100 * 1024 * 1024,
		.closeOnBackpressureLimit = false,
		.resetIdleTimeoutOnSend = false,
		.sendPingsAutomatically = true,
		/* Handlers */
		.upgrade = nullptr,
		.open = [&app](WS* ws) {
			std::cout << "connected\n";

			ws->subscribe("broadcast");
			ws->subscribe("matchmaking");
			//ws->send("hello");

			const auto subscribers_n = app->numSubscribers("matchmaking");
			if (subscribers_n == 2) {
				std::cout << "matched\n";

				const auto match_id = PerSocketData::match_id_gen++;

				const auto topic = app->topicTree->lookupTopic("matchmaking");
				const auto match_id_str = std::to_string(match_id);

				for (int i = 0; i < 2; ++i) {
					const auto sub_ws = (WS*)(*topic->begin())->user;
					const auto data = sub_ws->getUserData();
					data->in_match = true;
					data->match_id = match_id;
					data->color = i;

	/*				const auto str = json{
					{"color", i == 0 ? "white" : "black"},
					{"match_id", match_id_str}
					}.dump();*/

					std::string str = "S";
					str += i == 0 ? "W" : "B";

					sub_ws->send(str, uWS::OpCode::TEXT);

					sub_ws->subscribe(match_id_str);
					sub_ws->unsubscribe("matchmaking");
				}
				
				//app->publish(match_id_str, "started" + match_id_str, uWS::OpCode::TEXT);
			}
			else {
				std::cout << "waiting for second\n";
				ws->send("wait", uWS::OpCode::TEXT);
			}
		},
		.message = [](WS* ws, std::string_view message, uWS::OpCode opCode) {
			std::cout << "received: " << message << '\n';
			ws->publish("broadcast", message);
			//const auto data = ws->getUserData();
			//if (not data->in_match)
			//	return;
			//ws->publish(std::to_string(data->match_id), message);
			// evaluate
		},
		.close = [](WS* ws, int /*code*/, std::string_view /*message*/) {

		}
		}).listen(80, [](auto* listen_socket) {
			if (listen_socket) {
				std::cout << "Listening on port " << 80 << std::endl;
			}
			}).run();
}
