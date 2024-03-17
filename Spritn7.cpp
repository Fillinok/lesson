#pragma once

#include <cassert>
#include <initializer_list>
#include <cstddef>
#include <array>
#include <stdexcept>
#include <iostream>
#include <utility>


struct ReserveProxyObj {
    public:
    ReserveProxyObj(size_t n_capacity)
    {
        ncapacity_= n_capacity;
    }
    size_t ncapacity_=0;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}


template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept {
    size_=0;
    capacity_=0;
    items_=nullptr;
    }

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) {
        size_=size;
        capacity_=size;        
        items_ = new Type [size_];
        std::fill(begin(), end(), Type{});
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) {
        size_=size;
        capacity_=size;
        items_ = new Type [size_];        
        std::fill(begin(), end(), value);       
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) {        
        size_=init.size();
        capacity_=init.size();        
        items_ = new Type [size_];        
        std::copy(init.begin(), init.end(), items_);                       
    }
    
    SimpleVector(const SimpleVector& other) {
        items_ = new Type [other.capacity_];
        std::copy(other.begin(), other.end(), items_);        
        size_ = other.size_;
        capacity_ = other.capacity_;        
    }
    
    SimpleVector(ReserveProxyObj temp)
    {
        capacity_=temp.ncapacity_;
        size_=0;
        items_=nullptr;
    }
    

    SimpleVector& operator=(const SimpleVector& rhs) {        
        if (this == &rhs)
        {
            return *this;
        }
        SimpleVector tmp(rhs);
        swap(tmp);
        return *this;
    }   
    
    SimpleVector(SimpleVector&& other) {        
        items_ = std::exchange(other.items_, nullptr);
        size_ = std::exchange(other.size_, 0);
        capacity_ = std::exchange(other.capacity_, 0);
    }
    
    SimpleVector& operator=(SimpleVector&& other) {        
        items_ = std::exchange(other.items_, nullptr);
        size_ = std::exchange(other.size_, 0);
        capacity_ = std::exchange(other.capacity_, 0);
        return *this;
    }
    
    
    ~SimpleVector()
    {
        delete[] items_;        
    }
    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;        
    }
    
    void Reserve(size_t new_capacity)
    {
        if(new_capacity > capacity_)
        {
            Type* temp = new Type[new_capacity];
            std::move(begin(),end(),temp);
            delete[] items_;
            items_ = temp;
            capacity_=new_capacity;
        }
    }
    
    
    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) 
    {
        if(capacity_ > 0)
        {
            if(size_ == capacity_)
            {
                size_t temp = size_;
                Resize(size_*2);    
                items_[temp]=item;
                size_ = temp+1;
            }
            else
            {
                items_[size_]=item;
                size_++;
            }
        }
        else
        {
            Resize(1);
            items_[0]=item;            
        }
        
    }

    void PushBack(Type&& item) 
    {
        if(capacity_ > 0)
        {
            if(size_ == capacity_)
            {
                size_t temp = size_;
                Resize(size_*2);    
                items_[temp]=std::move(item);
                size_ = temp+1;
            }
            else
            {
                items_[size_]=std::move(item);
                size_++;
            }
        }
        else
        {
            Resize(1);
            items_[0]=std::move(item);            
        }
        
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        
        if(capacity_ == 0)
        {            
            PushBack(value);            
            return items_;
        }
        Iterator iter = std::next(begin(),pos-begin());
        size_t citer = std::distance(begin(),iter);
        if(size_ == capacity_)
        {        
            Type* temp = new Type[capacity_*2];                
            std::copy(begin(),iter,temp);
            temp[citer]=value;
            std::copy(iter,end(),&temp[std::distance(begin(),iter)+1]);
            delete[] items_;
            items_=temp;
            capacity_*=2;
            size_++;           
            return std::next(begin(),citer);
        }
        else
        {               
            std::copy(iter,end(),std::next(iter,1)); 
            size_++;                
            items_[citer] = value;            
            return std::next(begin(),citer);
        }                
    }
    
    Iterator Insert(ConstIterator pos, Type&& value) {
        
        if(capacity_ == 0)
        {            
            PushBack(std::move(value));            
            return items_;
        }
        Iterator iter = std::next(begin(),pos-begin());
        size_t citer = std::distance(begin(),iter);
        if(size_ == capacity_)
        {        
            Type* temp = new Type[capacity_*2];             
            std::move(begin(),iter,temp);           
            temp[citer]=std::move(value);
            std::move(iter,end(),&temp[std::distance(begin(),iter)+1]);          
            delete[] items_;            
            items_=temp;
            capacity_*=2;
            size_++;           
            return std::next(begin(),citer);
        }
        else
        {         
            std::move(iter,end(),std::next(iter,1)); 
            size_++;
            items_[citer] = std::move(value);            
            return std::next(begin(),citer);
        }                
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        size_--;       
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {    
        Iterator iter = nullptr;
        for(auto it = begin();it!=end();it++)
        {
            if(it == pos)
            {
                iter = it;
                break;
            }
        } 
        size_t citer = std::distance(begin(),iter);
        //std::copy(std::next(iter,1), end(), iter);        
        std::move(std::next(iter,1), end(), iter);
        size_--;        
        return std::next(begin(),citer);
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {        
        std::swap(items_, other.items_);        
        std::swap(size_, other.size_); 
        std::swap(capacity_, other.capacity_);        
    }
    
    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {        
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {        
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if(index >= size_)
        {
            throw std::out_of_range(".at() - out of range");
        }
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if(index >= size_)
        {
            throw std::out_of_range("const.at() - out of range");
        }
        return items_[index];        
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_=0;// Напишите тело самостоятельно
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if(new_size<=size_)
        {
            size_ = new_size;
        }
        else if(new_size > capacity_)
        {
            Type* temp = new Type[new_size];            
            std::move(begin(), end(), temp);
            delete[] items_;
            items_ = temp;
            //std::fill(&(items_[size_]), &(items_[new_size]), Type{});
            for(size_t i = size_;i<new_size;i++)
            {
                temp[i] = std::move(Type{});
            }
            size_=new_size;
            capacity_=new_size;
            
        }
        else if (new_size > size_ && new_size < capacity_)
        {
            //std::fill(end(), end()+(capacity_ - new_size), Type{});
            for(size_t i = size_;i<new_size;i++)
            {
                items_[i] = std::move(Type{});
            }
            size_=new_size;
        }
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
    if (size_ == 0) { return nullptr; }    
        return items_;
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
    if (size_ == 0) { return nullptr; }
        return &(items_[size_]);
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
    if (size_ == 0) { return nullptr; }
        return items_;
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
    if (size_ == 0) { return nullptr; }
        return &(items_[size_]);
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
    if (size_ == 0) { return nullptr; }
        return items_;
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
    if (size_ == 0) { return nullptr; }
        return &(items_[size_]);
    }
    private:
    Type* items_;
    size_t size_;
    size_t capacity_;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    if (lhs.GetSize()!= rhs.GetSize())
    {
        return false;
    }    
    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {    
    return !(lhs==rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {      
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {    
    return !(rhs < lhs) || rhs == lhs;
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {    
    return rhs<lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {    
    return !(lhs < rhs) || rhs == lhs;
} 
