#pragma once

namespace fs {
	namespace concurrency {
		template <typename T>
		class ConcurrentQueue {
		private:
			//std::deque<T> _queue;
			//std::mutex _queueMutex;
		public:
			inline void Push(T value) {
				//std::unique_lock<std::mutex> lock(_queueMutex);
				//_queue.push_back(value);
			}

			inline bool Pop(T &out) {
			#if 0
				std::unique_lock<std::mutex> lock(_queueMutex, std::defer_lock);
				lock.lock();
				if (!_queue.empty()) {
					T value = _queue.front();
					_queue.pop_front();
					out = value;
				}
				lock.unlock();
			#endif
				return false;
			}
		};

	};
};