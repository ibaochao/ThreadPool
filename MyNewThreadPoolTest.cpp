#include <iostream>
#include "MyNewThreadPool.hpp"


int main() {

	MyNewThreadPool pool(4);
	for (size_t i = 0; i < 20; i++)
	{
		auto f = pool.enques([](int a, int b)->int {
			std::cout << "��ǰ�̣߳�" << std::this_thread::get_id() << std::endl;
			return a + b;
			}, 10 * i, 10 * i);

		std::cout << "�����" << f.get() << std::endl;
	}

	return 0;
}
