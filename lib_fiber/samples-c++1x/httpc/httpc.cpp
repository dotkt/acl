#include <acl_cpp/lib_acl.hpp>
#include <fiber/libfiber.hpp>


// �ͻ���Э�̴����࣬�������Կͻ����͵����ݣ�ÿһ���ͻ������Ӱ�һ��������Э��
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
			// �ӿͻ��˶�ȡ���ݣ�����������Ϊfalse��ʾ�������������������ŷ��أ�
			int ret = conn_->read(buf, sizeof(buf), false);
			if (ret == -1) {
				break;
			}
			// ��ͻ���д�����������
			if (conn_->write(buf, ret) != ret) {
				break;
			}
		}
		delete this; // �����ٶ�̬������Э�̶���
	}
};

// ������Э�̹��̣����տͻ������ӣ��������յ��������´�����Э�̽��а�
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
			acl::socket_stream* conn = listener_.accept();  // �ȴ��ͻ�������
			if (conn == NULL) {
				printf("accept failed: %s\r\n", acl::last_serror());
				break;
			}
			// ����������������Э�̴���ͻ�������
			acl::fiber* fb = new fiber_echo(conn);
			// ���������Ŀͻ��˴���Э��
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
	// �������ص�ַ
	if (listener.open(addr) == false) {
		printf("listen %s error %s\r\n", addr, acl::last_serror());
		return 1;
	}

	// ���������������ļ���Э�̣����ܿͻ�������
	acl::fiber* fb = new fiber_listen(listener);
	fb->start();

	// ����Э�̵�����
	acl::fiber::schedule();
	return 0;
}