#pragma once

// The build phases use three pieces of the Microsoft Concurrency Runtime. Windows keeps ppl.h;
// everywhere else the same interface is provided here on top of the standard library.

#if !defined(IXR_WINDOWS)

#include <algorithm>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

namespace concurrency
{
	// Splits the range between the hardware threads and runs the body for every index.
	template <class Body>
	void parallel_for(size_t Begin, size_t End, Body Functor)
	{
		if (End <= Begin)
		{
			return;
		}

		const size_t Count = End - Begin;

		unsigned Threads = std::thread::hardware_concurrency();
		if (Threads == 0)
		{
			Threads = 1;
		}
		Threads = (unsigned)std::min<size_t>(Threads, Count);

		const size_t Chunk = (Count + Threads - 1) / Threads;

		std::vector<std::thread> Workers;
		Workers.reserve(Threads - 1);

		for (unsigned Thread = 1; Thread < Threads; ++Thread)
		{
			const size_t From = Begin + Thread * Chunk;
			const size_t To = std::min(End, From + Chunk);
			if (From >= To)
			{
				break;
			}

			Workers.emplace_back([&Functor, From, To]()
			{
				for (size_t Index = From; Index < To; ++Index)
				{
					Functor(Index);
				}
			});
		}

		const size_t MainTo = std::min(End, Begin + Chunk);
		for (size_t Index = Begin; Index < MainTo; ++Index)
		{
			Functor(Index);
		}

		for (std::thread& Worker : Workers)
		{
			Worker.join();
		}
	}

	// One value per thread, merged once the work is done.
	template <class T>
	class combinable
	{
	public:
		T& local()
		{
			const std::thread::id ThisThread = std::this_thread::get_id();

			std::lock_guard<std::mutex> Guard(Lock);
			for (auto& Pair : Values)
			{
				if (Pair.first == ThisThread)
				{
					return Pair.second;
				}
			}

			Values.emplace_back(ThisThread, T());
			return Values.back().second;
		}

		template <class Functor>
		void combine_each(Functor Body)
		{
			std::lock_guard<std::mutex> Guard(Lock);
			for (auto& Pair : Values)
			{
				Body(Pair.second);
			}
		}

	private:
		std::mutex Lock;
		std::vector<std::pair<std::thread::id, T>> Values;
	};

	// A vector that may be appended to from several threads at once.
	template <class T>
	class concurrent_vector
	{
	public:
		void reserve(size_t Count) { Data.reserve(Count); }
		void resize(size_t Count) { Data.resize(Count); }
		size_t size() const { return Data.size(); }
		bool empty() const { return Data.empty(); }

		T& operator[](size_t Index) { return Data[Index]; }
		const T& operator[](size_t Index) const { return Data[Index]; }

		void push_back(const T& Value)
		{
			std::lock_guard<std::mutex> Guard(*Lock);
			Data.push_back(Value);
		}

		typename std::vector<T>::iterator begin() { return Data.begin(); }
		typename std::vector<T>::iterator end() { return Data.end(); }
		typename std::vector<T>::const_iterator begin() const { return Data.begin(); }
		typename std::vector<T>::const_iterator end() const { return Data.end(); }

	private:
		// Shared so the vector stays copyable and movable, which containers of it require.
		std::shared_ptr<std::mutex> Lock = std::make_shared<std::mutex>();
		std::vector<T> Data;
	};
}

#endif // !IXR_WINDOWS
