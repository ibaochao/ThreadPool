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

	MyNewThreadPool(int threadnums); // 构造函数

	// 添加任务函数，万能引用（左值或右值）
	template<typename F, typename ...Args> // F是函数
	auto enques(F&& f, Args&&... args) -> std::future<typename std::invoke_result_t<F, Args...>>;  

	~MyNewThreadPool(); // 析构函数

private:
	void worker(); // 线程执行函数
	bool isstop; // 线程池状态
	std::condition_variable cv; // 条件变量，队列空满否
	std::mutex mtx; // 互斥锁
	std::vector<std::thread> workers; // 线程集合
	std::queue<std::function<void()>> myque; // 任务队列

};

// 构造函数
MyNewThreadPool::MyNewThreadPool(int threadnums):isstop(false) {

	for (size_t i = 0; i < threadnums; i++)
	{
		workers.emplace_back([this]() {
			this->worker();
			});
	}

}

// 析构函数
MyNewThreadPool::~MyNewThreadPool(){

	// 修改线程池状态
	{
		std::unique_lock<std::mutex>(mtx);
		isstop = true;
	}

	// 通知线程执行任务或关闭
	cv.notify_all();

	// 确保线程执行完成
	for (std::thread& t : workers) {
		t.join();
	}
}


// 添加任务函数
template<typename F, typename ...Args>
//auto MyNewThreadPool::enques(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>{ // C++11
//	// 自定义函数返回值类型
//	using functype = typename std::result_of < F(Args...)>::type;
auto MyNewThreadPool::enques(F&& f, Args&&... args) -> std::future<typename std::invoke_result_t<F, Args...>> { // C++17
	// 自定义函数返回值类型
	using functype = typename std::invoke_result_t<F, Args...>;
	// 封装任务，绑定参数，完美转发
	auto task = std::make_shared<std::packaged_task<functype()>>(
		std::bind(std::forward<F>(f), std::forward<Args>(args)...)
	);
	// 关联future对象
	std::future<functype> rsfuture = task->get_future();

	// 加入队列
	{
		std::lock_guard<std::mutex> lockguard(this->mtx);
		if (isstop) {
			throw std::runtime_error("线程池停止，已关闭了");
		}

		myque.emplace([task]() {
			(*task)();
			});
	}
	// 通知线程执行任务
	cv.notify_one();
	// 返回关联future对象
	return rsfuture;
}


// 线程执行函数
void MyNewThreadPool::worker() {
	while (true) {
		// 定义任务
		std::function<void()> task;
		// 从队列获取任务
		{
			std::unique_lock<std::mutex> lock(mtx);
			cv.wait(lock, [this]() {return this->isstop || !this->myque.empty(); });
			if (isstop && myque.empty()) return;
			task = std::move(this->myque.front());
			this->myque.pop();
		}
		// 执行任务
		task();
	}
}