#pragma once

#include <final_platform_layer.hpp>

namespace fs {
	namespace mem {
		struct MemoryBlock {
			size_t size;
			size_t offset;
			void *base;
		};

		inline MemoryBlock AllocateMemoryBlock(const size_t size, const size_t alignment = 16) {
			MemoryBlock result = {};
			result.size = size;
			result.base = fpl::memory::MemoryAlignedAllocate(size, alignment);
			return(result);
		}

		inline void ReleaseMemoryBlock(MemoryBlock *block) {
			if (block->base != nullptr) {
				fpl::memory::MemoryAlignedFree(block->base);
			}
			*block = {};
		}

		template <typename T>
		inline T *PushSize(MemoryBlock *block, const size_t size, const bool clear = true) {
			assert((block->offset + size) <= block->size);
			void *ptr = (void *)((uint8_t *)block->base + block->offset);
			block->offset += size;
			if (clear) {
				fpl::memory::ClearMemory(ptr, size);
			}
			T *result = (T*)ptr;
			return(result);
		}

		template <typename T>
		inline T *PushStruct(MemoryBlock *block, const bool clear = true) {
			T *result = PushSize<T>(block, sizeof(T), clear);
			return(result);
		}

		template <typename T>
		inline T *PushArray(MemoryBlock *block, const size_t count, const bool clear = true) {
			T *result = PushSize<T>(block, count * sizeof(T), clear);
			return(result);
		}

		inline void PopSize(MemoryBlock *block, const size_t size) {
			assert((block->offset - size) >= 0);
			block->offset -= size;
		}
	};
};