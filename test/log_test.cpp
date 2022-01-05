#include "../src/log/log.h"
#include <unistd.h>
int main()
{
    int m_close_log= 0;
    int max_log_num = 800;
    if (0 == m_close_log)
    {
        Log::get_instance()->init("./ServerLog", m_close_log, 2000, 800000, max_log_num);
    }

    
    LOG_INFO("INFO错误"); 
    LOG_DEBUG("DEBUG错误"); 
    LOG_WARN("WARN错误");
    LOG_ERROR("ERROR错误");

    if(max_log_num > 0)
        sleep(1);
}