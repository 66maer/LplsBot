#pragma once

#include <vector>
#include <string>
#include <map>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <mutex>
#include <queue>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>

namespace Mtools {


    /// <summary>
    /// 分割字符串（忽略空行）
    /// </summary>
    /// <param name="str">原始字符串</param>
    /// <param name="delims">分隔符，delims中的每一个字符都是分隔符</param>
    /// <returns>分割后的字符串组</returns>
    static std::vector<std::string> Split(const std::string& str, const std::string& delims) {
        std::vector<std::string> rtn;
        for (auto first = str.data(), second = str.data(), last = first + str.size();
            second != last && first != last; first = second + 1) {
            second = find_first_of(first, last, delims.cbegin(), delims.cend());
            if (first != second)
                rtn.emplace_back(first, second);
        }
        return rtn;
    }

    /// <summary>
    /// 分割字符串
    /// </summary>
    /// <param name="str">原始字符串</param>
    /// <param name="delims">分隔符</param>
    /// <param name="isSubStr">True: 将delims整体作为分隔符。 False:
    /// delims中的每一个字符都是分隔符</param> <param name="saveNul">True:
    /// 将保留空字符串</param> <returns>分割后的字符串组</returns>
    static std::vector<std::string> Split(const std::string& str, const std::string& delims,
        bool isSubStr, bool saveNul = false) {
        std::vector<std::string> rtn;
        if (isSubStr) {
            for (auto first = str.data(), second = str.data(), last = first + str.size();
                second != last && first != last; first = second + delims.size()) {
                auto it = str.find(delims, first - str.data());
                second = it == -1 ? last : &str[it];
                if (saveNul || first != second)
                    rtn.emplace_back(first, second);
            }
        }
        else {
            for (auto first = str.data(), second = str.data(), last = first + str.size();
                second != last && first != last; first = second + 1) {
                second = find_first_of(first, last, delims.cbegin(), delims.cend());
                if (saveNul || first != second)
                    rtn.emplace_back(first, second);
            }
        }
        return rtn;
    }



    template <typename T> class SafeQueue {
    public:
        SafeQueue() : eoq_(false) {}
        SafeQueue(SafeQueue const& other) {
            std::lock_guard<std::mutex> lk(other.mut_);
            que_ = other.que_;
            eoq_ = other.eoq_;
        }
        template <typename _InputIterator> SafeQueue(_InputIterator first, _InputIterator last) {
            for (auto it = first; it != last; ++it) {
                que_->emplace_back(it);
            }
            eoq_ = false;
        }
        SafeQueue(SafeQueue const&& other) {
            std::lock_guard<std::mutex> lk(other.mut_);
            que_ = std::move(other.que_);
        }
        void push(const T& new_value) {
            std::lock_guard<std::mutex> lk(mut_);
            que_.emplace_back(new_value);
            eoq_ = false;
            cond_.notify_one();
        }
        void push_front(const T& new_value) {
            std::lock_guard<std::mutex> lk(mut_);
            que_.emplace_front(new_value);
            eoq_ = false;
            cond_.notify_one();
        }
        // 等待直到有元素可以删除为止
        bool wait_and_pop(T* value) {
            std::unique_lock<std::mutex> lk(mut_);
            cond_.wait(lk, [this] { return eoq_ || !que_.empty(); });
            if (!que_.empty()) {
                *value = std::move(que_.front());
                que_.pop_front();
                return true;
            }
            return false;
        }
        std::shared_ptr<T> wait_and_pop() {
            std::unique_lock<std::mutex> lk(mut_);
            cond_.wait(lk, [this] { return eoq_ || !que_.empty(); });
            if (!que_.empty()) {
                std::shared_ptr<T> res(std::make_shared<T>(que_.front()));
                que_.pop_front();
                return res;
            }
            return nullptr;
        }
        //不管有没有队首元素直接返回
        bool try_pop(T* value) {
            std::lock_guard<std::mutex> lk(mut_);
            if (que_.empty())
                return false;
            *value = std::move(que_.front());
            que_.pop_front();
            return true;
        }
        std::shared_ptr<T> try_pop() {
            std::lock_guard<std::mutex> lk(mut_);
            if (que_.empty())
                return nullptr;
            std::shared_ptr<T> res(std::make_shared<T>(que_.front()));
            que_.pop_front();
            return res;
        }
        bool empty() const {
            std::lock_guard<std::mutex> lk(mut_);
            return que_.empty();
        }
        size_t size() const {
            std::lock_guard<std::mutex> lk(mut_);
            return que_.size();
        }
        void end_of_queue() {
            std::lock_guard<std::mutex> lk(mut_);
            eoq_ = true;
            cond_.notify_all();
        }
        void clear() {
            std::lock_guard<std::mutex> lk(mut_);
            que_.clear();
            eoq_ = false;
        }
        void CopyToVecAndClear(std::vector<T>* vec) {
            std::lock_guard<std::mutex> lk(mut_);
            for (auto it = que_.begin(); it != que_.end(); ++it) {
                vec->emplace_back(*it);
            }
            que_.clear();
            eoq_ = false;
        }

    private:
        mutable std::mutex      mut_;
        std::deque<T>           que_;
        std::condition_variable cond_;
        bool                    eoq_;
    };


}  // namespace QTtools
