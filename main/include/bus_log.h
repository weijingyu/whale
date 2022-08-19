#pragma once


#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <cmath>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <vector>
#include <filesystem>

namespace whale {

	enum class MESSAGE_TYPE { SEND, RESPONSE, BROADCAST };

	struct Trace {
		MESSAGE_TYPE type;
		std::string time;
		std::string id;
		std::string trace;
		std::string hexTrace;
	};

	struct ECU {
		std::string name;
		std::string address;
		std::string ids[2];
	};

	struct EcuLog {
		std::string name;
		std::vector<std::string> logs;
	};

	struct EcuTrace {
		ECU ecu;
		std::vector<Trace> traces;
	};

	/*
	static std::unordered_set<ECU> ECUS {
		{ "MSG",        "01", "0x17fc0076", "0x17fe0076" },
		{ "GSG",        "02", "0x17fc0077", "0x17fe0077" },
		{ "ESP",        "03", "0x00000713", "0x0000077d" },
		{ "Klima",      "08", "0x00000746", "0x000007b0" },
		{ "BCM",        "09", "0x0000070e", "0x00000778" },
		{ "ACC",        "13", "0x00000757", "0x000007c1" },
		{ "Airbag",     "15", "0x00000715", "0x0000077f" },
		{ "Kombi",      "17", "0x00000714", "0x0000077e" },
		{ "GW",         "19", "0x00000710", "0x0000077a" },
		{ "QCU",        "22", "0x0000070f", "0x00000779" },
		{ "ELV",        "2B", "0x00000731", "0x0000079b" },
		{ "MEM_FS",     "36", "0x0000074c", "0x000007b6" },
		{ "SWA",        "3C", "0x0000074e", "0x000007b8" },
		{ "TSG_FS",     "42", "0x0000074a", "0x000007b4" },
		{ "EPS",        "44", "0x00000712", "0x0000077c" },
		{ "Sound",      "47", "0x0000076f", "0x000007d9" },
		{ "MFMOD",      "4B", "0x17fc00a9", "0x17fe00a9" },
		{ "TSG_BS",     "52", "0x0000074b", "0x000007b5" },
		{ "MIB",        "5F", "0x00000773", "0x000007dd" },
		{ "RDK",        "65", "0x000007b0", "0x00000775" },
		{ "RV",         "6C", "0x00000769", "0x000007d3" },
		{ "HDSG",       "6D", "0x00000723", "0x0000078d" },
		{ "OCU",        "75", "0x00000767", "0x000007d1" },
		{ "PDC",        "76", "0x0000070a", "0x00000774" },
		{ "HUD",        "82", "0x0000071b", "0x00000785" },
		{ "MFK",        "A5", "0x0000074f", "0x000007b9" },
		{ "Kessy",      "B7", "0x00000732", "0x0000079c" },
		{ "TSG_F_H",    "BB", "0x0000073e", "0x000007a8" },
		{ "TSG_B_H",    "BC", "0x0000073f", "0x000007a9" },
		{ "SAD",        "CA", "0x17fc0084", "0x17fe0084" },
		{ "LLE_LI",     "D6", "0x17fc0096", "0x17fe0096" },
		{ "LLE_RE",     "D7", "0x17fc0097", "0x17fe0097" },
		{ "Broadcast",  "00", "0x00000700" }
	};
	*/

	static std::unordered_map<std::string, ECU> EcuMap{
		{ "MSG",       { "MSG",        "01", "0x17fc0076", "0x17fe0076" } },
		{ "GSG",       { "GSG",        "02", "0x17fc0077", "0x17fe0077" } },
		{ "ESP",       { "ESP",        "03", "0x00000713", "0x0000077d" } },
		{ "Klima",     { "Klima",      "08", "0x00000746", "0x000007b0" } },
		{ "BCM",       { "BCM",        "09", "0x0000070e", "0x00000778" } },
		{ "ACC",       { "ACC",        "13", "0x00000757", "0x000007c1" } },
		{ "Airbag",    { "Airbag",     "15", "0x00000715", "0x0000077f" } },
		{ "Kombi",     { "Kombi",      "17", "0x00000714", "0x0000077e" } },
		{ "GW",        { "GW",         "19", "0x00000710", "0x0000077a" } },
		{ "QCU",       { "QCU",        "22", "0x0000070f", "0x00000779" } },
		{ "ELV",       { "ELV",        "2B", "0x00000731", "0x0000079b" } },
		{ "MEM_FS",    { "MEM_FS",     "36", "0x0000074c", "0x000007b6" } },
		{ "SWA",       { "SWA",        "3C", "0x0000074e", "0x000007b8" } },
		{ "TSG_FS",    { "TSG_FS",     "42", "0x0000074a", "0x000007b4" } },
		{ "EPS",       { "EPS",        "44", "0x00000712", "0x0000077c" } },
		{ "Sound",     { "Sound",      "47", "0x0000076f", "0x000007d9" } },
		{ "MFMOD",     { "MFMOD",      "4B", "0x17fc00a9", "0x17fe00a9" } },
		{ "TSG_BS",    { "TSG_BS",     "52", "0x0000074b", "0x000007b5" } },
		{ "MIB",       { "MIB",        "5F", "0x00000773", "0x000007dd" } },
		{ "RDK",       { "RDK",        "65", "0x000007b0", "0x00000775" } },
		{ "RV",        { "RV",         "6C", "0x00000769", "0x000007d3" } },
		{ "HDSG",      { "HDSG",       "6D", "0x00000723", "0x0000078d" } },
		{ "OCU",       { "OCU",        "75", "0x00000767", "0x000007d1" } },
		{ "PDC",       { "PDC",        "76", "0x0000070a", "0x00000774" } },
		{ "HUD",       { "HUD",        "82", "0x0000071b", "0x00000785" } },
		{ "MFK",       { "MFK",        "A5", "0x0000074f", "0x000007b9" } },
		{ "Kessy",     { "Kessy",      "B7", "0x00000732", "0x0000079c" } },
		{ "TSG_F_H",   { "TSG_F_H",    "BB", "0x0000073e", "0x000007a8" } },
		{ "TSG_B_H",   { "TSG_B_H",    "BC", "0x0000073f", "0x000007a9" } },
		{ "SAD",       { "SAD",        "CA", "0x17fc0084", "0x17fe0084" } },
		{ "LLE_LI",    { "LLE_LI",     "D6", "0x17fc0096", "0x17fe0096" } },
		{ "LLE_RE",    { "LLE_RE",     "D7", "0x17fc0097", "0x17fe0097" } },
		{ "Broadcast", { "Broadcast",  "00", "0x00000700" }}
	};

	static std::unordered_map<std::string, std::string> ECU_ID{
		{ "0x17fc0076", "MSG" },		{ "0x17fe0076", "MSG" },
		{ "0x17fc0077", "GSG" },		{ "0x17fe0077", "GSG" },
		{ "0x00000713", "ESP" },		{ "0x0000077d", "ESP" },
		{ "0x00000746", "Klima" },		{ "0x000007b0", "Klima" },
		{ "0x0000070e", "BCM" },		{ "0x00000778", "BCM" },
		{ "0x00000757", "ACC" },		{ "0x000007c1", "ACC" },
		{ "0x00000715", "Airbag"},		{ "0x0000077f", "Airbag" },
		{ "0x00000714", "Kombi" },		{ "0x0000077e", "Kombi" },
		{ "0x00000710", "GW" },			{ "0x0000077a", "GW" },
		{ "0x0000070f", "QCU" },		{ "0x00000779", "QCU" },
		{ "0x00000731", "ELV" },		{ "0x0000079b", "ELV" },
		{ "0x0000074c", "MEM_FS" },		{ "0x000007b6", "MEM_FS" },
		{ "0x0000074e", "SWA" },		{ "0x000007b8", "SWA" },
		{ "0x0000074a", "TSG_FS" },		{ "0x000007b4", "TSG_FS" },
		{ "0x00000712", "EPS" },		{ "0x0000077c", "EPS" },
		{ "0x0000076f", "Sound" },		{ "0x000007d9", "Sound" },
		{ "0x17fc00a9", "MFMOD" },		{ "0x17fe00a9", "MFMOD" },
		{ "0x0000074b", "TSG_BS" },		{ "0x000007b5", "TSG_BS" },
		{ "0x00000773", "MIB" },		{ "0x000007dd", "MIB" },
		{ "0x000007b0", "RDK" },		{ "0x00000775", "RDK" },
		{ "0x00000769", "RV" },			{ "0x000007d3", "RV" },
		{ "0x00000723", "HDSG" },		{ "0x0000078d", "HDSG" },
		{ "0x00000767", "OCU" },		{ "0x000007d1", "OCU" },
		{ "0x0000070a", "PDC" },		{ "0x00000774", "PDC" },
		{ "0x0000071b", "HUD" },		{ "0x00000785", "HUD" },
		{ "0x0000074f", "MFK" },		{ "0x000007b9", "MFK" },
		{ "0x00000732", "Kessy" },		{ "0x0000079c", "Kessy" },
		{ "0x0000073e", "TSG_F_H" },	{ "0x000007a8", "TSG_F_H" },
		{ "0x0000073f", "TSG_B_H" },	{ "0x000007a9", "TSG_B_H" },
		{ "0x17fc0084", "SAD" },		{ "0x17fe0084", "SAD" },
		{ "0x17fc0096", "LLE_LI" },		{ "0x17fe0096", "LLE_LI" },
		{ "0x17fc0097", "LLE_RE" },		{ "0x17fe0097", "LLE_RE" },
		{ "0x00000700", "Broadcast"}
	};

	/*
	static std::unordered_map<std::string, std::string> ECU_ADDRESS {
		{ "MSG",        "01" },
		{ "GSG",        "02" },
		{ "ESP",        "03" },
		{ "Klima",      "08" },
		{ "BCM",        "09" },
		{ "ACC",        "13" },
		{ "Airbag",     "15" },
		{ "Kombi",      "17" },
		{ "GW",         "19" },
		{ "QCU",        "22" },
		{ "ELV",        "2B" },
		{ "MEM_FS",     "36" },
		{ "SWA",        "3C" },
		{ "TSG_FS",     "42" },
		{ "EPS",        "44" },
		{ "Sound",      "47" },
		{ "MFMOD",      "4B" },
		{ "TSG_BS",     "52" },
		{ "MIB",        "5F" },
		{ "RDK",        "65" },
		{ "RV",         "6C" },
		{ "HDSG",       "6D" },
		{ "OCU",        "75" },
		{ "PDC",        "76" },
		{ "HUD",        "82" },
		{ "MFK",        "A5" },
		{ "Kessy",      "B7" },
		{ "TSG_F_H",    "BB" },
		{ "TSG_B_H",    "BC" },
		{ "SAD",        "CA" },
		{ "LLE_LI",     "D6" },
		{ "LLE_RE",     "D7" },
		{ "Broadcast",  "00" }
	};
	*/

	typedef std::unordered_map<std::string, EcuTrace> EcuTraceMap;

	class BusLog {
	public:
		BusLog() = delete;
		BusLog(const std::filesystem::path &filename);

		const EcuTrace& ecuTrace(std::string ecuName);
		EcuTraceMap* ecuTraces();
		size_t size() const { return _ecuTraceMap.size(); };
		std::vector<std::string> getEcuList() const;

	private:
		EcuTraceMap _ecuTraceMap;
	};

}