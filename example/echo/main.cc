#include <muduo/base/Logging.h>
int main()
{
	muduo::Logger::setLogLevel(muduo::Logger::TRACE);
	LOG_TRACE << "test";
	LOG_DEBUG << "test";
	LOG_INFO << "test";
	LOG_WARN << "test";
	LOG_ERROR << "test";
	//LOG_FATAL << "test";
	LOG_INFO << "BYEBYE";
	LOG_SYSERR << "test";
	//LOG_SYSFATAL << "test";
	LOG_INFO << "BYEBYE";
}
