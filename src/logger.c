#include "logger.h"
#include <time.h>

#if LOG_LEVEL > LEVEL_NO_LOGS
void get_date_time(char* date_time_str)
{
    time_t ltime;
    struct tm result;
    ltime = time(NULL);
    localtime_r(&ltime, &result);
    // The string must be at least 26 character long. The returned value contains a \n\0 at the end.
    asctime_r(&result, date_time_str);
    // Overwrite the \n to avoid a new line.
    date_time_str[24] = 0;
}
#endif
