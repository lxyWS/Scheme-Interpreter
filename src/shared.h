//
// Created by 维Sir on 24-12-12.
//

#ifndef SHARED_H
#define SHARED_H


#include <utility>

template<typename T>
class SharedPtr {
// private:
public:
    T *_p;
    int *cnt; // 计数

    void myInfer() {
        if (cnt != nullptr) {
            --*this->cnt;
            if (*this->cnt == 0) {
                delete _p;
                delete cnt;
                _p = nullptr;
                cnt = nullptr;
            }
        }
    }


    explicit SharedPtr(T *temp) {
        _p = temp;
        if (_p != nullptr) {
            cnt = new int{1};
        } else {
            cnt = nullptr;
        }
    }

    // 默认构造函数
    SharedPtr() {
        _p = nullptr;
        cnt = nullptr;
    }

    // 析构函数
    ~SharedPtr() {
        myInfer();
    }

    // 拷贝构造
    SharedPtr(const SharedPtr &temp) noexcept{
        // 如果要复制的对象已经不存在了，*cnt无需++
        if (temp.cnt != nullptr) {
            ++*temp.cnt;
        }
        this->_p = temp._p;
        this->cnt = temp.cnt;
    }

    // 移动构造
    SharedPtr(SharedPtr &&temp) noexcept {
        this->_p = temp._p;
        this->cnt = temp.cnt;
        temp._p = nullptr;
        temp.cnt = nullptr;
    }

    // 拷贝赋值
    SharedPtr &operator=(const SharedPtr &temp) {
        if (this != &temp) {
            myInfer();

            this->_p = temp._p;
            this->cnt = temp.cnt;
            if (this->cnt != nullptr) {
                ++*this->cnt;
            }
        }

        return *this;
    }

    // 移动赋值
    SharedPtr &operator=(SharedPtr &&sp) noexcept {
        if (this != &sp) {
            myInfer();

            this->cnt = sp.cnt;
            this->_p = sp._p;
            sp._p = nullptr;
            sp.cnt = nullptr;
        }

        return *this;
    }

    int use_count() const noexcept{
        if (cnt != nullptr) {
            return *cnt;
        } else {
            return 0;
        }
    }

    T *get() const noexcept{
        return _p;
    }

    // 重载*
    T &operator*() const noexcept {
        return *_p;
    }

    // 重载->
    T *operator->() const noexcept{
        return _p;
    }

     explicit operator bool() const noexcept{
        return _p != nullptr;
    }

    void reset() noexcept{
        myInfer();

        _p = nullptr;
        cnt = nullptr;
    }

    void reset(T *temp) noexcept{
        myInfer();

        _p = temp;
        if (_p != nullptr) {
            cnt = new int(1);
        } else {
            cnt = nullptr;
        }
    }
};

template<typename T, typename... Tlist>
SharedPtr<T> make_shared(Tlist &&... rest) {
    return SharedPtr<T>(new T(std::forward<Tlist>(rest)...));
}


#endif //SHARED_H
