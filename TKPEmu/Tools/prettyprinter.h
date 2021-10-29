#pragma once
#ifndef TKP_TOOLS_PRETTYPRINTER_H
#define TKP_TOOLS_PRETTYPRINTER_H
#include <sstream>
#include <string>
// TODO: Remove this class, create a console instead
namespace TKPEmu::Tools {
	enum class PrettyPrinterMessageType {
		Default,
		Error,
		Success,
		Info
	};

	class PrettyPrinter {
	public:
		PrettyPrinter();
		template<PrettyPrinterMessageType ppmt>
		void PrettyAdd(std::string&& s);
	private:
		std::string buffer;
	};

	template<PrettyPrinterMessageType ppmt>
	inline void PrettyPrinter::PrettyAdd(std::string&& s) {
		buffer += "[BAD-MESSAGE-TYPE]: PrettyAdd has no message type specified.";
	}

	template<>
	inline void PrettyPrinter::PrettyAdd<PrettyPrinterMessageType::Default>(std::string&& s) {
		buffer += "[MESSAGE]: " + s + "\n";
	}

	template<>
	inline void PrettyPrinter::PrettyAdd<PrettyPrinterMessageType::Error>(std::string&& s) {
		buffer += "[ ERROR ]: " + s + "\n";
	}

	template<>
	inline void PrettyPrinter::PrettyAdd<PrettyPrinterMessageType::Info>(std::string&& s) {
		buffer += "[ INFO  ]: " + s + "\n";
	}
	template<>
	inline void PrettyPrinter::PrettyAdd<PrettyPrinterMessageType::Success>(std::string&& s) {
		buffer += "[SUCCESS]: " + s + "\n";
	}
}
#endif