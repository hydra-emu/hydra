#ifndef TKP_TOOLS_PRETTYPRINTER_H
#define TKP_TOOLS_PRETTYPRINTER_H
#include <sstream>
namespace TKP::Tools {
	enum class PrettyPrinterMessageType {
		DEFAULT,
		ERROR,
		WARNING,
		SUCCESS
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
	inline void PrettyPrinter::PrettyAdd<PrettyPrinterMessageType::DEFAULT>(std::string&& s) {
		buffer += "[MESSAGE]: " + s + "\n";
	}

	template<>
	inline void PrettyPrinter::PrettyAdd<PrettyPrinterMessageType::ERROR>(std::string&& s) {
		buffer += "[ ERROR ]: " + s + "\n";
	}

	template<>
	inline void PrettyPrinter::PrettyAdd<PrettyPrinterMessageType::WARNING>(std::string&& s) {
		buffer += "[WARNING]: " + s + "\n";
	}
	template<>
	inline void PrettyPrinter::PrettyAdd<PrettyPrinterMessageType::SUCCESS>(std::string&& s) {
		buffer += "[SUCCESS]: " + s + "\n";
	}
}
#endif