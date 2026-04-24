#pragma once
#include <array>
#include <atomic>
#include <condition_variable>
#include <cstddef>
#include "Shared/SharedContainers.hpp"
#include "TradingSysType.h"
#include <functional>
#include <mutex>
#include <semaphore.h>
#include <thread>
#include <tuple>
#include <utility>

namespace tdsys {
    
    //SPSC LockFree Queue
    template <typename T, size_t SIZE=127>
    class SPSCQueue{

        static constexpr size_t LENGTH=1ull << (std::bit_width(SIZE));
        static constexpr size_t MODULUS_MASK=LENGTH-1;
        //index
        private:
            alignas(64) std::atomic<size_t> head;
            alignas(64) std::atomic<size_t> tail;
            alignas(64) size_t slowHead;
            size_t slowTail;
            std::array<T, LENGTH> dataSection;
        public:
            SPSCQueue();
            ~SPSCQueue();
            bool popFromHead(T&);
            template<typename U>
            bool insertAtTail(U&& );
            bool isEmpty() const;
            bool isFull() const;

    };

    //MPSC LockFree Queue
    template <typename T, size_t SIZE=127>
    class MPSCQueue{

        static constexpr  size_t LENGTH=1ull << (std::bit_width(SIZE));
        static constexpr  size_t MODULUS_MASK=LENGTH-1;
        static constexpr  size_t INDEX_BITS=16;
        static constexpr  size_t VERSION_BITS=sizeof(size_t)*CHAR_BIT-INDEX_BITS;
        static constexpr  size_t VERSION_SHIFT = INDEX_BITS;
        static constexpr  size_t INDEX_MASK = (1ULL << INDEX_BITS) - 1;

        struct WrappedData{

            std::atomic<bool> isCompleted;
            T data;

        };

        private:
            alignas(64) std::atomic<size_t> head;
            //tail包含版本号，以修正一个小概率bug
            alignas(64) std::atomic<size_t> versionedTail;
            alignas(64) std::array<WrappedData, LENGTH> dataSection;
        public:
            MPSCQueue();
            ~MPSCQueue();
            bool popFromHead(T&);
            template<typename U>
            bool insertAtTail(U&& );
            bool isEmpty() const;
            bool isFull() const;

    };

    //SPMC LockFree Queue
    template <typename T, size_t SIZE=127>
    class SPMCQueue{

        static constexpr  size_t LENGTH=1ull << (std::bit_width(SIZE));
        static constexpr  size_t MODULUS_MASK=LENGTH-1;
        static constexpr  size_t INDEX_BITS=16;
        static constexpr  size_t VERSION_BITS=sizeof(size_t)*CHAR_BIT-INDEX_BITS;
        static constexpr  size_t VERSION_SHIFT = INDEX_BITS;
        static constexpr  size_t INDEX_MASK = (1ULL << INDEX_BITS) - 1;

        private:
            alignas(64) std::atomic<size_t> versionedHead;
            alignas(64) std::atomic<size_t> tail;
            alignas(64) std::array<T, LENGTH> dataSection;
        public:
            SPMCQueue();
            ~SPMCQueue();
            bool popFromHead(T&);
            template<typename U>
            bool insertAtTail(U&&);
            bool isEmpty() const;
            bool isFull() const;

    };

    template<typename T,size_t SIZE=127>
    class MsgQueue{

        //特殊的MPSC Queue，设置条件变量唤醒机制，规避热等待带来的性能损耗

        static constexpr  size_t LENGTH=1ull << (std::bit_width(SIZE));
        static constexpr  size_t MODULUS_MASK=LENGTH-1;
        static constexpr  size_t INDEX_BITS=16;
        static constexpr  size_t VERSION_BITS=sizeof(size_t)*CHAR_BIT-INDEX_BITS;
        static constexpr  size_t VERSION_SHIFT = INDEX_BITS;
        static constexpr  size_t INDEX_MASK = (1ULL << INDEX_BITS) - 1;

        struct WrappedData{

            std::atomic<bool> isCompleted;
            T data;

        };

        private:
            alignas(64) std::atomic<size_t> head;
            //tail包含版本号，以修正一个小概率bug
            alignas(64) std::atomic<size_t> versionedTail;
            alignas(64) sem_t semaphore;
            std::array<WrappedData, LENGTH> dataSection;
        public:
            MsgQueue();
            ~MsgQueue();
            bool consumerRead(T&);
            bool readFromHead(T&);
            template<typename U>
            bool producerInsert(U&& );
            bool isEmpty() const;
            bool isFull() const;

    };

    //TradingSys ThreadPool
    template<size_t N=3>
    class ThreadPool{

        private:
            std::array<std::thread, N> threads;
            SPMCQueue<std::function<void()>> tasks;
            std::mutex task_mtx;
	        std::condition_variable cv;
            bool stop = false;
            ThreadPool();
        public:
            ~ThreadPool();
            ThreadPool<N>& getInstance();
            template<typename Func,typename ...Args>
            bool submitTask(Func&& f, Args&& ...args);

    };
    
    using SystemMessageQueue=MsgQueue<SystemMessage>;//using SPSC Queue for systematic communication should be strictly individual-to-individual;
    using SharedSystemMsgQueues=SharedContainers<SystemMessageQueue,TDSYS_SYSMSG_QUEUE_NUM>;

    /*******************************模板实现**************************************/

    template <typename T, size_t SIZE>
    SPSCQueue<T,SIZE>::SPSCQueue(){

        head.store(0);
        tail.store(0);
        slowTail = tail.load(std::memory_order_relaxed);
		slowHead = head.load(std::memory_order_relaxed);

    }

    template <typename T,size_t SIZE>
    bool SPSCQueue<T,SIZE>::popFromHead(T& target){

        size_t localHead=head.load(std::memory_order_relaxed);
        if(localHead!=slowTail 
            || 
            localHead!=(slowTail=tail.load(std::memory_order_acquire))
        ){
            T* source=&dataSection[localHead];
            target=std::move(*source);
            localHead=(localHead+1) & MODULUS_MASK;
            head.store(localHead,std::memory_order_release);
            return true;
        }
        return false;

    }

    template <typename T, size_t SIZE>
    template<typename U>
    bool SPSCQueue<T,SIZE>::insertAtTail(U&& source){

        size_t localTail = tail.load(std::memory_order_relaxed);
        if(
            ((localTail + 1) & MODULUS_MASK) != slowHead 
            || 
            ((localTail + 1) & MODULUS_MASK) != (slowHead = head.load(std::memory_order_acquire))
        ){
            T* target=&dataSection[localTail];
            new (target) T(std::forward<U>(source));
            localTail=(localTail+1) & MODULUS_MASK;
            tail.store(localTail,std::memory_order_release);
            return true;
        }
        return false;

    }

    template <typename T, size_t SIZE>
    bool SPSCQueue<T,SIZE>::isEmpty()
    const{

        if(head.load(std::memory_order_acquire)==tail.load(std::memory_order_acquire)){
            return true;
        }
        return false;

    }

    template <typename T, size_t SIZE>
    bool SPSCQueue<T,SIZE>::isFull()
    const{

        if(
            ((head.load(std::memory_order_acquire)+1) & MODULUS_MASK)
            ==
            tail.load(std::memory_order_acquire)
        ){
            return true;
        }
        return false;

    }

    template <typename T,size_t SIZE>
    MPSCQueue<T,SIZE>::MPSCQueue(){

        head.store(0,std::memory_order_relaxed);
        versionedTail.store(0,std::memory_order_relaxed);

    }

    template <typename T,size_t SIZE>
    MPSCQueue<T,SIZE>::~MPSCQueue(){

    }

    template<typename T,size_t SIZE>
    bool MPSCQueue<T,SIZE>::popFromHead(T& target){

        size_t localHead=head.load(std::memory_order_relaxed);
        if(localHead!=(versionedTail.load(std::memory_order_acquire) & INDEX_MASK)){
            WrappedData* source=&dataSection[localHead];
            if(source->isCompleted.load(std::memory_order_acquire)!=true){
               return false;
            }
            target=std::move(source->data);
            source->isCompleted.store(false,std::memory_order_release);
            localHead=(localHead+1) & MODULUS_MASK;
            head.store(localHead,std::memory_order_release);
            return true;    
        }
        return false;

    }
    
    template<typename T,size_t SIZE>
    template<typename U>
    bool MPSCQueue<T,SIZE>::insertAtTail(U&& source){

        size_t localVersionedTail;
        size_t version;
        size_t newLocalTail;
        size_t oldLocalTail;
        size_t newVersionedTail;
        while(true){
            localVersionedTail=versionedTail.load(std::memory_order_acquire);
            oldLocalTail=versionedTail & INDEX_MASK;
            version=(versionedTail>>VERSION_SHIFT);
            if(((oldLocalTail + 1) & MODULUS_MASK) != head.load(std::memory_order_acquire)){
                newLocalTail=(oldLocalTail+1) & MODULUS_MASK;
                newVersionedTail = (version + 1) << VERSION_SHIFT | newLocalTail; 
                if(versionedTail.compare_exchange_strong(
                        localVersionedTail,newVersionedTail,std::memory_order_acq_rel
                    )
                ){
                    WrappedData* target=&dataSection[oldLocalTail];
                    new (&(target->data)) T(std::forward<U>(source));
                    target->isCompleted.store(true,std::memory_order_release);
                    return true;
                }
                else{
                    continue;
                }
            }
            else{
                break;
            }
        }
        return false;

    }

    template<typename T,size_t SIZE>
    bool MPSCQueue<T,SIZE>::isEmpty()
    const{

        if(
            ((head.load(std::memory_order_acquire)) & INDEX_MASK)
            ==
            ((versionedTail.load(std::memory_order_acquire)) & INDEX_MASK )
        ){
            return true;
        }
        return false;

    }

    template<typename T,size_t SIZE>
    bool MPSCQueue<T,SIZE>::isFull()
    const{

        if(
            (((versionedTail.load(std::memory_order_acquire) &INDEX_MASK )+1) &MODULUS_MASK)
            == 
            (head.load(std::memory_order_acquire) & INDEX_MASK)
        ){
            return true;
        }
        return false;

    }

    template<typename T,size_t SIZE>
    SPMCQueue<T,SIZE>::SPMCQueue(){

        versionedHead.store(0,std::memory_order_relaxed);
        tail.store(0,std::memory_order_relaxed);

    }

    template<typename T,size_t SIZE>
    SPMCQueue<T,SIZE>::~SPMCQueue(){

    }

    template<typename T,size_t SIZE>
    bool SPMCQueue<T,SIZE>::popFromHead(T& target){

        size_t localVersionedHead;
        size_t version;
        size_t newLocalHead;
        size_t oldLocalHead;
        size_t newVersionedHead;
        while(true){
            localVersionedHead=versionedHead.load(std::memory_order_acquire);
            oldLocalHead=localVersionedHead & INDEX_MASK;
            if(oldLocalHead==tail.load(std::memory_order_relaxed)){
                newLocalHead=(oldLocalHead+1) & MODULUS_MASK;
                version=localVersionedHead>>VERSION_SHIFT;
                newVersionedHead=(version+1)<<VERSION_SHIFT | newLocalHead;
                if(versionedHead.compare_exchange_strong(
                    localVersionedHead,
                    newVersionedHead,
                    std::memory_order_acq_rel
                    )
                ){
                    T* source= &dataSection[oldLocalHead];
                    target=std::move(*source);
                    return true;
                }
                else{
                    continue;
                }
                
            }
            else{
                break;
            }
        }
        return false;

    }

    template<typename T,size_t SIZE>
    template<typename U>
    bool SPMCQueue<T,SIZE>::insertAtTail(U&& source){

        size_t localTail = tail.load(std::memory_order_relaxed);
        if(
            ((localTail + 1) & MODULUS_MASK)
            != 
            ((versionedHead.load(std::memory_order_acquire)) & INDEX_MASK)
        ){
            T* target=&dataSection[localTail];
            new (target) T(std::forward<U>(source));
            localTail=(localTail+1) & MODULUS_MASK;
            tail.store(localTail,std::memory_order_release);
            return true;
        }
        return false;

    }

    template<typename T,size_t SIZE>
    bool SPMCQueue<T,SIZE>::isEmpty()
    const{

        if(
            (versionedHead.load(std::memory_order_acquire) & INDEX_MASK)
            ==
            tail.load(std::memory_order_acquire)
        ){
            return true;
        }
        return false;

    }

    template<typename T,size_t SIZE>
    bool SPMCQueue<T,SIZE>::isFull()
    const{

        if(
            ((tail.load(std::memory_order_acquire)+1)&MODULUS_MASK)
            ==
            (versionedHead.load(std::memory_order_acquire) & INDEX_MASK)
        ){
            return true;
        }
        return false;
        
    }

    template<typename T,size_t SIZE>
    MsgQueue<T,SIZE>::MsgQueue(){

        head.store(0,std::memory_order_relaxed);
        versionedTail.store(0,std::memory_order_relaxed);
        sem_init(&this->semaphore, 1, 0);

    }

    template<typename T,size_t SIZE>
    MsgQueue<T,SIZE>::~MsgQueue(){

        sem_destroy(&this->semaphore);

    }

    template<typename T,size_t SIZE>
    bool MsgQueue<T,SIZE>::readFromHead(T& target){

        size_t localHead=head.load(std::memory_order_relaxed);
        if(localHead!=(versionedTail.load(std::memory_order_acquire) & INDEX_MASK)){
            WrappedData* source=&dataSection[localHead];
            if(source->isCompleted.load(std::memory_order_acquire)!=true){
               return false;
            }
            target=std::move(source->data);
            source->isCompleted.store(false,std::memory_order_release);
            localHead=(localHead+1) & MODULUS_MASK;
            head.store(localHead,std::memory_order_release);
            return true;    
        }
        return false;

    }

    template<typename T,size_t SIZE>
    template<typename U>
    bool MsgQueue<T,SIZE>::producerInsert(U&& source){
                
        size_t localVersionedTail;
        size_t version;
        size_t newLocalTail;
        size_t oldLocalTail;
        size_t newVersionedTail;
        while(true){
            localVersionedTail=versionedTail.load(std::memory_order_acquire);
            oldLocalTail=versionedTail & INDEX_MASK;
            version=(versionedTail>>VERSION_SHIFT);
            if(((oldLocalTail + 1) & MODULUS_MASK) != head.load(std::memory_order_acquire)){
                newLocalTail=(oldLocalTail+1) & MODULUS_MASK;
                newVersionedTail = (version + 1) << VERSION_SHIFT | newLocalTail; 
                if(versionedTail.compare_exchange_strong(
                        localVersionedTail,newVersionedTail,std::memory_order_acq_rel
                    )
                ){
                    WrappedData* target=&dataSection[oldLocalTail];
                    new (&(target->data)) T(std::forward<U>(source));
                    target->isCompleted.store(true,std::memory_order_release);
                    //插完按门铃
                    // readerCV.notify_one();
                    sem_post(&semaphore);
                    return true;
                }
                else{
                    continue;
                }
            }
            else{
                break;
            }
        }
        return false;

    }

    template<typename T,size_t SIZE>
    bool MsgQueue<T,SIZE>::isEmpty()
    const{

        if(
            ((head.load(std::memory_order_acquire)) & INDEX_MASK)
            ==
            ((versionedTail.load(std::memory_order_acquire)) & INDEX_MASK )
        ){
            return true;
        }
        return false;

    }

    template<typename T,size_t SIZE>
    bool MsgQueue<T,SIZE>::isFull()
    const{

        if(
            (((versionedTail.load(std::memory_order_acquire) &INDEX_MASK )+1) &MODULUS_MASK)
            == 
            (head.load(std::memory_order_acquire) & INDEX_MASK)
        ){
            return true;
        }
        return false;

    }

    template<typename T,size_t SIZE>
    bool MsgQueue<T,SIZE>::consumerRead(T& target){
        
        //单线程消费者期待
        // std::unique_lock<std::mutex> lock(cvlock);
        // readerCV.wait(lock,[this]()->bool { return !this->isEmpty();});
        do{
            sem_wait(&semaphore);
            if(this->isEmpty()){
                continue;
            }
        }while(false);
        
        return readFromHead(target);

    }

    template<size_t N>
    ThreadPool<N>::ThreadPool(){

        auto workFunc
            =
        [this](){  
            while(true){
                {
                    std::unique_lock<std::mutex> lock(task_mtx);
                    cv.wait(lock,[this](){return tasks.isEmpty();});
                }
                if(stop==true && tasks.isEmpty()){
                    break;
                }
                std::function<void()> task;
                tasks.popFromHead(task);
                task();
            }
        };

        for(size_t i=0;i<N;i++){
            new(&threads[i]) std::thread(workFunc);
        }

    }

    template<size_t N>
    ThreadPool<N>& ThreadPool<N>::getInstance(){
        
        static ThreadPool<N> instance;
        return instance;
    
    }

    template<size_t N>
    ThreadPool<N>::~ThreadPool(){        
        
        stop=true;
        for(auto& iter : threads){
            if(iter.joinable()){
                iter.join();
            }
        }

    }

    template<size_t N>
    template<typename Func,typename ...Args>
    bool ThreadPool<N>::submitTask(Func&& f, Args&& ...args){

        auto taskFunc
            =
        [func=std::forward<Func>(f), args=std::make_tuple(std::forward<Args>(args)...)]
        ()
        mutable{
            std::apply(func,std::move(args));
        };

        {
            std::lock_guard<std::mutex> lock(task_mtx);
            return tasks.insertAtTail(taskFunc);
        }

    }

}//namespace tdsys