/*
 * Copyright (C) 2014  mingspy@163.com
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#pragma once

#include <iostream>
using namespace std;
namespace mingspy
{
/*
* 获取堆数组左孩子节点下标， 下标0处也存放数据
*/
inline int Left(int i)
{
    return (i<<2) + 1;
}

/*
* 获取堆数组右孩子节点下标， 下标0处也存放数据
*/
inline int Right(int i)
{
    return (i<<2) + 2;
}

/*
* 获取堆数组父节点下标， 下标0处也存放数据
*/
inline int Parent(int i)
{
    return (i+1)>>2;
}

/*
*最小堆,A[Parent(i)]>=A[i]
*  @see http://blog.csdn.net/mingspy/article/details/8582822
*/
template<typename T>
class MinHeap
{
public:
    MinHeap(int cap = 0):_size(0), _capability(cap)
    {
        A = new T[cap];
    }

    MinHeap(const T * arr, int len):_size(len), _capability(len)
    {
        A = new T[len];
        memcpy(A, arr, len * sizeof(T));
    }

    MinHeap(const MinHeap & r):_size(r._size), _capability(r._capability)
    {
        A = new T[r._capability];
        memcpy(A, r.A, r._capability * sizeof(T));
    }
    ~MinHeap()
    {
        delete [] A;
        A = 0;
    }

    /*
    *   Max-heapify(A, i) A[1...n]
    *   l <- LEFT(i)
    *   r <- RIGHT(i)
    *   if l<= heap-size[A] and A[l] > A[i]
    *       then largest <- l
    *       else largest <- i
    *   if r <= heapsize(A) and A[r] > A[largest]
    *       then largest <- r
    *   if largest != i
    *       then exchange A[i]<->A[largest]
    *           Max-heapify(A, largest)
    */
    void heapify(int i)
    {
        int l = Left(i);
        int r = Right(i);
        int smallest = i;
        if ( l < _size && A[l] < A[smallest]) {
            smallest = l;
        }

        if ( r < _size && A[r] < A[smallest]) {
            smallest = r;
        }

        if ( i != smallest) {
            swap(i, smallest);
            heapify(smallest);
        }
    }

    void heapify_up(int i)
    {
        int parent;
        while (i > 0 && A[i] < A[(parent=Parent(i))]) {
            swap(i, parent);
            i = parent;
        }
    }

    /*
    * build-Max-Heap(A)  A[1...n]
    *   heap-size[A] <- length[A]
    *   for i <- foot[length[A]/2] downto 1
    *       do Max-heapify(A,i);
    */
    void build()
    {
        if ( _size == 0) return;
        for (int i = (_size - 1) / 2; i >= 0; i--) {
            heapify(i);
        }
    }

    /*
    *Heapsort(A) A[1...n]
    *   build-Max-Heap(A)
    *   for i <- length[A] down to 2  //
    *       do exchange A[1] <->A[i]  //
    *       heap-size[A] <- heap-size[A] - 1 //
    *       Max-heapify(A, i)         //
    */
    void sort()
    {
        if(_size == 0) return;
        size_t tmpSize = _size;
        build();
        for (int i = --_size; i > 0; i--) {
            swap(i, 0);
            --_size;
            heapify(0);
        }

        _size = tmpSize;

        // reverse
        int mid = _size / 2;
        for(int i = 0; i < mid ; i++) {
            swap(i, _size  - 1 - i);
        }
    }

    bool empty()
    {
        return _size == 0;
    }
    /*
    * Heap-Maximum(A)
    *   return A[1]
    */

    /*
     * get the minest element
     */
    T & top()
    {
        assert (_size > 0); //
        return A[0];
    }

    T & operator[](int idx)
    {
        assert (_size > idx); //
        return A[idx];
    }

    /*
    *Heap-Extract-Max(A) A[1...n]
    *   if heap-size[A] < 1
    *       then error "heap underflow"
    *   max <- A[1]
    *   A[1] <-A[heap-size[A]]
    *   heap-size[A] <- heap-size[A] - 1
    *   Max-heapify(A, 1)
    *   return max;
    */
    T pop()
    {
        assert (_size > 0);
        T min = A[0]; A[0] = A[--_size]; heapify(0); return min;
    }

    /*
    *Heap-Increase-Key(A, i, key)
    *   if key < A[i]
    *       then error "new key is smaller"
    *   A[i] <- Key
    *   while i > 1 and A[Parent(i)] < A[i]
    *       do exchange A[i] <-> A[Parent(i)]
    *       i <- Parent(i)
    */
    void increase_key(int i, const T & key)
    {
        assert(i>=0 && i < _size);
        A[i] = key;
        heapify_up(i);
    }

    /*
    *Max-Heap-insert(A,key)
    *   heap-size[A] <- heap-size[A] + 1
    *   A[heap-size[A]] <- -INFINITY
    *   Heap-Increase-Key(A, heap-size[A], key)
    */
    void add(const T & key)
    {
        assert(_size < _capability);
        A[_size] = key;
        heapify_up(_size);
        _size ++;

    }

    size_t size() const
    {
        return _size;
    }

    void resize(int cap){
        if( cap > _capability){
            T * tmp = new T[cap];
            if(A){
                memcpy(tmp, A, _capability * sizeof(T));
                delete [] A;
            }
            A = tmp;
        }
        _capability = cap;
    }

    /*
    * add key only if key is letter than
    * A[_size] when the heap is full.
    * useful when do NPath.
    */
    void add_if_small(const T & key)
    {
        if(_size < _capability) {
            A[_size] = key;
            heapify_up(_size);
            _size ++;
        } else if(key < A[_size - 1]) {
            A[_size - 1] = key;
            heapify_up(_size - 1);
        }
    }
private:
    inline void swap(int i, int j)
    {
        T t = A[i];
        A[i] = A[j];
        A[j] = t;
    }

    size_t _size; //
    size_t _capability; //
    T * A;
};
}

