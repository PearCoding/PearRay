#include "PerformanceWriter.h"
#include "Performance.h"

#include <fstream>
#include <iomanip>

namespace PR {
// Lets just do it here :)
thread_local PerformanceEntry* PerformanceManager::mCurrentParent = nullptr;

void PerformanceWriter::write(const std::string& filename)
{
	std::ofstream stream(filename);

	const auto& entries = PerformanceManager::instance().entries();
	stream << "version: 1" << std::endl
		   << "creator: PearRay" << std::endl
		   << "positions: line" << std::endl
		   << "events: Ir" << std::endl;

	for (const auto& p : entries) {
		for (const auto& s : p.second) {
			PerformanceEntry* e = s.Entry;

			stream << "fl=(" << e->hash() << ") " << e->file() << std::endl
				   << "fn=(" << e->hash() << ") " << e->function() << std::endl
				   << e->line() << " " << e->ticks() << std::endl;

			for (auto c : e->children()) {
				stream << "cfl=(" << c->hash() << ") " << c->file() << std::endl
					   << "cfn=(" << c->hash() << ") " << c->function() << std::endl
					   << "calls=1 " << e->line() << std::endl
					   << c->line() << " 1" << std::endl;
			}

			stream << std::endl;
		}
	}
	stream.close();
}
}
