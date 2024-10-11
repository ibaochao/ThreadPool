#pragma once

#include <iostream>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <future>


class MyNewThreadPool {
public:

	MyNewThreadPool(int threadnums); // ���캯��

	// ������������������ã���ֵ����ֵ��
	template<typename F, typename ...Args> // F�Ǻ���
	auto enques(F&& f, Args&&... args) -> std::future<typename std::invoke_result_t<F, Args...>>;  

	~MyNewThreadPool(); // ��������

private:
	void worker(); // �߳�ִ�к���
	bool isstop; // �̳߳�״̬
	std::condition_variable cv; // �������������п�����
	std::mutex mtx; // ������
	std::vector<std::thread> workers; // �̼߳���
	std::queue<std::function<void()>> myque; // �������

};

// ���캯��
MyNewThreadPool::MyNewThreadPool(int threadnums):isstop(false) {

	for (size_t i = 0; i < threadnums; i++)
	{
		workers.emplace_back([this]() {
			this->worker();
			});
	}

}

// ��������
MyNewThreadPool::~MyNewThreadPool(){

	// �޸��̳߳�״̬
	{
		std::unique_lock<std::mutex>(mtx);
		isstop = true;
	}

	// ֪ͨ�߳�ִ�������ر�
	cv.notify_all();

	// ȷ���߳�ִ�����
	for (std::thread& t : workers) {
		t.join();
	}
}


// ���������
template<typename F, typename ...Args>
//auto MyNewThreadPool::enques(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>{ // C++11
//	// �Զ��庯������ֵ����
//	using functype = typename std::result_of < F(Args...)>::type;
auto MyNewThreadPool::enques(F&& f, Args&&... args) -> std::future<typename std::invoke_result_t<F, Args...>> { // C++17
	// �Զ��庯������ֵ����
	using functype = typename std::invoke_result_t<F, Args...>;
	// ��װ���񣬰󶨲���������ת��
	auto task = std::make_shared<std::packaged_task<functype()>>(
		std::bind(std::forward<F>(f), std::forward<Args>(args)...)
	);
	// ����future����
	std::future<functype> rsfuture = task->get_future();

	// �������
	{
		std::lock_guard<std::mutex> lockguard(this->mtx);
		if (isstop) {
			throw std::runtime_error("�̳߳�ֹͣ���ѹر���");
		}

		myque.emplace([task]() {
			(*task)();
			});
	}
	// ֪ͨ�߳�ִ������
	cv.notify_one();
	// ���ع���future����
	return rsfuture;
}


// �߳�ִ�к���
void MyNewThreadPool::worker() {
	while (true) {
		// ��������
		std::function<void()> task;
		// �Ӷ��л�ȡ����
		{
			std::unique_lock<std::mutex> lock(mtx);
			cv.wait(lock, [this]() {return this->isstop || !this->myque.empty(); });
			if (isstop && myque.empty()) return;
			task = std::move(this->myque.front());
			this->myque.pop();
		}
		// ִ������
		task();
	}
}