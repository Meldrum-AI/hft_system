#include "SharedContainers.hpp"

namespace tdsys {

    template <typename T, size_t SIZE>
    SharedContainers<T, SIZE>::SharedContainers()
        : lock(ATOMIC_FLAG_INIT)
    {
    }

    template <typename T, size_t SIZE>
    SharedContainers<T, SIZE>::~SharedContainers()
    {
    }

    template <typename T, size_t SIZE>
    void SharedContainers<T, SIZE>::setAllUsed() {
        bitMap.flip();
    }

    template <typename T, size_t SIZE>
    bool SharedContainers<T, SIZE>::setNewContainer(const std::string& code) {
        spinLock();
        if (auto result = hashTable.find(code.c_str()); result) {
            spinUnlock();
            return true;
        }
        size_t slot;
        if (!findAvailableSlot(slot)) {
            spinUnlock();
            return false;
        }
        hashTable.insert(code.c_str(), slot);
        spinUnlock();
        return true;
    }

    template <typename T, size_t SIZE>
    bool SharedContainers<T, SIZE>::setNewContainer(const std::string& code, T** target) {
        spinLock();
        if (auto result = hashTable.find(code.c_str()); result) {
            *target = &containerSection[*result];
            spinUnlock();
            return true;
        }
        size_t slot;
        if (!findAvailableSlot(slot)) {
            spinUnlock();
            return false;
        }
        hashTable.insert(code.c_str(), slot);
        *target = &containerSection[slot];
        spinUnlock();
        return true;
    }

    template <typename T, size_t SIZE>
    T* SharedContainers<T, SIZE>::getContainerAddress(const std::string& code) {
        auto p = hashTable.find(code.c_str());
        return p ? &containerSection[*p] : nullptr;
    }

    template <typename T, size_t SIZE>
    bool SharedContainers<T, SIZE>::getContainerAddress(const std::string& code, T** target) {
        auto p = hashTable.find(code.c_str());
        if (!p) return false;
        *target = &containerSection[*p];
        return true;
    }

    template <typename T, size_t SIZE>
    bool SharedContainers<T, SIZE>::discardContainer(const std::string& code) {
        spinLock();
        auto p = hashTable.find(code.c_str());
        if (!p) {
            spinUnlock();
            return false;
        }
        size_t slot = *p;
        T* ptr = &containerSection[slot];
        ptr->~T();
        new(ptr) T();
        bitMap.reset(slot);
        hashTable.erase(code.c_str());
        spinUnlock();
        return true;
    }

    template <typename T, size_t SIZE>
    bool SharedContainers<T, SIZE>::findAvailableSlot(size_t& target) {
        if (bitMap.all()) return false;
        for (size_t i = 0; i < SIZE; ++i) {
            if (!bitMap.test(i)) {
                bitMap.set(i);
                target = i;
                return true;
            }
        }
        return false;
    }

    template <typename T, size_t SIZE>
    void SharedContainers<T, SIZE>::spinLock() {
        while (lock.test_and_set(std::memory_order_acquire))
            std::this_thread::yield();
    }

    template <typename T, size_t SIZE>
    void SharedContainers<T, SIZE>::spinUnlock() {
        lock.clear(std::memory_order_release);
    }

    template <typename T, size_t SIZE>
    T& SharedContainers<T, SIZE>::operator[](size_t offset) {
        return containerSection[offset];
    }

    template <typename T, size_t SIZE>
    auto SharedContainers<T, SIZE>::begin() noexcept {
        return containerSection.begin();
    }

    template <typename T, size_t SIZE>
    auto SharedContainers<T, SIZE>::end() noexcept {
        return containerSection.end();
    }

    template <typename T, size_t SIZE>
    auto SharedContainers<T, SIZE>::cbegin() const noexcept {
        return containerSection.cbegin();
    }

    template <typename T, size_t SIZE>
    auto SharedContainers<T, SIZE>::cend() const noexcept {
        return containerSection.cend();
    }

    template <typename T, size_t SIZE>
    SharedContainers<T, SIZE>* SharedContainersManager::getSharedContainersCreate(const std::string& syscode) {
        auto it = infoMap.find(syscode);
        if (it != infoMap.end())
            return static_cast<SharedContainers<T, SIZE>*>(it->second.mptr);

        int fd = shm_open(syscode.c_str(), O_CREAT | O_RDWR, 0666);
        if (fd == -1)
            throw std::runtime_error("shm_open create failed: " + syscode);

        size_t ps = getpagesize();
        size_t sz = (sizeof(SharedContainers<T, SIZE>) + ps - 1) & ~(ps - 1);
        ftruncate(fd, sz);

        void* ptr = mmap(NULL, sz, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        new (ptr) SharedContainers<T, SIZE>();
        infoMap[syscode] = { true, fd, ptr, sz };
        return static_cast<SharedContainers<T, SIZE>*>(ptr);
    }

    template <typename T, size_t SIZE>
    SharedContainers<T, SIZE>* SharedContainersManager::getSharedContainers(const std::string& syscode) {
        auto it = infoMap.find(syscode);
        if (it != infoMap.end())
            return static_cast<SharedContainers<T, SIZE>*>(it->second.mptr);

        int fd = shm_open(syscode.c_str(), O_RDWR, 0666);
        if (fd == -1)
            throw std::runtime_error("shm_open open failed: " + syscode);

        size_t ps = getpagesize();
        size_t sz = (sizeof(SharedContainers<T, SIZE>) + ps - 1) & ~(ps - 1);
        void* ptr = mmap(NULL, sz, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        infoMap[syscode] = { false, fd, ptr, sz };
        return static_cast<SharedContainers<T, SIZE>*>(ptr);
    }

    void SharedContainersManager::releaseSharedContainers(const std::string& syscode) {
        auto it = infoMap.find(syscode);
        if (it == infoMap.end()) return;

        munmap(it->second.mptr, it->second.size);
        close(it->second.fd);
        if (it->second.isCreated)
            shm_unlink(syscode.c_str());

        infoMap.erase(it);
    }

    void SharedContainersManager::releaseAllSharedContainers() {
        for (auto& p : infoMap) {
            munmap(p.second.mptr, p.second.size);
            close(p.second.fd);
            if (p.second.isCreated)
                shm_unlink(p.first.c_str());
        }
        infoMap.clear();
    }

    template class SharedContainers<int, 128>;
    template class SharedContainers<char, 128>;

}