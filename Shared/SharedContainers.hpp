#pragma once
#include "Memory/FixedHashTable.hpp"
#include <atomic>
#include <bitset>
#include <array>
#include <string>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <unordered_map>

namespace tdsys {

    template<typename T, size_t SZ = 128>
    class SharedContainers {
    private:
        std::bitset<SZ> bitMap;
        FixedHashTable<32, size_t, 2 * SZ> hashTable;
        std::array<T, SZ> containerSection;
        std::atomic_flag lock;

        bool findAvailableSlot(size_t& target);
        void spinLock();
        void spinUnlock();

    public:
        SharedContainers();
        ~SharedContainers();

        void setAllUsed();
        bool setNewContainer(const std::string& code);
        bool setNewContainer(const std::string& code, T** target);
        bool discardContainer(const std::string& code);
        T* getContainerAddress(const std::string& code);
        bool getContainerAddress(const std::string& code, T** target);

        T& operator[](size_t offset);
        auto begin() noexcept;
        auto end() noexcept;
        auto cbegin() const noexcept;
        auto cend() const noexcept;
    };

    struct SharedContainersInfo {
        bool isCreated;
        int fd;
        void* mptr;
        size_t size;
    };

    class SharedContainersManager {
    private:
        std::unordered_map<std::string, SharedContainersInfo> infoMap;
    public:
        template<typename T, size_t SIZE>
        SharedContainers<T, SIZE>* getSharedContainersCreate(const std::string& syscode);

        template<typename T, size_t SIZE>
        SharedContainers<T, SIZE>* getSharedContainers(const std::string& syscode);

        void releaseSharedContainers(const std::string& syscode);
        void releaseAllSharedContainers();
    };

}
