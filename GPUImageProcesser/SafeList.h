#pragma once
#include<list>
#include<mutex>

class _mutex {
public:
	std::mutex * locker;
	_mutex() { locker = new std::mutex(); }
	~_mutex() { delete locker; }
	_mutex(_mutex const& mutex) { locker = new std::mutex(); }
	_mutex & operator =(_mutex const& mutex) { return *this; }
};

template<typename T>
class SafeList{
	std::list<T> list= * new std::list<T>;
	mutable std::mutex locker;
public:
	//SafeList<T> & operator = (SafeList<T> const & li) { return this; }

	T getAndPop_front() {
		locker.lock();
		T t=list.front();
		list.pop_front();
		locker.unlock();
		return t;
	}
	T getAndPop_back() {
		locker.lock();
		T t = list.back();
		list.pop_back();
		locker.unlock();
		return t;
	}
	T front() {
		return list.front();
	}
	T back() {
		return list.back();
	}

	void pushToFront(T t) {
		locker.lock();
		list.push_front(t);
		locker.unlock();
	}

	void pushToBack(T t) {
		locker.lock();
		list.push_back(t);
		locker.unlock();
	}
	size_t size() {
		return list.size();
	}

};