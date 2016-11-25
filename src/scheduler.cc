/*
 * Copyright (C) 2016 deipi.com LLC and contributors. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <memory>

#include "scheduler.h"

#include "utils.h"       // for time_point_to_ullong


ScheduledTask::ScheduledTask(std::chrono::time_point<std::chrono::system_clock> created_at_)
	: wakeup_time(0),
	  created_at(time_point_to_ullong(created_at_)),
	  cleared_at(0) { }


ScheduledTask::~ScheduledTask() { }


bool
ScheduledTask::clear()
{
	unsigned long long c = 0;
	return cleared_at.compare_exchange_strong(c, time_point_to_ullong(std::chrono::system_clock::now()));
}


SchedulerQueue::SchedulerQueue() { }


TaskType*
SchedulerQueue::next(uint64_t final_key, bool keep_going)
{
	TaskType* task = nullptr;
	queue.next(&task, final_key, keep_going, false);
	return task;
}


TaskType*
SchedulerQueue::peep()
{
	TaskType* task = nullptr;
	queue.next(&task, 0, true, true);
	return task;
}


uint64_t
SchedulerQueue::add(const TaskType& task, uint64_t key)
{
	return queue.add(task, key);
}


Scheduler::Scheduler(const std::string& name_)
	: name(name_),
	  running(-1),
	  inner_thread(&Scheduler::run, this) { }


Scheduler::Scheduler(const std::string& name_, const std::string format, size_t num_threads)
	: thread_pool(std::make_unique<ThreadPool<>>(format, num_threads)),
	  name(name_),
	  running(-1),
	  inner_thread(&Scheduler::run, this) { }


Scheduler::~Scheduler()
{
	finish(1);
}


size_t
Scheduler::running_size()
{
	if (thread_pool) {
		thread_pool->running_size();
	}
	return 0;
}


void
Scheduler::finish(int wait)
{
	running = wait;
	wakeup_signal.notify_all();

	if (thread_pool) {
		thread_pool->finish();
	}

	if (wait) {
		join();
	}
}


void
Scheduler::join()
{
	try {
		if (inner_thread.joinable()) {
			inner_thread.join();
		}
	} catch (const std::system_error&) { }

	if (thread_pool) {
		thread_pool->join();
		thread_pool.reset();
	}
}


void
Scheduler::add(const TaskType& task, std::chrono::time_point<std::chrono::system_clock> wakeup)
{
	if (running != 0) {
		auto wt = scheduler_queue.add(task, time_point_to_ullong(wakeup));
		task->wakeup_time = wt;

		bool notify;
		auto nwt = next_wakeup_time.load();
		do {
			notify = nwt > wt;
		} while (notify && !next_wakeup_time.compare_exchange_weak(nwt, wt));

		if (notify) {
			wakeup_signal.notify_one();
		}
	}
}


void
Scheduler::run_one(TaskType& task)
{
	L_INFO_HOOK_LOG("Scheduler::run_one", this, "Scheduler::run_one()");

	if (!task->cleared_at) {
		if (task->clear()) {
			if (thread_pool) {
				try {
					thread_pool->enqueue(task);
				} catch (const std::logic_error&) { }
			} else {
				task->run();
			}
		}
	}
}


void
Scheduler::run()
{
	set_thread_name(name);

	std::mutex mtx;
	std::unique_lock<std::mutex> lk(mtx);

	auto nwt = next_wakeup_time.load();

	while (running != 0) {
		if (--running < 0) {
			running = -1;
		}

		auto now = std::chrono::system_clock::now();
		auto wt = time_point_to_ullong(now + (running < 0 ? 5s : 100ms));

		TaskType* task;

		if ((task = scheduler_queue.peep()) && *task) {
			wt = (*task)->wakeup_time;
		}

		next_wakeup_time.compare_exchange_strong(nwt, wt);
		while (nwt > wt && !next_wakeup_time.compare_exchange_weak(nwt, wt));

		L_INFO_HOOK_LOG("Scheduler::run::loop", this, "Scheduler::run()::loop - now:%llu, next_wakeup_time:%llu", time_point_to_ullong(now), next_wakeup_time.load());
		wakeup_signal.wait_until(lk, time_point_from_ullong(next_wakeup_time.load()));

		while ((task = scheduler_queue.next())) {
			if (*task) {
				run_one(*task);
				(*task).reset();
			}
		}

		if (running >= 0) {
			break;
		}
	}
}
