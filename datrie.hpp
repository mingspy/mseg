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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <fstream>

#ifndef SIZE_MAX
#ifdef _WIN64
#define SIZE_MAX _UI64_MAX
#else
#define SIZE_MAX UINT_MAX
#endif
#endif

using namespace std;

namespace mingspy
{
    
    const int TRIE_INDEX_ERROR = 0;
    const int TRIE_CHILD_MAX = 256; // how many children a char can have in dat.
    
    template <class T>
    class Length {
    public: size_t operator()(const T *key) const
        { size_t i; for (i = 0; key[i] != (T)0; ++i) {} return i; }
    };
    
    template <> class Length<char> {
    public: size_t operator()(const char *key) const
        { return std::strlen(key); }
    };
    
    template  <class node_type_,  class node_u_type_,
    class array_type_, class array_u_type_,
    class length_func_ = Length<node_type_> >
    class DoubleArray
    {
        
    private:
        static const int DA_ROOT = 2;       // root address
        static const int DA_POOL_BEGIN = 3; // the first cell to save characters
        static const int DA_SIGNATURE = 0xfdeeabcd;
        
        /**
         * The address of free list header.<br>
         * The free cells are managed by link to a double-circular-link<br>
         * As r1r2...ri...rm are m free cells.<br>
         * base[1] = -rm     check[1] = -r1<br>
         * .....<br>
         * base[ri]= -r(i-1) check[ri] = -r(i+1)<br>
         * .....<br>
         * base[rm]= -r(m-1) check[ri] = -1<br>
         */
        static const int FREE_LIST_HEAD = 1;
        
        // How many cell it call allocate.
        static const int TRIE_INDEX_MAX = 0x0fffffff;
    private:
        struct Cell {
            array_type_ base; // save the first child's index
            array_type_ check;// save the father's index
        };
        Cell * _cell;
        int num_cells;
    public:
        
        explicit DoubleArray() { init(); }
        
        ~DoubleArray() { clear();}
        
        void clear() { free(_cell);  _cell = NULL; num_cells = 0;}
        inline array_type_ getRoot() const { return DA_ROOT; }
        
        inline array_type_ getBase(size_t position) const {
            return position < num_cells ? _cell[position].base : TRIE_INDEX_ERROR;
        }
        
        inline array_type_ getCheck(size_t position) const {
            return position < num_cells ? _cell[position].check : TRIE_INDEX_ERROR;
        }
        
        inline void setBase(size_t position, array_type_ value) {
            _cell[position].base = value;
        }
        
        inline bool isWorkable(array_type_ s, node_u_type_ c) const {
            return getCheck(getBase(s) + c) == s;
        }
        
        /**
         * Walk the double-array trie from state s, using input character c. If
         * there exists an edge from s with arc labeled c, this function returns
         * TRUE and s is updated to the new state. Otherwise, it returns FALSE and s
         * is left unchanged.
         * @param s : in-out, save state(c)'s base if walkable.
         * @param c : child c of state(s)
         * @return
         */
        inline bool walk(array_type_ * s, node_u_type_ c) const {
            array_type_ next = getBase(*s) + c;
            if (getCheck(next) == *s) {
                *s = next;
                return true;
            }
            return false;
        }
        
        /**
         * @brief Insert a branch from trie node
         * @param s   : the state to add branch to
         * @param c   : the character for the branch label
         * @return the index of the new node
         *         Insert a new arc labelled with character c from the trie node
         *         represented by index s in double-array. Note
         *         that it assumes that no such arc exists before inserting.
         */
        array_type_ insertBranch(array_type_ s, node_u_type_ c)
        {
            array_type_ base = getBase(s);
            array_type_ next;
            // base > 0 means s already has children.
            if (base > 0) {
                // if already there, do not actually insert.
                next = base + c;
                if (getCheck(next) == s) return next;
                
                // if (base + c) > TRIE_INDEX_MAX  means 'next' is overflow, or
                // cell[next] is not free, relocate to a free slot.
                if (base > TRIE_INDEX_MAX - c || !checkFreeCell(next)) {
                    // relocate BASE[s]
                    vector<node_u_type_> children;
                    findAllChildren(s, children);
                    children.push_back(c);
                    array_type_ new_base = findFreeBase(children);
                    if (TRIE_INDEX_ERROR == new_base) return TRIE_INDEX_ERROR;
                    relocateBase(s, new_base);
                    next = new_base + c;
                }
            } else {
                // s don't have a child yet.
                vector<node_u_type_> children;
                children.push_back(c);
                array_type_ new_base = findFreeBase(children);
                if (TRIE_INDEX_ERROR == new_base) return TRIE_INDEX_ERROR;
                setBase(s, new_base);
                next = new_base + c;
            }
            allocCell(next);
            setCheck(next, s);
            
            return next;
        }
        
        /**
         * @brief Prune the single branch
         * @param d     : the double-array structure
         * @param s     : the dangling state to prune off
         *            Prune off a non-separate path up from the final state  s. If
         *            s still has some children states, it does nothing. Otherwise,
         *            it deletes the node and all its parents which become
         *            non-separate.
         */
        void prune(array_type_ s) { pruneUpto(getRoot(), s); }
        
        /**
         * @brief Prune the single branch up to given parent
         * @param d   : the double-array structure
         * @param p   : the parent up to which to be pruned
         * @param s   : the dangling state to prune off
         *            Prune off a non-separate path up from the final state  s to
         *            the given parent  p. The pruning stop when either the
         *            parent  p is met, or a first non-separate node is found.
         */
        void pruneUpto(array_type_ p, array_type_ s) {
            while (p != s && !hasChildren(s)) {
                array_type_ parent = getCheck(s);
                freeCell(s);
                s = parent;
            }
        }
        
        
        bool hasChildren(array_type_ s) {
            array_type_ base = getBase(s);
            if (TRIE_INDEX_ERROR == base || base < 0) return false;
            array_type_ max_c = min(TRIE_CHILD_MAX,TRIE_INDEX_MAX - base);
            for (array_type_ c = 0; c < max_c; c++) {
                if (getCheck(base + c) == s) return true;
            }
            return false;
        }
        
        
        bool write(ofstream & outf) {
            outf.write(reinterpret_cast<char *>(&num_cells), sizeof(int));
            outf.write(reinterpret_cast<char *>(_cell), sizeof(Cell) * num_cells);
            return true;
        }
        
        bool read(ifstream & inf) {
            clear();
            if (!inf.read(reinterpret_cast<char *>(&num_cells), sizeof(int))){
                return false;
            }
            _cell = reinterpret_cast<Cell *>(malloc(sizeof(Cell) * num_cells));
            if (!inf.read(reinterpret_cast<char *>(_cell), sizeof(Cell) * num_cells)){
                return false;
            }
            return true;
        }
    private:
        void init() {
            _cell = (Cell *) malloc(DA_POOL_BEGIN * sizeof(Cell));
            num_cells = DA_POOL_BEGIN;
            _cell[0].base = DA_SIGNATURE;   // mark of datrie
            _cell[0].check = DA_POOL_BEGIN; // number of cells
            _cell[1].base = -1;             // address of last free cell
            _cell[1].check = -1;            // address of first free cell
            _cell[2].base = DA_POOL_BEGIN;  // set root's base at DA_POOL_BEGIN
            _cell[2].check = 0;
        }
        
        inline void setCheck(size_t position, array_type_ value) { _cell[position].check = value; }
        
        inline bool checkFreeCell(array_type_ s) { return extendPool(s) && getCheck(s) < 0; }
        
        /* allocate a cell from the free list
         *  by link the prev cell to next cell
         */
        inline void allocCell(size_t index) {
            array_type_ prev = -getBase(index);
            array_type_ next = -getCheck(index);
            setCheck(prev, -next);
            setBase(next, -prev);
        }
        
        void freeCell(size_t cell)
        {
            // find insertion point
            array_type_ i = -getCheck(FREE_LIST_HEAD);
            while (i != FREE_LIST_HEAD && i < cell) i = -getCheck(i);
            array_type_ prev = -getBase(i);
            
            // insert cell before i
            setCheck(cell, -i);
            setBase(cell, -prev);
            setCheck(prev, -cell);
            setBase(i, -cell);
        }
        
        bool extendPool(array_type_ to_index)
        {
            if (to_index < num_cells) return true;
            if (to_index <= 0 || TRIE_INDEX_MAX <= to_index) return false;
            
            array_type_ new_begin = num_cells;
            _cell = (Cell *)realloc(_cell, (to_index + 1)*sizeof(Cell));
            num_cells = to_index + 1;
            
            // initialize new free list, link one by one
            for (array_type_ i = new_begin; i < to_index; i++) {
                setCheck(i, -(i + 1));   // set next(i) = i+1
                setBase(i + 1, -i);      // set prev(i+1) = i
            }
            
            // merge the new circular list to the old
            array_type_ free_tail = -getBase(FREE_LIST_HEAD);
            setCheck(free_tail, -new_begin);
            setBase(new_begin, -free_tail);
            setCheck(to_index, -FREE_LIST_HEAD);
            setBase(FREE_LIST_HEAD, -to_index);
            _cell[0].check = to_index + 1; // update header cell
            
            return true;
        }
        
        
        void relocateBase(array_type_ s, array_type_ new_base)
        {
            array_type_ old_base = getBase(s);
            vector<node_u_type_> children;
            findAllChildren(s, children);
            
            for (int i = 0; i < children.size(); i++) {
                array_type_ child_old_idx = old_base + children[i];
                array_type_ child_new_idx = new_base + children[i];
                array_type_ child_base = getBase(child_old_idx);
                
                // allocate new next node and copy BASE value */
                allocCell(child_new_idx);
                setCheck(child_new_idx, s); // set s as children[i]'s father
                setBase(child_new_idx, child_base); // copy child's base
                
                // reset children[i]'s children point to child_new_idx
                if (child_base > 0) {
                    array_type_ max_c = min(TRIE_CHILD_MAX, TRIE_INDEX_MAX - child_base);
                    for (int c = 0; c < max_c; c++) {
                        if (getCheck(child_base + c) == child_old_idx)
                            setCheck(child_base + c, child_new_idx);
                    }
                }
                
                // free old cell
                freeCell(child_old_idx);
            }
            
            // finally, make BASE[s] point to new_base
            setBase(s, new_base);
        }
        
        int findFreeBase(const vector<node_u_type_> & children)
        {
            // find first free cell that is beyond the first symbol
            node_u_type_ first_child = children[0];
            array_type_ s = -getCheck(FREE_LIST_HEAD);
            while (s != FREE_LIST_HEAD && s < first_child + DA_POOL_BEGIN) {
                s = -getCheck(s);
            }
            if (s == FREE_LIST_HEAD) {
                for (s = first_child + DA_POOL_BEGIN;; ++s) {
                    if (!extendPool(s)) return TRIE_INDEX_ERROR;
                    if (getCheck(s) < 0) break;
                }
            }
            
            /* search for next free cell that fits the symbols set */
            while (!fitAllChildren(s - first_child, children)) {
                /* extend pool before getting exhausted */
                if (-getCheck(s) == FREE_LIST_HEAD) {
                    if (!extendPool(num_cells+children.size()))
                        return TRIE_INDEX_ERROR;
                }
                
                s = -getCheck(s);
            }
            
            return s - first_child;
        }
        
        /**
         * Check if the base can save all children.
         *
         * @param base
         * @param children
         * @return
         */
        bool fitAllChildren(array_type_ base, const vector<node_u_type_> & children)
        {
            for (int i = 0; i < children.size(); i++) {
                node_u_type_ sym = children[i];
                
                /*
                 * if (base + sym) > TRIE_INDEX_MAX which means it's overflow, or
                 * cell [base + sym] is not free, the symbol is not fit.
                 */
                if (base > TRIE_INDEX_MAX - sym || !checkFreeCell(base + sym))
                    return false;
            }
            return true;
        }
        
        /**
         * Find all children of s.<br>
         * @param s
         * @return
         */
        void findAllChildren(array_type_ s, vector<node_u_type_> & children)
        {
            array_type_ base = getBase(s);
            int max_c = min(TRIE_CHILD_MAX, TRIE_INDEX_MAX - base);
            for (int c = 0; c < max_c; c++) {
                if (getCheck(base + c) == s)
                    children.push_back((node_u_type_) c);
            }
        }
    };
    
}

namespace mingspy
{
    
    // A help class used to
    // make the suffix be safe-use.
    
    template  <class node_type_,  class node_u_type_,
    class array_type_, class array_u_type_,
    class data_type_ = int,
    class length_func_ = Length<node_type_> >
    class Tail
    {
    public:
        class TailBlock
        {
            public :
            array_type_ next_free;
            data_type_ data;
            node_type_ * suffix;
        };
        
    private:
        static const int TAIL_START_BLOCKNO = 1;
        static const int TAIL_SIGNATURE = 0xffddabaa;
        static const int INCREASEMENT_SIZE = 16;
    private:
        int num_tails;
        TailBlock * tails;
        int first_free;
    public:
        explicit Tail()
        {
            num_tails = 1;
            tails = (TailBlock *) malloc(sizeof(TailBlock));
            tails[0].data = (data_type_) 0 ;
            tails[0].suffix = NULL;
            tails[0].next_free = TAIL_SIGNATURE;
            first_free = 0;
        }
        
        ~Tail()
        {
            clear();
        }
        
        /**
         * @brief Get suffix
         * @param t      : the tail data
         * @param index  : the index of the suffix
         * @return an allocated string of the indexed suffix.
         *         Get suffix from tail with given index. The returned string is
         *         allocated. The caller should free it with free().
         */
        inline const node_type_ * getSuffix(int index) const
        {
            index -= TAIL_START_BLOCKNO;
            return (index < num_tails) ? tails[index].suffix : NULL;
        }
        
        /**
         * @brief Set suffix of existing entry
         *
         * @param t       : the tail data
         * @param index   : the index of the suffix
         * @param suffix  : the suffix to be set, notice:the suffix[len - 1] must be zero.
         * @param len     : the len of suffix.
         */
        bool setSuffix(size_t index, const node_type_ * suffix)
        {
            index -= TAIL_START_BLOCKNO;
            if (index < num_tails) {
                //  suffix and tails[index].suffix may overlap; so, dup it before
                //  it's overwritten
                int size = (length_func_()(suffix) + 1) * sizeof(node_type_);
                node_type_ * tmp = (node_type_ *) malloc(size);
                memcpy(tmp, suffix, size);
                if(tails[index].suffix != NULL) {
                    free(tails[index].suffix);
                }
                
                tails[index].suffix = tmp;
                return true;
            }
            return false;
        }
        
        /**
         * @brief Add a new suffix
         * @param suffix   : the new suffix notice:the suffix[len - 1] must be zero.
         * @param len      : the len of suffix.
         * @return the index of the newly added suffix.
         */
        array_type_ addSuffix(const node_type_ * suffix)
        {
            array_type_ new_block = allocBlock();
            setSuffix(new_block, suffix);
            return new_block;
        }
        
        
        /**
         * @brief Get data associated to suffix entry
         * @param t      : the tail data
         * @param index  : the index of the suffix
         * @return the data associated to the suffix entry
         *         Get data associated to suffix entry index in tail data.
         */
        inline data_type_ getData(int index) const {
            index -= TAIL_START_BLOCKNO;
            return (index < num_tails) ? tails[index].data : (data_type_) 0;
        }
        
        /**
         * @brief Set data associated to suffix entry
         * @param t     : the tail data
         * @param index : the index of the suffix
         * @param data  : the data to set
         * @param bytes : how many bytes the data has.
         * @return bool indicating success
         *         Set data associated to suffix entry index in tail data.
         */
        inline bool setData(int index, data_type_ data)
        {
            index -= TAIL_START_BLOCKNO;
            if (index < num_tails) {
                tails[index].data = data;
                return true;
            }
            return false;
        }
        
        /**
         * @brief Delete suffix entry
         * @param t        : the tail data
         * @param index    : the index of the suffix to delete
         *            Delete suffix entry from the tail data.
         */
        void remove(array_type_ index)
        {
            freeBlock(index);
        }
        
        /**
         * @brief Walk in tail with a string
         * @param t    : the tail data
         * @param s    : the tail data index
         * @param suffix_idx   : pointer to current character index in suffix
         * @param str          : the string to use in walking
         * @param len  : total characters in str to walk
         * @return total number of characters successfully walked
         *         Walk in the tail data t at entry s, from given character position
         *         *suffix_idx, using len characters of given string str. On
         *         return,*suffix_idx is updated to the position after the last
         *         successful walk, and the function returns the total number of
         *         character successfully walked.
         */
        int walkStr(array_type_ s, int * suffix_idx, node_type_ * str, int len)
        {
            const node_type_ * suffix = getSuffix(s);
            if (suffix == NULL)
                return 0;
            
            int i = 0;
            int j = *suffix_idx;
            while (i < len) {
                // stop and stay at null-terminator
                if (str[i] != suffix[j]) break;
                ++i;
                if (0 == suffix[j]) break;
                ++j;
            }
            *suffix_idx = j;
            return i;
        }
        
        /**
         * @brief Walk in tail with a character
         * @param t    : the tail data
         * @param s    : the tail data index
         * @param suffix_idx   : pointer to current character index in suffix
         * @param c    : the character to use in walking
         * @return true indicating success Walk in the tail data t at entry s, from
         *         given character position suffix_idx, using given character c. If
         *         the walk is successful, it returns true, and *suffix_idx is
         *         updated to the next character. Otherwise, it returns false, and
         *         *suffix_idx is left unchanged.
         */
        inline bool walkChar(array_type_ s, int * suffix_idx, node_type_ c) const
        {
            node_type_ suffix_char;
            const node_type_ * suffix = getSuffix(s);
            if (suffix == NULL) return false;
            
            suffix_char = suffix[*suffix_idx];
            if (suffix_char == c) {
                if ((node_type_)0 != suffix_char)
                    *suffix_idx = *suffix_idx + 1;
                return true;
            }
            return false;
        }
        
        bool write(ofstream & outf){
            outf.write(reinterpret_cast<char *>(&first_free), sizeof(int));
            outf.write(reinterpret_cast<char *>(&num_tails), sizeof(int));
            outf.write(reinterpret_cast<char *>(tails), sizeof(TailBlock)*num_tails);
            int len;
            for(int i = 1; i < num_tails; i++) {
                if(tails[i].suffix != NULL) {
                    len = sizeof(node_type_)*(length_func_()(tails[i].suffix)+1);
                    outf.write(reinterpret_cast<char *>(&len), sizeof(int));
                    outf.write(reinterpret_cast<char *>(tails[i].suffix), len);
                }
            }
            return true;
        }
        bool read(ifstream & inf) {
            clear();
            if (!inf.read(reinterpret_cast<char *>(&first_free), sizeof(int))){
                return false;
            }
            if (!inf.read(reinterpret_cast<char *>(&num_tails), sizeof(int))){
                return false;
            }
            tails = reinterpret_cast<TailBlock *>(malloc(sizeof(TailBlock) * num_tails));
            if (!inf.read(reinterpret_cast<char *>(tails), sizeof(TailBlock)*num_tails)){
                return false;
            }
            int len;
            for(int i = 1; i < num_tails; i++) {
                if(tails[i].suffix != NULL) {
                    inf.read(reinterpret_cast<char *>(&len),sizeof(int));
                    tails[i].suffix = reinterpret_cast<node_type_ *>(malloc(len));
                    inf.read(reinterpret_cast<char *>(tails[i].suffix), len);
                }
            }
            return true;
        }
    private:
        
        void clear()
        {
            if(tails != NULL) {
                for(int i = 1; i < num_tails; i++) {
                    if(tails[i].suffix != NULL) {
                        free( tails[i].suffix);
                    }
                }
                
                free(tails);
            }
            
            tails = NULL;
            num_tails = 0;
        }
        
        int allocBlock()
        {
            int block;
            if (0 != first_free) {
                block = first_free;
                first_free = tails[block].next_free;
            } else {
                tails = (TailBlock *)realloc(tails, (INCREASEMENT_SIZE+num_tails)*sizeof(TailBlock));
                memset(tails+num_tails, 0, sizeof(TailBlock) * INCREASEMENT_SIZE);
                block = num_tails;
                num_tails += INCREASEMENT_SIZE;
                first_free = block + 1;
                for(int i = first_free; i < num_tails - 1; i++) {
                    tails[i].next_free = i + 1;
                }
                tails[num_tails - 1].next_free = 0;
            }
            return block + TAIL_START_BLOCKNO;
        }
        
        void freeBlock(int block)
        {
            block -= TAIL_START_BLOCKNO;
            if (block >= num_tails) return;
            if(tails[block].suffix != NULL) {
                free( tails[block].suffix);
            }
            
            tails[block].data = (data_type_)0;
            tails[block].suffix = NULL;
            
            // insert block to free list, as the
            // first one.
            tails[block].next_free = first_free;
            first_free = block;
        }
    };
}


namespace mingspy
{
    
    template  <class node_type_,  class node_u_type_,
    class array_type_, class array_u_type_,
    class data_type_ = int,
    class length_func_ = Length<node_type_> >
    class DATrie
    {
    private:
        typedef DoubleArray<node_type_, node_u_type_, array_type_, array_u_type_, length_func_> _DA_;
        typedef Tail<node_type_, node_u_type_, array_type_, array_u_type_, data_type_, length_func_> _TAIL_;
        _DA_ _da;
        _TAIL_  _tail;
        
    public:
        explicit DATrie() { }
        _TAIL_ & getTail() { return _tail; }
        
        /**
         * @brief Store a value for an entry to trie
         * @param trie        : the trie
         * @param key         : the key for the entry to retrieve
         * @param data        : the data associated to the entry
         * @return boolean value indicating the success of the process Store a data
         *         for the given key in trie. If key does not exist in trie, it will
         *         be appended. If it does, its current data will be overwritten.
         */
        inline bool add(const node_type_ * key, data_type_ val) {
            return storeConditionally(key, val, false);
        }
        /**
         * @brief Delete an entry from trie
         * @param trie   : the trie
         * @param key    : the key for the entry to delete
         * @return boolean value indicating whether the key exists and is removed
         *         Delete an entry for the given key from trie.
         */
        bool remove(const node_type_ * key)
        {
            array_type_ t;
            /* walk through branches */
            array_type_ s = _da.getRoot();
            const node_type_ * p = key;
            for (; !isSeparate(s); p++) {
                if (!_da.walk(&s, *p)) { return false; }
                if ((node_type_)0 == *p) break;
            }
            
            /* walk through tail */
            t = getTailIndex(s);
            int suffix_idx = 0;
            for (; ; p++) {
                if (!_tail.walkChar(t, &suffix_idx, *p)) { return false; }
                if ((node_type_)0 == *p) break;
            }
            
            _tail.remove(t);
            _da.setBase(s, TRIE_INDEX_ERROR);
            _da.prune(s);
            
            return true;
        }
        
        /**
         * @brief Retrieve an entry from trie
         * @param key       : the key for the entry to retrieve
         * @param o_data    : the storage for storing the entry data on return
         * @return 0 value indicating the existence of the entry.
         *         Retrieve an entry for the given key from trie. On return, if key
         *         is found and o_data is not NULL, *o_data is set to the data
         *         associated to key.
         */
        bool find(const node_type_ * key, data_type_ * data = NULL) const
        {
            // walk through branches
            array_type_ s = _da.getRoot();
            const node_type_ * p = key;
            for (; !isSeparate(s); p++) {
                if (!_da.walk(&s, *p)) { return false; }
                if ((node_type_)0 == *p) break;
            }
            
            // walk through tail
            s = getTailIndex(s);
            int suffix_idx = 0;
            for (; ; p++) {
                if (!_tail.walkChar(s, &suffix_idx, *p)) { return false; }
                if ((node_type_)0 == *p) break;
            }
            
            // found, set the val and return
            if (data) *data =  _tail.getData(s);
            return true;
        }
        
        bool hasPrefix(const node_type_ * prefix) const
        {
            // walk through branches
            array_type_ s = _da.getRoot();
            const node_type_ * p = prefix;
            for (; !isSeparate(s); p++) {
                if ((node_type_)0 == *p) return true;
                if (!_da.walk(&s, *p)) { return false; }
            }
            
            // walk through tail
            s = getTailIndex(s);
            int suffix_idx = 0;
            for (; ; p++) {
                if ((node_type_)0 == *p) return true;
                if (!_tail.walkChar(s, &suffix_idx, *p)) { return false; }
            }
            
            return false;
        }
        
        bool write(ofstream & outf) {
            if(!_da.write(outf)){
                return false;
            }
            
            if(!_tail.write(outf)){
                return false;
            }
            cout<<"save datrie finished"<<endl;
            return true;
        }
        
        bool read(ifstream & inf) {
            if(!_da.read(inf)){
                return false;
            }
            
            if(!_tail.read(inf)){
                return false;
            }
            return true;
        }
        
    private:
        inline bool isSeparate(int s) const { return _da.getBase(s) < 0; }
        // save -idx in da.base when s' suffix in tail.
        inline array_type_ getTailIndex(int s) const { return - _da.getBase(s); }
        
        /**
         * @brief Store a value for an entry to trie only if the key is not present
         * @param trie      : the trie
         * @param key       : the key for the entry to retrieve
         * @param data      : the data associated to the entry
         * @return boolean value indicating the success of the process Store a data
         *         for the given key in trie. If key does not exist in trie, it will
         *         be appended. If it does, the function will return failure and the
         *         existing value will not be touched. This can be useful for
         *         multi-thread applications, as race condition can be avoided.
         *         Available since: 0.2.4
         */
        inline bool storeIfAbsent(const node_type_ * key, data_type_ data) {
            return storeConditionally(key, data,false);
        }
        
        bool storeConditionally(const node_type_ * key, data_type_ data, bool is_overwrite)
        {
            // walk through branches 
            array_type_ s = _da.getRoot();
            const node_type_ * p = key;
            for (; !isSeparate(s); p++) {
                if (!_da.walk(&s, *p)) { return branchInBranch(s, p, data); }
                if ((node_type_)0 == *p) break;
            }
            
            // walk through tail
            const node_type_ * sep = p;
            int t = getTailIndex(s);
            int suffix_idx = 0;
            for (; ; p++) {
                if (!_tail.walkChar(t, &suffix_idx, *p)) { return branchInTail(s, sep, data); }
                if ((node_type_)0 == *p) break;
            }
            
            // duplicated key, overwrite val if flagged
            if (!is_overwrite) { return false; }
            _tail.setData(t, data);
            return true;
        }
        
        inline void setTailIndex(size_t s, int v) { _da.setBase(s, -v); }
        
        /**
         * add suffix as children of sep_node
         * only happened when suffix[0] not a child of sep_node yet.
         */
        bool branchInBranch(array_type_ sep_node, const node_type_ * suffix, data_type_ data) {
            array_type_ new_da, new_tail;
            new_da = _da.insertBranch(sep_node, *suffix);
            if (TRIE_INDEX_ERROR == new_da) return false;
            
            if ((node_type_)0 != *suffix)  ++suffix;
            new_tail = _tail.addSuffix(suffix);
            _tail.setData(new_tail, data);
            setTailIndex(new_da, new_tail);
            return true;
        }
        
        bool branchInTail(array_type_ sep_node,const node_type_ * suffix, data_type_ data)
        {
            // adjust separate point in old path
            array_type_ old_tail = getTailIndex(sep_node);
            const node_type_ * old_suffix = _tail.getSuffix(old_tail);
            if (old_suffix == NULL)  return false;
            array_type_ s = sep_node;
            const node_type_ * p = old_suffix;
            // all equal node are saved to double-array as separate node.
            for (; *p == *suffix; p++, suffix ++) {
                array_type_ t = _da.insertBranch(s, *suffix);
                if (TRIE_INDEX_ERROR == t) {
                    _da.pruneUpto(sep_node, s);
                    setTailIndex(sep_node, old_tail);
                    return false;
                }
                s = t;
            }
            
            int old_da = _da.insertBranch(s, *p);
            if (TRIE_INDEX_ERROR == old_da) {
                _da.pruneUpto(sep_node, s);
                setTailIndex(sep_node, old_tail);
                return false;
            }
            
            // when run here, all equals prefix are added.
            // save old left suffix.
            if ((node_type_)0 != *p)  ++p;
            _tail.setSuffix(old_tail, p);
            setTailIndex(old_da, old_tail);
            
            // insert the new branch at the new separate point
            return branchInBranch(s, suffix, data);
        }
        
    };
    
    typedef DATrie<char, unsigned char, int, unsigned int, int> CharTrie;
}

