#include <acl_cpp/lib_acl.hpp>
#include <fiber/libfiber.hpp>


// 客户端协程处理类，用来回显客户发送的内容，每一个客户端连接绑定一个独立的协程
class fiber_echo : public acl::fiber
{
public:
	fiber_echo(acl::socket_stream* conn) : conn_(conn) {}
private:
	acl::socket_stream* conn_;
	~fiber_echo(void) { delete conn_; }
	// @override
	void run(void) {
		char buf[8192];
		while (true) {
			// 从客户端读取数据（第三个参数为false表示不必填满整个缓冲区才返回）
			int ret = conn_->read(buf, sizeof(buf), false);
			if (ret == -1) {
				break;
			}
			// 向客户端写入读到的数据
			if (conn_->write(buf, ret) != ret) {
				break;
			}
		}
		delete this; // 自销毁动态创建的协程对象
	}
};

// 独立的协程过程，接收客户端连接，并将接收的连接与新创建的协程进行绑定
class fiber_listen : public acl::fiber
{
public:
	fiber_listen(acl::server_socket& listener) : listener_(listener) {}
private:
	acl::server_socket& listener_;
	~fiber_listen(void) {}
	// @override
	void run(void) {
		while (true) {
			acl::socket_stream* conn = listener_.accept();  // 等待客户端连接
			if (conn == NULL) {
				printf("accept failed: %s\r\n", acl::last_serror());
				break;
			}
			// 创建并启动单独的协程处理客户端连接
			acl::fiber* fb = new fiber_echo(conn);
			// 启动独立的客户端处理协程
			fb->start();
		}
		delete this;
	}
};

int main(void)
{
    // Initialize Winsock
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
    {
        printf("WSAStartup failed: %d\n", result);
        return 1;
    }

    // Use Windows Sockets API here...


	const char* addr = "127.0.0.1:8800";
	acl::server_socket listener;
	// 监听本地地址
	if (listener.open(addr) == false) {
		printf("listen %s error %s\r\n", addr, acl::last_serror());
		return 1;
	}

	// 创建并启动独立的监听协程，接受客户端连接
	acl::fiber* fb = new fiber_listen(listener);
	fb->start();

	// 启动协程调度器
	acl::fiber::schedule();
	return 0;
}