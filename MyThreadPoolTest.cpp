#include<iostream>
#include<chrono>
#include<thread>
#include"mythreadpool.h"
//��Щ��������ϣ���ܹ���ȡ�߳�ִ������õ�����ֵ��
//������
//1+������+3000�ĺ�
//thread1 1+..��+1000
//thread1 1000 + ... + 3000

using uLong = unsigned long long;
class MyTask :public Task
{
public:
	MyTask(uLong begin, uLong end)
		:begin_(begin)
		, end_(end)
	{}
	//����1����ô���run�����ķ���ֵ�����Ա�ʾ���������
	Any run()  //run�������̳߳ط�����߳���ȥ������
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
	//	//��ʼ�����̳߳�
	//	pool.start(4);
	//	Result res1 = pool.submitTask(std::make_shared<MyTask>(1, 100000000));
	//	uLong sum1 = res1.get().cast_<uLong>();
	//	std::cout << sum1 << std::endl;
	//}
	//std::cout << "main over!" << std::endl;
	//getchar();
//#if 0
	//���⣺ThreadPool���������Ժ����������̳߳���ص��߳���Դȫ������
	{
		ThreadPool pool;
		//�û��Լ������̳߳صĹ���ģʽ
		pool.setMode(PoolMode::MODE_CACHED);
		//��ʼ�����̳߳�
		pool.start(4);
		//����2�������������Result���ơ�ʹ���MyTask��
		Result res1 = pool.submitTask(std::make_shared<MyTask>(1, 100000000));
		Result res2 = pool.submitTask(std::make_shared<MyTask>(100000001, 200000000));
		Result res3 = pool.submitTask(std::make_shared<MyTask>(200000001, 300000000));
		pool.submitTask(std::make_shared<MyTask>(200000001, 300000000));
		pool.submitTask(std::make_shared<MyTask>(200000001, 300000000));
		pool.submitTask(std::make_shared<MyTask>(200000001, 300000000));

		//����task��ִ���꣬task����û�ˣ�������Task��Result����Ҳû��
		uLong sum1 = res1.get().cast_<uLong>();  //get����һ��Any���ͣ���ôת��Ϊ���������
		uLong sum2 = res2.get().cast_<uLong>();
		uLong sum3 = res3.get().cast_<uLong>();

		//Master -Slave�߳�ģ��
		//Master�߳������ֽ�����Ȼ�������Salve�̷߳�������
		//�ȴ�����Slave�߳�ִ�������񣬷��ؽ��
		//Master�̺߳ϲ����������������
		std::cout << (sum1 + sum2 + sum3) << std::endl;
		/*pool.submitTask(std::make_shared<MyTask>());
		pool.submitTask(std::make_shared<MyTask>());
		pool.submitTask(std::make_shared<MyTask>());
		pool.submitTask(std::make_shared<MyTask>());*/

		getchar();
		//���߳̽����󣬷����̸߳��Զ������ˣ������߳̿��ܻ�û����
		//std::this_thread::sleep_for(std::chrono::seconds(5));  //˯�ߺ��������߳�˯��5s

	}
//#endif

}




//�����ܽ᣺����ʱ�����г��������������