#include "bus_log.h"
#include <algorithm>
#include "pdx.h"
#include <log.h>


namespace whale {

	typedef std::unordered_map<std::string, std::vector<std::string>> EcuLogMap;

	BusLog::BusLog(const std::filesystem::path &filename)
	{
		std::ifstream logFile;
		logFile.open(filename);

		EcuLogMap ecuLogMap;

		if (logFile.is_open()) {
			std::cout << "File [" << filename << "] is opened!" << std::endl;
			while (logFile) {
				std::string line;
				std::getline(logFile, line);
				// TODO: use regex to check if a line in the log file is a CAN trace
				// currently just check if it starts with '['
				if (line[0] == '[' && line.size() >= 129) {
					std::string id = line.substr(93, 10);
					if (ECU_ID.count(id)) {
						ecuLogMap[ECU_ID[id]].push_back(line);
					}
				}
			}
		}

		for (auto& ecuLog : ecuLogMap) {
			EcuTrace ecuTrace;
			ecuTrace.ecu = EcuMap[ecuLog.first];;

			for (auto line = ecuLog.second.cbegin(); line != ecuLog.second.cend(); line++) {
				std::string time, id, trace, hexTrace;
				MESSAGE_TYPE type = MESSAGE_TYPE::SEND;

				time = line->substr(1, 16);
				id = line->substr(93, 10);

				if (id == EcuMap["Broadcast"].ids[0]) {
					type = MESSAGE_TYPE::BROADCAST;
				}
				else {
					if (id == ecuTrace.ecu.ids[0]) {
						type = MESSAGE_TYPE::SEND;
					}
					else {
						if (id == ecuTrace.ecu.ids[1]) {
							type = MESSAGE_TYPE::RESPONSE;
						}
					}
				}

				hexTrace = trace = line->substr(105, 23);
				hexTrace.erase(std::remove(hexTrace.begin(), hexTrace.end(), ' '), hexTrace.end());

				// Single Frame
				if (trace[0] == '0') {
					size_t length = 0;
					std::istringstream(hexTrace.substr(1, 1)) >> std::hex >> length;
					hexTrace = hexTrace.substr(2, 2 * length);
				}

				// First frame
				if (trace[0] == '1') {
					size_t length = 0;
					std::istringstream(hexTrace.substr(1, 3)) >> std::hex >> length;
					std::string nextLine;
					size_t currentLength = 6;
					hexTrace = hexTrace.substr(4);

					// Consecutive frames
					while (currentLength < length && ++line != ecuLog.second.cend()) {
						std::string nextTrace;
						std::string nexthexTrace;
						nexthexTrace = nextTrace = line->substr(105, 23);
						nexthexTrace.erase(std::remove(nexthexTrace.begin(), nexthexTrace.end(), ' '), nexthexTrace.end());

						if (nexthexTrace[0] == '2') {
							if (length - currentLength > 7) {
								hexTrace += nexthexTrace.substr(2);
								currentLength += 7;
							}
							else {
								hexTrace += nexthexTrace.substr(2, (length - currentLength) * 2);
								currentLength = length;
							}
							trace += '\n' + nextTrace;
						}
					}
				}

				ecuTrace.traces.push_back(Trace{ type, time, id, trace, hexTrace });
			}

			_ecuTraceMap[ecuLog.first] = ecuTrace;
		}
	}

	const EcuTrace& BusLog::ecuTrace(std::string ecuName)
	{
		auto& ecuTrace = _ecuTraceMap[ecuName];

		PDX::get().decodeEcuTrace(ecuTrace);

		return ecuTrace;
	}

	EcuTraceMap* BusLog::ecuTraces()
	{
		return &_ecuTraceMap;
	}

	bool compareEcu(std::string lhs, std::string rhs) {
		return EcuMap[lhs].address < EcuMap[rhs].address;
	}

	std::vector<std::string> BusLog::getEcuList() const {
		std::vector<std::string> ecuList;
		for (auto& ecu : _ecuTraceMap) {
			ecuList.push_back(ecu.first);
		}
		std::sort(ecuList.begin(), ecuList.end(), compareEcu);

		WH_INFO("Got total of {} ecus.", ecuList.size());
		return ecuList;
	}

	void EcuTrace::decode()
	{
		String evName, evVersion;
		for (auto& trace : traces) {
			String hexString = trace.hexTrace;
			if (hexString.size() > 6 && hexString.substr(0, 6) == "62f19e") {
				auto response = trace.hexTrace.substr(6);
				evName = whale::hexToString(response);
				if (evName.back() == '\0') {
					evName.pop_back();
				}
			}
			if (!evName.empty() && hexString.substr(0, 6) == "62f1a2") {
				auto response = trace.hexTrace.substr(6);
				evVersion = whale::hexToString(response);
				break;
			}
		}

		if (!evName.empty()) {
			auto ev = PDX::get().getDlcById(evName + "_" + evVersion.substr(0, 3));

			if (ev != nullptr) {
				WH_INFO("Get EV: {}", ev->id());

				WH_INFO("DiagServices in [{}]:", ev->shortName());
				ev->inherit();
				for (auto& trace : traces) {
					ev->decode(trace);
				}
			}
		}
	}

}