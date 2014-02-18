#include <cxxabi.h>
#include <cstdlib>
#include <string>
#include "qolor/utilities.h"

// Get type name in a pretty form.
// http://stackoverflow.com/questions/281818/unmangling-the-result-of-stdtype-infoname

std::string qolor::utils::type_name(const char* name, bool withOriginal)
{
#if defined(__GNUC__) && defined(__GNUC_MINOR__)
	int status = -4;
	char* res = abi::__cxa_demangle(name, NULL, NULL, &status);
	const char* const demangled_name = (status == 0)? res : name;
	std::string ret_val(demangled_name);
	free(res);
	if (withOriginal && (status == 0)) {
		ret_val += '(';
		ret_val += name;
		ret_val += ')';
	}
	return ret_val;
#else
	withOriginal = false;
	return name;
#endif
}

std::string qolor::utils::type_name(const std::type_info& type, const bool& withOriginal)
{
	return type_name(type.name(), withOriginal);
}
