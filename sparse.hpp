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
#include <ostream>
#include <fstream>
#include <cassert>
#include <vector>
#include <algorithm>

using namespace std;
namespace mingspy
{
    
    /*
     * SparseVector is a vector used to save sparse data.
     * For example, a vector has thousands of features,
     * however, for a instance only a few features has
     * values. Assume vector space {x1,x2,x3....x1000},here
     * an instance is only have (x1 = 10, x10 = 11, x20 = 99),
     * to present this instance, we can use a SparseVector:
     *     indices:  1, 10, 20
     *     values : 10, 11, 99
     */
    template<typename T = int>
    class SparseVector
    {
    public:
        struct Cell{
            int id;
            T val;
        };
    protected:
        int _size;
        Cell * _cells;
        mutable T _sum;
        mutable bool modified;
        
    public:
        explicit SparseVector()
        {
            _size = 0;
            _cells = NULL;
            _sum = (T)0;
            modified = false;
        }
        
        ~SparseVector()
        {
            clear();
        }
        
        explicit SparseVector(const SparseVector & refer)
        {
            copyOf(refer);
        }
        
        SparseVector & operator=(const SparseVector & refer)
        {
            if(&refer != this) {
                copyOf(refer);
            }
            return *this;
        }
        /**
         * Returns the id of the attribute stored at the given position.
         * @param position the position
         */
        inline int getId(int index) const
        {
            assert(index < _size);
            return _cells[index].id;
        }
        
        /*
         * index is the direct index of cell.
         */
        inline T & getVal(int index) const
        {
            assert(index < _size);
            return _cells[index].val;
        }
        
        inline void setVal(int position, const T & val)
        {
            assert(position < _size);
            _cells[position].val = val;
            modified = true;
        }
        
        T sum() const
        {
            if(_sum == 0 || modified) {
                _sum = (T)0;
                for(int i = 0; i < _size; i++) {
                    _sum += _cells[i].val;
                }
                modified = false;
            }
            
            return _sum;
        }
        
        /**
         * Sets a specific value in the instance to the given value (internal
         * floating-point format). Performs a deep copy of the vector of attribute
         * values before the value is set.
         *
         * @param attIndex the attribute's index
         * @param value the new attribute value (If the corresponding attribute is
         *          nominal (or a string) then this is the new value's index as a
         *          double).
         */
        void setAttrVal(int id, const T & value)
        {
            int index = locateId(id);
            
            if ((index >= 0) && (_cells[index].id == id)) {
                _cells[index].val = value;
            } else {
                // need insert a new value after index.
                index ++; // now insert at index.
                Cell * tmp = reinterpret_cast<Cell *>( malloc((_size + 1)*sizeof(Cell)));
                if(_size > 0) {
                    // move old data before(include) index.
                    memcpy(tmp, _cells, index * sizeof(Cell));
                    if(index  < _size ) {
                        memcpy(tmp + index + 1, _cells + index,(_size - index) * sizeof(Cell));
                    }
                    free(_cells);
                }
                
                tmp[index].id = id;
                tmp[index].val = value;
                _cells = tmp;
                _size ++;
            }
            modified = true;
        }
        
        int addAttrFreq(int id, const T & inc){
            T s = getAttrValue(id) + inc;
            setAttrVal(id, s);
            return s;
        }
        /**
         * Returns an instance's attribute value in internal format.
         *
         * @param attIndex the attribute's index
         * @return the specified value as a double (If the corresponding attribute is
         *         nominal (or a string) then it returns the value's index as a
         *         double).
         */
        inline T getAttrValue(int id) const
        {
            int index = locateId(id);
            if ((index >= 0) && (_cells[index].id == id)) {
                return _cells[index].val;
            }
            return (T)0;
        }
        
        /**
         * Remove the attribute at given attribute index. Here only mark the attribute
         * as the default value (0).
         *
         * @param attIndex the attribute' index
         * @return the specified value as a double (If the corresponding attribute is
         *         nominal (or a string) then it returns the value's index as a
         *         double).
         */
        T removeAttr(int id)
        {
            T old_value = 0;
            int index = locateId(id);
            if ((index >= 0) && (_cells[index].id == id)) {
                old_value = _cells[index].val;
                _cells[index].val = (T)0;;
            }
            return old_value;
        }
        
        void merge( const SparseVector & refer){
            if(refer._size == 0) return;
            if(_size == 0){
                copyOf(refer);
                return;
            }
            vector<int> ids;
            for( int i = 0; i < _size; i++){
                if(_cells[i].val != (T)0 && find(ids.begin(), ids.end(), _cells[i].id) == ids.end()){
                    ids.push_back(_cells[i].id);
                }
            }
            for( int i = 0; i < refer._size; i++){
                if(refer._cells[i].val != (T)0 && find(ids.begin(), ids.end(), refer._cells[i].id) == ids.end()){
                    ids.push_back(refer._cells[i].id);
                }
            }
            sort(ids.begin(), ids.end());
            Cell * tmp = reinterpret_cast<Cell *>(malloc(sizeof(Cell)*ids.size()));
            _sum = (T)0;
            for(int i = 0; i < ids.size(); i++){
                tmp[i].id = ids[i];
                tmp[i].val = getAttrValue(tmp[i].id) + refer.getAttrValue(tmp[i].id);
                _sum += tmp[i].val;
            }
            free(_cells);
            _cells = tmp;
            _size = ids.size();
            modified = false;
        }
        
        friend ostream & operator<< (ostream & out, const SparseVector & ins)
        {
            out<<"{size:"<<ins._size<<", vals:[";
            for(int i = 0; i < ins._size; i++) {
                out<<"("<<ins._cells[i].id<<","<<ins._cells[i].val<<"),";
            }
            out<<"]}";
            return out;
        }
        
        bool read(ifstream & inf){
            clear();
            if (!inf.read(reinterpret_cast<char *>( &_size),sizeof(int))) {
                return false;
            }
            if (_size) {
                _cells = reinterpret_cast<Cell *>(malloc(_size * sizeof(Cell)));
                if (!inf.read(reinterpret_cast<char *>( _cells), _size * sizeof(Cell))) {
                    return false;
                }
            }
            return true;
        }
        
        bool write(ofstream & outf) const{
            if (!outf.write(reinterpret_cast<char *> (const_cast<int *>(&_size)),sizeof(int))) {
                return false;
            }
            if (_size) {
                if (!outf.write(reinterpret_cast<char *>(const_cast<Cell *>(_cells)), _size * sizeof(Cell))) {
                    return false;
                }
            }
            return true;
        }
    private:
        inline void clear(){
            if (_cells) free(_cells);
            _cells = NULL;
            _size = 0;
            _sum = (T)0;
            modified = false;
        }
        /**
         * Locates the greatest index that is not greater than the given index.
         * @return the internal index of the id. Returns -1 if not found.
         */
        int locateId(int id) const
        {
            int min = 0, max = _size - 1;
            if (max == -1) { return -1;}
            
            // Binary search
            while ((_cells[min].id <= id) && (_cells[max].id >= id)) {
                int current = (max + min) / 2;
                if (_cells[current].id > id) {
                    max = current - 1;
                } else if (_cells[current].id < id) {
                    min = current + 1;
                } else {
                    return current;
                }
            }
            
            if (_cells[max].id < id) {
                return max;
            } else {
                return min - 1;
            }
        }
        void copyOf(const SparseVector & refer)
        {
            int num = 0;
            for(int i = 0; i < refer._size; i++) {
                if(refer._cells[i].val != (T)0) {
                    num ++;
                }
            }
            Cell * tmp = NULL;
            if(num > 0) {
                tmp = reinterpret_cast<Cell *>(malloc(num * sizeof(Cell)));
                int k = 0;
                for(int i = 0; i < refer._size; i++) {
                    if(refer._cells[i].val != (T)0) {
                        tmp[k] = refer._cells[i];
                        k++;
                    }
                }
            }
            
            if (_cells) {
                free(_cells);
            }
            _cells = tmp;
            _sum = refer._sum;
            modified = refer.modified;
            _size = num;
        }
        
    };
    
}

