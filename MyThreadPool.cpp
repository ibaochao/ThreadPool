#include "mythreadpool.h"
#include<functional>
#include<thread>
#include<iostream>

const int TASK_MAX_THRESHHOLD = 1024;
const int THREAD_MAX_THRESHHOLD = 10;
const int THREAD_MAX_IDLE_TIME = 10;  //��λ����  �߳�������ʱ��

//�̳߳ع���
ThreadPool::ThreadPool()
	:initThreadSize_(4)
	, taskSize_(0)
	, idleThreadSize_(0)
	, taskQueMaxThreshHold_(TASK_MAX_THRESHHOLD)
	, threadSizeThreshHold_(THREAD_MAX_THRESHHOLD)
	, curThreadSize_(0)
	, poolMode_(PoolMode::MODE_FIXED)
	, isPoolRunning_(false)
{}

//�̳߳�����
ThreadPool::~ThreadPool()
{
	isPoolRunning_ = false;


	//�ȴ��̳߳��������е��̷߳���  ������״̬������ & ����ִ��������
	std::unique_lock<std::mutex>lock(taskQueMtx_);
	notEmpty_.notify_all();
	exitCond_.wait(lock, [&]()->bool {return threads_.size() == 0; });
}

//�����̳߳صĹ���ģʽ
void ThreadPool::setMode(PoolMode mode)
{
	if (checkRunningState())
		return;
	poolMode_ = mode;
}

//����task������е�������ֵ
void ThreadPool::setTaskQueMaxThreshHold(int threshhold)
{
	if (checkRunningState())
		return;
	taskQueMaxThreshHold_ = threshhold;
}

//�����̳߳�cachedģʽ���߳���ֵ
void ThreadPool::setThreadSizeThreshHold(int threshhold)
{
	if (checkRunningState())
		return;
	if (poolMode_ == PoolMode::MODE_CACHED)
	{
		threadSizeThreshHold_ = threshhold;
	}
}

//���̳߳��ύ����  �û����ýӿ� �������� ��������
Result ThreadPool::submitTask(std::shared_ptr<Task>sp)  //��Ϊ�������ڲ�֪������˴�һ������ָ��
{
	//��ȡ��
	std::unique_lock<std::mutex>lock(taskQueMtx_);

	//�̵߳�ͨ��  �ȴ���������п���
	/*while (taskQue_.size() == taskQueMaxThreshHold_)  //��lamda���ʽ������ͬ
	{
		notFull_.wait(lock);
	}*/
	//�û��ύ�������������1s�������ж��ύ����ʧ�ܣ�����
	if (!notFull_.wait_for(lock, std::chrono::seconds(1),
		[&]()->bool {return taskQue_.size() < (size_t)taskQueMaxThreshHold_; })) //lamda���ʽ
	{
		//��ʾnotfull_�ȴ�1s��������Ȼû������
		std::cerr << "task queue in full,submit task fail." << std::endl;
		return Result(sp, false);
	}
	//����п��࣬������������������
	taskQue_.emplace(sp);
	taskSize_++;

	//��Ϊ�·�������������п϶������ˣ���notEmpty_�Ͻ���֪ͨ,�Ͽ�����߳�ִ������
	notEmpty_.notify_all();

	//��Ҫ�������������Ϳ����̵߳��������ж��Ƿ���Ҫ�����µ��̳߳���  cachedģʽ:С������߳�
	if (poolMode_ == PoolMode::MODE_CACHED
		&& taskSize_ > idleThreadSize_
		&& curThreadSize_ < threadSizeThreshHold_)
	{
		std::cout << ">>>create new threat" << std::this_thread::get_id() << std::endl;
		//�������߳�
		//����thread�̶߳����ʱ�򣬰�threadpool���̺߳�������thread����   //��
		auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this, std::placeholders::_1));
		int threadId = ptr->getId();
		threads_.emplace(threadId, std::move(ptr));
		threads_[threadId]->start();
		curThreadSize_++;
		idleThreadSize_++;
		//threads_.emplace_back(ptr);  //unique_ptr������������
		//threads_.emplace_back(std::move(ptr));
	}

	//���������Result����
	return Result(sp);
}
//�����̳߳�
void ThreadPool::start(int initThreadSize)
{
	//�����̳߳ص�start
	isPoolRunning_ = true;

	//��¼��ʼ�̸߳���
	initThreadSize_ = initThreadSize;
	curThreadSize_ = initThreadSize;

	//�����̶߳���
	for (int i = 0; i < (int)initThreadSize_; i++)
	{
		//����thread�̶߳����ʱ�򣬰�threadpool���̺߳�������thread����   //��
		auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this, std::placeholders::_1));
		int threadId = ptr->getId();
		threads_.emplace(threadId, std::move(ptr));
		//threads_.emplace_back(ptr);  //unique_ptr������������
		//threads_.emplace_back(std::move(ptr));
	}

	//���������߳�
	for (int i = 0; i < (int)initThreadSize_; i++)
	{
		threads_[i]->start();
		idleThreadSize_++; //��¼��ʼ�����̵߳�����
	}
}

//�����̺߳���  �����̺߳����Ǵ������������������
void ThreadPool::threadFunc(int threadid)
{
	auto lastTime = std::chrono::high_resolution_clock().now();

	//�����������ִ����ɣ��̳߳زſ��Ի��������߳���Դ
	//while (isPoolRunning_)
	for (;;)
	{
		std::shared_ptr<Task> task;
		{
			//�Ȼ�ȡ��
			std::unique_lock<std::mutex>lock(taskQueMtx_);

			std::cout << "tid:" << std::this_thread::get_id()
				<< "���Ի�ȡ����" << std::endl;

			//cachedģʽ�£��п����Ѿ������˺ܶ���̣߳����ǿ���ʱ�䳬��60s��Ӧ�ðѶ��ࣨ������ʼ���߳����������߳̽������յ�
			//��ǰʱ��-��һ���߳�ִ�е�ʱ��>60s

			//ÿһ���з���һ��  ��ô���֣���ʱ���أ������������ִ�з���
			while (taskQue_.size() == 0)
			{
				//�̳߳�Ҫ�����������߳���Դ
				if (!isPoolRunning_)
				{
					threads_.erase(threadid);
					std::cout << "threadid:" << std::this_thread::get_id() << "exit" << std::endl;
					exitCond_.notify_all();
					return;
				}
				if (poolMode_ == PoolMode::MODE_CACHED)
				{
					//����������ʱ������
					if (std::cv_status::timeout == notEmpty_.wait_for(lock, std::chrono::seconds(1)))
					{
						auto now = std::chrono::high_resolution_clock().now();
						auto dur = std::chrono::duration_cast<std::chrono::seconds>(now - lastTime);
						if (dur.count() >= THREAD_MAX_IDLE_TIME && curThreadSize_ > initThreadSize_)
						{
							//��ʼ�����߳�
							//��¼�߳���������ر�����ֵ�޸�
							//���̶߳�����߳��б�������ɾ�� 
							//threadId=>thread����=��ɾ��
							threads_.erase(threadid);
							curThreadSize_--;
							idleThreadSize_--;
							std::cout << "threadid:" << std::this_thread::get_id() << "exit" << std::endl;
							return;
						}
					}
				}
				else
				{
					//�ȴ�notEmpty����
					notEmpty_.wait(lock);
				}
				//
				/*if (!isPoolRunning_)
				{
					threads_.erase(threadid);
					std::cout << "threadid:" << std::this_thread::get_id() << "exit" << std::endl;
					exitCond_.notify_all();
					return;
				}*/
			}

			// �̳߳�Ҫ�����������߳���Դ
				//if (!isPoolRunning_)
				//{
				//	break;
				//}

				idleThreadSize_--;

			std::cout << "tid:" << std::this_thread::get_id() << "��ȡ����ɹ�" << std::endl;
			//�����������ȡһ���������
			task = taskQue_.front();
			taskQue_.pop();
			taskSize_--;

			//�����Ȼ��ʣ�����񣬼���֪ͨ�������߳�ִ������
			if (taskQue_.size() > 0)
			{
				notEmpty_.notify_all();
			}
			//ȡ��һ�����񣬽���֪ͨ,֪ͨ���Լ����ύ��������
			notFull_.notify_all();
		}//��Ӧ�ð����ͷŵ�

			//��ǰ�̸߳���ִ���������
		if (task != nullptr)
		{
			//task->run();  //ִ������ ������ķ���ֵsetVal��������Result
			task->exec();
		}
		idleThreadSize_++;
		lastTime = std::chrono::high_resolution_clock().now();  //�����߳�ִ���������ʱ��
	}
	/*threads_.erase(threadid);
	std::cout << "threadid:" << std::this_thread::get_id() << "exit" << std::endl;
	exitCond_.notify_all();*/
}




bool ThreadPool::checkRunningState() const
{
	return isPoolRunning_;
}
// �̷߳���ʵ��
int Thread::generateId_ = 0;

//�̹߳���
Thread::Thread(ThreadFunc func)
	:func_(func)
	, threadId_(generateId_++)
{

}
//�߳�����
Thread::~Thread()
{

}

//�����߳�
void Thread::start()
{
	//ִ��һ���̺߳���
	std::thread t(func_, threadId_);         //C++11���߳���  �̶߳���t ���̺߳���func_
	t.detach();  //���÷����߳�   ���̶߳�����̺߳������з��룬���������߳���������������̶߳�����ʧ����ʧ�ˣ���Ӱ�캯��������
}
int Thread::getId()const
{
	return threadId_;
}

///Task����ʵ��
Task::Task()
	:result_(nullptr)
{

}
void Task::exec()
{
	if (result_ != nullptr)
	{
		result_->setVal(run());  //���﷢����̬����
	}
}
void Task::setResult(Result* res)
{
	result_ = res;
}
//  Result������ʵ��
Result::Result(std::shared_ptr<Task> task, bool isValid)
	:isValid_(isValid)
	, task_(task)
{
	task->setResult(this);
}
Any Result::get()  //�û����õ�
{
	if (!isValid_)
	{
		return"";
	}

	sem_.wait();  //task�������û��ִ���꣬����������û����߳�
	return std::move(any_);
}

void Result::setVal(Any any)  //˭���õ�
{
	//�洢task�ķ���ֵ
	this->any_ = std::move(any);
	sem_.post();  //�Ѿ���ȡ������ķ���ֵ�������ź�����Դ
}