#include<iostream>
#include<chrono>
#include<thread>
#include"mythreadpool.h"
//有些场景，是希望能够获取线程执行任务得到返回值的
//举例：
//1+。。。+3000的和
//thread1 1+..。+1000
//thread1 1000 + ... + 3000

using uLong = unsigned long long;
class MyTask :public Task
{
public:
	MyTask(uLong begin, uLong end)
		:begin_(begin)
		, end_(end)
	{}
	//问题1：怎么设计run函数的返回值，可以表示任意的类型
	Any run()  //run函数在线程池分配的线程中去做事情
	{
		std::cout << "tid:" << std::this_thread::get_id()
			<< "begin!" << std::endl;
		//std::this_thread::sleep_for(std::chrono::seconds(3));
		uLong sum = 0;
		for (uLong i = begin_; i <= end_; i++)
		{
			sum += i;
		}
		std::cout << "tid:" << std::this_thread::get_id()
			<< "end!" << std::endl;
		return sum;
	}
private:
	uLong begin_;
	uLong end_;
};


int main()
{
	//{
	//	ThreadPool pool;
	//	//开始启动线程池
	//	pool.start(4);
	//	Result res1 = pool.submitTask(std::make_shared<MyTask>(1, 100000000));
	//	uLong sum1 = res1.get().cast_<uLong>();
	//	std::cout << sum1 << std::endl;
	//}
	//std::cout << "main over!" << std::endl;
	//getchar();
//#if 0
	//问题：ThreadPool对象析构以后，怎麽样把线程池相关的线程资源全部回收
	{
		ThreadPool pool;
		//用户自己设置线程池的工作模式
		pool.setMode(PoolMode::MODE_CACHED);
		//开始启动线程池
		pool.start(4);
		//问题2：如何设计这里的Result机制。使其和MyTask绑定
		Result res1 = pool.submitTask(std::make_shared<MyTask>(1, 100000000));
		Result res2 = pool.submitTask(std::make_shared<MyTask>(100000001, 200000000));
		Result res3 = pool.submitTask(std::make_shared<MyTask>(200000001, 300000000));
		pool.submitTask(std::make_shared<MyTask>(200000001, 300000000));
		pool.submitTask(std::make_shared<MyTask>(200000001, 300000000));
		pool.submitTask(std::make_shared<MyTask>(200000001, 300000000));

		//随着task被执行完，task对象没了，依赖于Task的Result对象也没了
		uLong sum1 = res1.get().cast_<uLong>();  //get返回一个Any类型，怎么转换为具体的类型
		uLong sum2 = res2.get().cast_<uLong>();
		uLong sum3 = res3.get().cast_<uLong>();

		//Master -Slave线程模型
		//Master线程用来分解任务，然后给各个Salve线程分配任务
		//等待各个Slave线程执行完任务，返回结果
		//Master线程合并各个任务结果，输出
		std::cout << (sum1 + sum2 + sum3) << std::endl;
		/*pool.submitTask(std::make_shared<MyTask>());
		pool.submitTask(std::make_shared<MyTask>());
		pool.submitTask(std::make_shared<MyTask>());
		pool.submitTask(std::make_shared<MyTask>());*/

		getchar();
		//主线程结束后，分离线程给自动回收了，分离线程可能还没启动
		//std::this_thread::sleep_for(std::chrono::seconds(5));  //睡眠函数，主线程睡眠5s

	}
//#endif

}




//问题总结：不定时的运行出现死锁的情况，