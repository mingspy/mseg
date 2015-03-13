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
#include <map>

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
#define NO_MEMPOOL 0
#define CELL_ON_MEMPOOL 1
#define HEADER_ON_MEMPOOL 2
#define ON_MEMPOOL 3

template<typename T>
class SparseVector
{
public:
    struct Header{
        unsigned int using_mempool:2;
        int _size:15;
        int _cap:15;
    };
    struct Cell {
        int id;
        T val;
    };
protected:
    Header _header;
    Cell * _cells;

public:
    explicit SparseVector()
    {
        init();
    }

    ~SparseVector()
    {
        clear();
    }

    SparseVector(const SparseVector & refer)
    {
        init();
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
        assert(index < size());
        return _cells[index].id;
    }

    /*
     * index is the direct index of cell.
     */
    inline T & getVal(int index) const
    {
        assert(index < size());
        return _cells[index].val;
    }

    inline void setVal(int position, const T & val)
    {
        assert(position < size());
        _cells[position].val = val;
    }

    inline T sum() const
    {
        T _sum = (T)0;
        for(int i = 0; i < size(); i++) {
            _sum += _cells[i].val;
        }

        return _sum;
    }

    inline int size()const
    {
        return _header._size;
    }

    inline int bytes() const{
        return sizeof(SparseVector) + size() * sizeof(Cell);
    }
    inline unsigned int using_mempool() const
    {
        return _header.using_mempool;
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
    void setValById(int id, const T & value)
    {
        int index = locateId(id);
        if ((index >= 0) && (_cells[index].id == id)) {
            _cells[index].val = value;
        } else {
            // need insert a new value after index.
            index ++; // now insert at index.
            Cell * tmp = reinterpret_cast<Cell *>( malloc((size() + 1)*sizeof(Cell)));
            if(size() > 0) {
                // move old data before(include) index.
                memcpy(tmp, _cells, index * sizeof(Cell));
                if(index  < size() ) {
                    memcpy(tmp + index + 1, _cells + index,(size() - index) * sizeof(Cell));
                }
                if (using_mempool()&CELL_ON_MEMPOOL == 0)
                    free(_cells);
            }

            tmp[index].id = id;
            tmp[index].val = value;
            _cells = tmp;
            _header._size ++;
            _header.using_mempool &= HEADER_ON_MEMPOOL;
        }
    }

    int addAttrFreq(int id, const T & inc)
    {
        T s = getValById(id) + inc;
        setValById(id, s);
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
    inline T getValById(int id) const
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

    void merge( const SparseVector & refer)
    {
        if(refer.size() == 0) return;
        if(size() == 0) {
            copyOf(refer);
            return;
        }
        vector<int> ids;
        for( int i = 0; i < size(); i++) {
            if(_cells[i].val != (T)0 && find(ids.begin(), ids.end(), _cells[i].id) == ids.end()) {
                ids.push_back(_cells[i].id);
            }
        }
        for( int i = 0; i < refer.size(); i++) {
            if(refer._cells[i].val != (T)0 && find(ids.begin(), ids.end(), refer._cells[i].id) == ids.end()) {
                ids.push_back(refer._cells[i].id);
            }
        }
        sort(ids.begin(), ids.end());
        Cell * tmp = reinterpret_cast<Cell *>(malloc(sizeof(Cell)*ids.size()));
        for(int i = 0; i < ids.size(); i++) {
            tmp[i].id = ids[i];
            tmp[i].val = getValById(tmp[i].id) + refer.getValById(tmp[i].id);
        }
        if (using_mempool()&CELL_ON_MEMPOOL == 0) free(_cells);
        _cells = tmp;
        _header._size = ids.size();
        _header.using_mempool &= HEADER_ON_MEMPOOL;
    }

    friend ostream & operator<< (ostream & out, const SparseVector & ins)
    {
        out<<"{size:"<<ins.size()<<", vals:[";
        for(int i = 0; i < ins._size; i++) {
            out<<"("<<ins._cells[i].id<<","<<ins._cells[i].val<<"),";
        }
        out<<"]}";
        return out;
    }

    static SparseVector * read(ifstream & inf)
    {
        SparseVector * sparse = new SparseVector();
        if (!inf.read(reinterpret_cast<char *>( sparse),sizeof(SparseVector))) {
            return false;
        }
        if (sparse->size()) {
            sparse->_cells = reinterpret_cast<Cell *>(malloc(sparse->size() * sizeof(Cell)));
            if (!inf.read(reinterpret_cast<char *>( sparse->_cells), sparse->size() * sizeof(Cell))) {
                return false;
            }
        }
        return sparse;
    }

    static SparseVector * read(char* pool)
    {
        SparseVector * sparse =  reinterpret_cast<SparseVector *>(pool);
        if (sparse->size()) {
            sparse->_cells = reinterpret_cast<Cell *>(pool + sizeof(SparseVector));
        }
        sparse->_header.using_mempool = ON_MEMPOOL;
        return sparse;
    }

    static bool write(ofstream & outf, const SparseVector * sparse) 
    {
        if (!outf.write(reinterpret_cast<char *> (const_cast<SparseVector *>(sparse)),sizeof(SparseVector))) {
            return false;
        }
        if (sparse->size()) {
            if (!outf.write(reinterpret_cast<char *>(const_cast<Cell *>(sparse->_cells)), sparse->size() * sizeof(Cell))) {
                return false;
            }
        }
        return true;
    }

    static void collect(SparseVector * sp){
        if (sp->using_mempool() & ON_MEMPOOL == NO_MEMPOOL){
            delete sp;
        }else if(sp->using_mempool() & ON_MEMPOOL == HEADER_ON_MEMPOOL){
            sp->clear();
        }
    }
    inline void clear()
    {
        if (using_mempool()&CELL_ON_MEMPOOL == 0 &&_cells) free(_cells);
        init();
    }
private:
    inline void init()
    {
        _header.using_mempool = NO_MEMPOOL;
        _header._size = 0;
        _header._cap = 0;
        _cells = NULL;
    }
    /**
     * Locates the greatest index that is not greater than the given index.
     * @return the internal index of the id. Returns -1 if not found.
     */
    int locateId(int id) const
    {
        int min = 0, max = size() - 1;
        if (max == -1) {
            return -1;
        }

        // Binary search
        while (min<=max&&(_cells[min].id <= id) && (_cells[max].id >= id)) {
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
        for(int i = 0; i < refer.size(); i++) {
            if(refer._cells[i].val != (T)0) {
                num ++;
            }
        }
        Cell * tmp = NULL;
        if(num > 0) {
            tmp = reinterpret_cast<Cell *>(malloc(num * sizeof(Cell)));
            int k = 0;
            for(int i = 0; i < refer.size(); i++) {
                if(refer._cells[i].val != (T)0) {
                    tmp[k] = refer._cells[i];
                    k++;
                }
            }
        }

        if (using_mempool()&CELL_ON_MEMPOOL == 0&&_cells) {
            free(_cells);
        }
        _cells = tmp;
        _header._size = num;
        _header.using_mempool &= HEADER_ON_MEMPOOL;
    }

};

template<typename T>
class SparseList
{
public:
    struct Header{
        unsigned int using_mempool:2;
        int _size:15;
        int _cap:15;
    };
    struct Cell {
        int id;
        T val;
        struct Cell * prev;
        struct Cell * next;
        Cell():id(0),val(0),prev(0),next(0){}
    };

    class iterator{
        Cell * cur;
    public:
        iterator(Cell * ptr):cur(ptr){}
        Cell * next(){
            return cur->next;
        }
        Cell * operator->(){
            return cur;
        }
        void operator++(){
            cur = cur->next;
        }
        void operator++(int){
            cur = cur->next;
        }
        bool operator == (const iterator & refer){
            return this->cur == refer.cur;
        }
        bool operator != (const iterator & refer){
            return this->cur != refer.cur;
        }
        static const iterator & empty(){
            static const iterator EMPTY(NULL);
            return EMPTY;
        }
    };
protected:
    Header _header;
    Cell * _first;
    Cell * _last;

public:
    explicit SparseList()
    {
        init();
    }

    ~SparseList()
    {
    }

    SparseList(const SparseList & refer)
    {
        init();
        copyOf(refer);
    }

    SparseList & operator=(const SparseList & refer)
    {
        if(&refer != this) {
            copyOf(refer);
        }
        return *this;
    }
    iterator begin() const{
        return iterator(_first);
    }
    const iterator & end(){
        return iterator::empty();
    }
    /**
     * Returns the id of the attribute stored at the given position.
     * @param position the position
    inline int getId(int index) const
    {
        assert(index < size());
        Cell * cur = _first;
        for ( int i = 0; i < index && cur; i++){
            cur = cur->next;
        }
        return cur->id;
    }
     */

    /*
     * index is the direct index of cell.
    inline T & getVal(int index) const
    {
        assert(index < size());
        Cell * cur = _first;
        for ( int i = 0; i < index && cur; i++){
            cur = cur->next;
        }
        return cur->val;
    }

    inline void setVal(int index, const T & val)
    {
        assert(index < size());
        Cell * cur = _first;
        for ( int i = 0; i < index && cur; i++){
            cur = cur->next;
        }
        cur->val = val;
    }

    */
    inline T sum() const
    {
        T _sum = (T)0;
        const Cell * cur = _first;
        for(; cur; cur = cur->next) {
            _sum += cur->val;
        }

        return _sum;
    }

    inline int size()const
    {
        return _header._size;
    }

    inline Cell & operator[](int index){
        Cell * cur = _first;
        for ( int i = 0; i < index && cur; i++){
            cur = cur->next;
        }
        return *cur;
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
    void setValById(int id, const T & value)
    {
        Cell * prev, *next;
        Cell * tmp = find_cell(id, &prev, &next);
        if (tmp){
            tmp->val = value;
            return;
        }

        tmp = new Cell();
        tmp->id = id;
        tmp->val = value;
        tmp->next = next;
        tmp->prev = prev;
        _header._size ++;

        if (prev){
            prev->next = tmp;
        }else{
            _first = tmp;
        }

        if(next){
            next->prev = tmp;
        }else{
            _last = tmp;
        }

    }

    int addAttrFreq(int id, const T & inc)
    {
        T s = getValById(id) + inc;
        setValById(id, s);
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
    inline T getValById(int id) const
    {
        Cell * cur = _first;
        for ( ;cur && cur->id < id; cur = cur->next){
        }
        if (cur && cur->id == id){
            return cur->val;
        }
        return 0;
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
        Cell * cur, * prev, *next;
        cur = find_cell(id,prev,next);
        if(cur){
            old_value = cur->val;
            if(prev){
                prev->next = next;
            }else{
                _first = next;
            }
            if(next){
                next->prev = prev;
            }else{
                _last = prev;
            }
            _header._size --;
            delete cur;
        }
        return old_value;
    }

    void merge( const SparseList & refer)
    {
        Cell * cur = _first;
        Cell * prev = NULL;
        Cell * refer_cur = refer._first;
        while(cur && refer_cur){
            if(cur->id == refer_cur->id){
                cur->val += refer_cur->val;
                prev = cur;
                cur = cur->next;
                refer_cur = refer_cur->next;
            }else if (cur->id < refer_cur->id){
                prev = cur;
                cur = cur->next;
            }else{
                Cell * tmp = new Cell();
                tmp->id = refer_cur->id;
                tmp->val = refer_cur->val;
                tmp->next = cur;
                tmp->prev = prev;
                cur->prev = tmp;
                if(!prev){
                    _first = tmp;
                }else{
                    prev->next = tmp;
                }
                prev = tmp;
                refer_cur = refer_cur->next;
                _header._size ++;
            }
        }
        while(refer_cur){
                Cell * tmp = new Cell();
                tmp->id = refer_cur->id;
                tmp->val = refer_cur->val;
                tmp->prev = prev;
                if(!prev){
                    _first = tmp;
                } else{
                    prev->next = tmp;
                }
                prev = tmp;
                refer_cur = refer_cur->next;
                _header._size ++;
                _last = prev;
        }
    }

    friend ostream & operator<< (ostream & out, const SparseList & ins)
    {
        out<<"{size:"<<ins.size()<<", vals:[";
        const Cell * cur = ins._first;
        while(cur){
            out<<"("<<cur->id<<","<<cur->val<<"),";
            cur = cur->next;
        }
        out<<"]}";
        return out;
    }

private:
    inline Cell * find_cell(int id, Cell **prev, Cell **next) const{
        Cell * cur = _first; 
        *prev = NULL;
        *next = NULL;
        while(cur && cur->id < id){
            *prev = cur;
            cur = cur->next;
        }
        if (cur && cur->id == id){
            *next = cur->next;
            return cur;
        }

        return NULL;
    }
    inline void init()
    {
        _header.using_mempool = NO_MEMPOOL;
        _header._size = 0;
        _header._cap = 0;
        _first = NULL;
        _last = NULL;
    }

    void clear(){
        while(_first){
            _last = _first->next;
            delete _first;
            _first = _last;
        }
    }
    void copyOf(const SparseList & refer)
    {
        const Cell * cur = refer._first;
        Cell * prev = NULL;
        while(cur){
            Cell * tmp = new Cell(*cur);
            tmp->prev = prev;
            if(prev) prev->next = tmp;
            else _first = tmp;
            prev = tmp;
            cur = cur->next;
        }
        _header = refer._header;
    }

};


template<class T>
class Matrix
{
    mutable map<int, SparseList<T> > _matrixs;
public:
    inline SparseList<T> & operator[](int row) const
    {
        return _matrixs[row];
    }
    inline T & get(int row,int col) const
    {
        return _matrixs[row].getAttrVal(col);
    }
    inline void set(int row,int col, T & val) const
    {
        return _matrixs[row].setValById(col,val);
    }

    void clear(){return _matrixs.clear();}
     map<int, SparseList<T> > & getMeta(){return _matrixs;}
};

const int FIX_MATRIX_COL = 10;
const int FIX_MATRIX_ROW = 1000;
template<class T>
class FixedMatrix
{
public:
    struct Cell{int id; T val;};
    struct Row{
        Row():_size(0){}
        int _size;
        Cell _cells[FIX_MATRIX_COL];

        inline int size() const {return _size;}
        Cell & operator[](int col)
        {
            return _cells[col];
        }
        const Cell & operator[](int col) const
        {
            return _cells[col];
        }
        inline void push_back(int id, const T & val){
            _cells[_size].id = id;
            _cells[_size++].val = val;
            if(_size == FIX_MATRIX_COL){
                cout<<*this<<endl;
            }
            assert(_size < FIX_MATRIX_COL);
        }

        friend ostream & operator<<(ostream & out, const Row & row){
            for ( int j = 0; j < row.size(); j ++){ 
                out<<"("<<row[j].id<<","<<row[j].val<<")";
            }
            return out;
        }
    };
public:
    FixedMatrix():_rowsize(0){
    }
    Row & operator[](int row) 
    {
        return _rows[row];
    }
    const Row & operator[](int row) const 
    {
        return _rows[row];
    }
    inline void push_back(int row,int id, const T & val)
    {
        _rows[row].push_back(id,val);
        if (row >= _rowsize){
            _rowsize = row + 1;
        }
        assert (_rowsize <= FIX_MATRIX_ROW);
    }

    void clear(){
        for(int i = 0; i < _rowsize; i++){
            _rows[i]._size = 0;
        }
        _rowsize = 0;
    }
    inline int size() const {
        return _rowsize;
    }
    friend ostream & operator<<(ostream & out, const FixedMatrix & matrix){
        for ( int j = 0; j < matrix.size(); j ++){ 
            out<<j<<":"<<matrix[j]<<endl;
        }
        return out;
    }
private:
    Row _rows[FIX_MATRIX_ROW];
    int _rowsize;
};

}

