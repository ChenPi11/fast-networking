/* @NO MRG */
#include "13-fmt.h"
#include "15-logging.h"
#include "16-net.h"

/* @END NO MRG */

int main()
{
	RESULT_NET_Socket v = NET_async_socket(NET_INET_VERSION_4);
	if (!v.has_value)
	{
		printf("%d", v.result.error.code);
		LOGGING_error("MAIN", "Failed to create async socket: %1",
					  FMT_str(EXC_strerror(v.result.error)));
		return 1;
	}
	else
	{
		LOGGING_info("MAIN", "Successfully created async socket: %1",
					 FMT_hex16_4((UTIL_UInt16)v.result.value));
	}

	return 0;
}
