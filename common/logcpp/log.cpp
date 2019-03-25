#include <string.h>

namespace cpplog {
const char* StripFileName(const char *full_name) {
	const char *pos = full_name + strlen(full_name);
	while (pos != full_name) {
		-- pos;
		if (*pos == '/') {
			++ pos;
			break;
		}
	}
	return pos;
}
}
