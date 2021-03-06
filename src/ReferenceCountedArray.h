/* ReferenceCountedArray.h
 *
 * Copyright (C) Bryan Drewery
 *
 * This program is private and may not be distributed, modified
 * or used without express permission of Bryan Drewery.
 *
 * THIS PROGRAM IS DISTRIBUTED WITHOUT ANY WARRANTY.
 * IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY
 * FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
 * ARISING OUT OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY
 * DERIVATIVES THEREOF, EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE
 * IS PROVIDED ON AN "AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE
 * NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
 * MODIFICATIONS.
 */
#ifndef _BD_REFERENCE_COUNTED_ARRAY_H
#define _BD_REFERENCE_COUNTED_ARRAY_H 1

#include "bdlib.h"
#include "hash.h"
#include <iterator>
#include <stdint.h>
#include <sys/types.h>
#ifdef DEBUG
#include <stdio.h>
#endif

BDLIB_NS_BEGIN
template <class T>
/**
 * @class ArrayRef
 * @brief Helps the String and Array classes with reference counting
 * @todo look into something like a string object made of string pointers, and << displays all of the pointed to objects. (this would save copying to construct immuatable concatenated strings, but would only work with the << stream
 */
class ArrayRef {
  public:
    ~ArrayRef() { FreeBuf(buf); };
    mutable size_t size; //Capacity of buffer
    mutable T* buf;
    mutable int n; //References

    ArrayRef() : size(0), buf(NULL), n(1) {};
    /**
     * @brief Ensure that the buffer capacity() is >= newSize; else grow/copy into larger buffer.
     * @param newSize A size that we need to Allocate the buffer to.
     * @param offset The offset of the old buffer so we know where to start
     * @param sublen The length of the subarray in use
     * @pre newSize is > 0 (assumed as size_t is unsigned)
     * @post The buffer is at least nsize bytes long.
     * @post If the buffer had to grow, the old data was deep copied into the new buffer and the old deleted.
     */
    void Reserve(const size_t newSize, size_t& offset, size_t sublen) const
    {
      /* Don't new if we already have enough room! */
      if (size < newSize) {
        size = std::max(size_t(size * 1.5), newSize);

        T *newbuf = AllocBuf(size);

        if (newbuf != buf) {
          /* Copy old buffer into new - only copy the subarray */
          std::copy(buf + offset, buf + offset + sublen, newbuf);
          FreeBuf(buf);
          buf = newbuf;
          offset = 0;
        }
      } else if ((size - offset) < newSize) {
        // There's enough room in the current buffer, but we're offsetted/shifted to a point where there's no room left
        // Shift everything to the beginning and reset the offset
        /* Only copy the subarray */
        memmove(buf, buf + offset, sublen);
        offset = 0;
      }
    }

    /**
     * @brief Allocates a buffer and returns it's address.
     * @param bytes The number of bytes to allocate.
     * @post A new block of memory is allocated.
     * @todo Implement mempool here.
     */
    inline T* AllocBuf(const size_t bytes) const {
      return new T[bytes];
    }

    /**
     * @brief Free's up the allocated buffer.
     * @param p The buffer to be free'd
     * @post The buffer is deleted.
     * @todo Implement mempool here.
     */
    inline void FreeBuf(T* p) const {
      delete[] p;
    }

    /**
     * @brief Is this ReferenceCountedArray shared?
     */
    inline bool isShared() const { return n > 1; };
  private:
    // No copying allowed
    ArrayRef(const ArrayRef&); ///<Block implicit copy constructor
    ArrayRef& operator=(const ArrayRef&); ///<Block implicit copy constructor
};

template <class T>
/**
 * @class Slice
 * @brief Safe subarray reading and writing.
 * @todo This should not provide copy constructors for Cref, they shouldn't be needed because of const char String::operator[]
 * This class should be optimized away and fully inlined such that:
 * String s("look over there");
 * s(0, 4) = "blah"';
 * Should be rewritten as:
 * s.replace(0, "blah", 4);
 */
class Slice {
  private:
    T& rca;
    int start;
    int len;

    Slice();

  public:
    Slice(T& _rca, int _start, int _len) : rca(_rca), start(_start), len(_len) {};
    Slice(const Slice& slice) : rca(slice.rca), start(slice.start), len(slice.len) {};

    /**
     * @brief return a new (const) slice
     */

    inline operator T() const {
      T newArray(rca);
      newArray.slice(start, len);
      return newArray;
    };

    inline Slice& operator= (const Slice& slice) {
      (*this) = T(slice);
      return (*this);
    }

    /**
     * @todo This needs to account for negative start/len
     */
    inline Slice& operator= (const T& array) {
      rca.replace(start, array, len);
      return (*this);
    }
};


template <class T>
/**
 * @class ReferenceCountedArray
 * @brief
 */
class ReferenceCountedArray {
  public:
    typedef T value_type;

    typedef size_t             size_type;
    typedef ptrdiff_t          difference_type;
    typedef value_type*        pointer;
    typedef const value_type*  const_pointer;
    typedef value_type&        reference;
    typedef const value_type&  const_reference;

    static const size_t npos = static_cast<size_t>(-1);

  private:
    /**
     * @brief Detach from the shared reference.
     * This is only called when losing the old buffer or when modifying the buffer (and copy-on-write is used)
     * @note This does not free the old reference, as it is still in use
     */
    void doDetach() const {
      decRef();
      Ref = new ArrayRef<value_type>();
      sublen = 0;
      offset = 0;
      my_hash = 0;
    }
    /**
     * @brief Increment our reference counter.
     */
    inline int incRef() const { return ++Ref->n; };

    /**
     * @brief Decrement our reference counter.
     */
    inline int decRef() const { return --Ref->n; };

  protected:
    /**
     * @brief Set the lengths to the specified length
     * @param newLen the new length to set to
     */
    inline void setLength(size_t newLen) const { sublen = newLen; my_hash = 0; };

    /**
     * @sa setLength()
     */
    inline void addLength(size_t diff) const { sublen += diff; my_hash = 0; };

    /**
     * @sa setLength()
     */
    inline void subLength(size_t diff) const { sublen -= diff; my_hash = 0; };

    /**
     * @brief Mutable Ref->buf+offset reference for use internally
     */
    //pointer mdata() const { return Buf() + offset; };

    /**
     * @brief Mutable Ref->buf reference for use internally
     */
    inline pointer Buf(size_t pos = 0) const { return &Ref->buf[offset + pos]; };

    /**
     * @brief Ref->buf reference for use internally
     */
    inline const_pointer constBuf(size_t pos = 0) const { return Buf(pos); };
  private:
    /**
     * @brief The array reference for reference counting
     * This is mutable so that Ref->n can be modified, which really is mutable
     */
    mutable ArrayRef<value_type> *Ref;
  protected:
    /**
     * Return the real buffer's start point, without accounting for offset. This is used for cleaning the buffer when needed.
     */
    inline const_pointer real_begin() const { return Ref->buf; };

    /**
     * This is for subarrays: so we know where the subarray starts.
     */
    mutable size_t offset;
    /**
     * This is for subarrays: so we know where the subarray ends.
     */
    mutable size_t sublen;

    /**
     * Cache of current hash() result. 0 if stale
     */
    mutable size_t my_hash;

  private:
    /**
     * @brief Detach from the reference
     * This is called when the old buffer is no longer needed for this Array.
     * ie, operator=() was called.
     */
    void Detach() {
      if (isShared()) {
        doDetach();
      } else {
        setLength(0);
        offset = 0;
        my_hash = 0;
      }
    }

    /**
     * @brief Free up our reference if we have the last one.
     * @post The reference counter is decremented.
     * @post If this was the last Reference, it is free'd
     * This is only called in ~Array() and operator=(Array&).
     * It checks whether of not this Array was the last reference to the buffer, and if it was, it removes it.
     */
    inline void CheckDeallocRef() {
      if (decRef() < 1)
        delete Ref;
    }

    //    void COW(size_t) const;
    /**
     * @brief Ensure that our internal buffer is unshared.
     * @param n Create/Grow the buffer to this size.
     * @pre n is > 0; this is assumed due to size_t being unsigned though.
     * @post The internal buffer/data is unshared
     * @post The buffer is at least size n.
     * @post The buffer is deep copied to a new buffer.
     *
     * Ensure that our internal buffer is unshared.
     * If needed, performs a deep copy into a new buffer (COW).
     * Also take a hint size n of the new ReferenceCountedArray's size as to avoid needless copying/allocing.
     */
    void COW(size_t n) const {
      const_pointer oldBuf = constBuf();
      size_t oldLength = length();

      doDetach(); //Detach from the shared reference
      Reserve( std::max(oldLength, n) ); //Will set capacity()/size
      std::copy(oldBuf, oldBuf + oldLength, Buf());
      setLength(oldLength);
    }

  protected:
    /**
     * @brief Force COW if needed
     * @post The array is no longer shared, if it was.
     */
    inline void getOwnCopy() const { AboutToModify(length()); };

    /**
     * @brief Warn the reference counting that it may need to COW
     * @post The buffer is detached/COW, and possibly larger
     * @todo If the buffer is shared and needs to shrink, the sublen should just be decreased.
     */
    inline void AboutToModify(size_t n) const {
      my_hash = 0;
      if (isShared())
        COW(n); // Clears the offset
      else {
        Reserve(n);
        /* Shift the offset away */
      }
    }
  public:
    ReferenceCountedArray() : Ref(new ArrayRef<value_type>()), offset(0), sublen(0), my_hash(0) {};
    ReferenceCountedArray(const ReferenceCountedArray& rca) : Ref(rca.Ref), offset(rca.offset), sublen(rca.sublen), my_hash(rca.my_hash) { incRef(); };
    /**
     * @brief Create an empty container with at least the specified bytes in size.
     * @param newSize Reserve at least this many bytes for this ReferenceCountedArray.
     * @post This ReferenceCountedArray's memory will also never be shrunk.
     * @post A buffer has been created.
     *
     * The idea behind this is that if a specific size was asked for, the buffer is like
     * a char buf[N];
     */
    explicit ReferenceCountedArray(const size_t newSize) : Ref(new ArrayRef<value_type>()), offset(0), sublen(0), my_hash(0) {
      if (newSize <= 0) return;
      Reserve(newSize);
    };
    /**
     * @brief Create a container filled with n copies of the given value.
     * @param newSize Reserve at least this many bytes for this ReferenceCountedArray.
     * @param value The value to populate the array with
     * @post This ReferenceCountedArray's memory will also never be shrunk.
     * @post A buffer has been created.
     *
     */
    ReferenceCountedArray(const size_t newSize, const value_type value) : Ref(new ArrayRef<value_type>()), offset(0), sublen(0), my_hash(0) {
      if (newSize <= 0) return;
      Reserve(newSize);

      for (size_t i = 0; i < newSize; ++i)
        *(Buf(i)) = value;
      this->setLength(newSize);
    }

    /**
     * @brief Array Destructor
     * @post If the Array's Reference is not shared, it is free'd.
     * @post If the Array's Reference IS shared, it is decremented and detached.
     */
    virtual ~ReferenceCountedArray() { CheckDeallocRef(); };

    /**
     * @brief Sets our Reference to the given ReferenceCountedArray reference.
     * @param rca The ReferenceCountedArray object to reference.
     * @post The old buffer (if we had one) is free'd.
     * @post Our Reference now points to the given ReferenceCountedArray.
     * @post Our old rca object has been deleted (disconnected).
     * @return The new rca object.
     * This handles self-assignment just fine, checking for it explicitly would be ineffecient for most cases.
     */
    const ReferenceCountedArray& operator=(const ReferenceCountedArray &rca) {
      rca.incRef();
      offset = rca.offset;
      sublen = rca.sublen;
      my_hash = rca.my_hash;
      CheckDeallocRef();
      Ref = rca.Ref;
      return *this;
    }

    /**
     * @brief Sets our buffer to the given item.
     * @param item The item to set our buffer to.
     * @post The old buffer (if we had one) is free'd.
     * @post A sufficiently sized new buffer is made with the item within.
     * @return The new array object.
     */
    const ReferenceCountedArray& operator=(const_reference item) {
      Detach();
      append(item);
      return *this;
    }

    /**
     * @brief How many references does this object have?
     */
    inline size_t rcount() const { return Ref->n; };
    /**
     * @return True if this object is shared; false if not.
     */
    inline bool isShared() const { return Ref->isShared(); };


    /**
     * @sa ArrayRef::Reserve()
     * @post The ReferenceCountedArray will also never shrink after this.
     */
    virtual void Reserve(const size_t newSize) const { Ref->Reserve(newSize, offset, sublen); };

    /**
     * @brief Clear contents of ReferenceCountedArray and set length to 0
     */
    virtual inline void clear() { Detach(); };

    /**
     * @brief Returns capacity of the ReferenceCountedArray object.
     * @return Capacity of the ReferenceCountedArray object.
     */
    inline size_t capacity() const { return Ref->size; };

    /**
     * @brief Resize the array to the given length.
     * @param len The length to resize to.
     * @param value The optional parameter to fill the space with if the array is expanded
     */
    void resize(size_t len, value_type value = value_type()) {
      if (len < this->length()) {
        this->subLength(this->length() - len);
      } else {
        this->AboutToModify(len);
        for (size_t i = 0; i < (len - this->length()); ++i)
          *(Buf(this->length() + i)) = value;
        this->addLength(len - this->length());
      }
    }

    /* Accessors */
    /**
     * @brief Returns length of the ReferenceCountedArray.
     * @return Length of the ReferenceCountedArray.
     */
    inline size_t length() const { return sublen; };
    /**
     * @sa length()
     */
    inline size_t size() const { return length(); };

    /**
     * @brief Check whether the ReferenceCountedArray is 'empty'
     * @return True if empty, false if non-empty
     */
    inline bool isEmpty() const { return length() == 0; };
    /**
     * @sa isEmpty()
     * This is for: if (!string)
     * Having if(string) conflicts with another operator
     */
    inline bool operator ! () const { return isEmpty(); };

    /**
     * @brief Data accessor
     * @return Pointer to array of characters (not necesarily null-terminated).
     */
    inline const_pointer data() const { return constBuf(); }
    inline const_pointer begin() const { return data(); };
    inline const_pointer end() const { return begin() + length(); };

    typedef Hash<value_type> HashType;

   /**
     * @brief Return a hash of every element in the array. Cache result as well.
     * @note DJB's hash function
     */
    virtual size_t hash() const {
      if (my_hash != 0) return my_hash;
      HashType hasher;
      size_t _hash = 5381;

      for(size_t i = 0; i < this->length(); ++i)
        _hash = ((_hash << 5) + _hash) + hasher(this->data()[i]);
      return (my_hash = (_hash & 0x7FFFFFFF));
    }


    /**
     * @brief Find an item in the array
     * @return The position of the item if found, or npos if not found
     **/
    size_t find(const_reference item) const {
      for (size_t i = 0; i < length(); ++i)
        if (*(Buf(i)) == item)
          return i;
      return size_t(-1);
    }

    /**
     * @brief Find an item in the array, starting from the end
     * @return The position of the item if found, or npos if not found
     **/
    size_t rfind(const_reference item) const {
      for (size_t i = length() - 1; i + 1 > 0; --i)
        if (*(Buf(i)) == item)
          return i;
      return size_t(-1);
    }

    // Safe index accessors
    /**
     * @brief Checks if the buffer has the given index or not.
     * @return Boolean true/false as to whether or not index exists.
     * @param pos Index to check.
     */
    inline bool hasIndex(size_t pos) const {
      if (pos >= (offset + length())) {
#ifdef DEBUG
        ::printf("ATTEMPT TO ACCESS INDEX %zu/%zu\n", pos, offset + length());
#endif
        return 0;
      }
      return (pos < length());
    };

    /**
     * @sa at()
     * Unlinke at() this is unchecked.
     */
    inline value_type read(size_t pos) const { return *(constBuf(pos)); };

    /**
     * @brief Write an item to the given index
     */
    inline void write(size_t pos, value_type item) {
      getOwnCopy();
      *(Buf(pos)) = item;
    };

    /**
     * @brief Safe element access operator
     * @todo This is only called on a (const) ReferenceCountedArray, but should for a ReferenceCountedArray as well.
     */
    inline value_type operator [](size_t pos) const { return read(pos); };

    /**
     * @class Cref
     * @brief Safe element reading and writing.
     * @todo This should not provide copy constructors for Cref, they shouldn't be needed because of const char String::operator[]
     * This class should be optimized away and fully inlined such that:
     * String s;
     * s[0] = 'a';
     * Should be rewritten as:
     * s.write(0, 'a');
     */
    class Cref {
      private:
        friend class ReferenceCountedArray;
        ReferenceCountedArray& rca;
        size_t k;

        /**
         * @brief Used by String::Cref operator[]
         */
        Cref(ReferenceCountedArray& _rca, size_t pos) : rca(_rca), k(pos) {};
        Cref(); //Not defined - never used

      public:
        Cref(const Cref& cref) : rca(cref.rca), k(cref.k) {};
        inline Cref& operator=(const Cref& cref) {
          (*this) = value_type(cref);
          return (*this);
        }

        public:
        /**
         * @sa ReferenceCountedArray::operator[]
         */
        inline operator value_type() const { return rca.read(k); };

        /**
         * Stroustrup shows using this as void with no return value, but that breaks chaining a[n] = b[n] = 'b';
         */
        inline Cref& operator=(value_type c) {
          rca.write(k, c);
          return (*this);
        };
    };

    /**
     * @brief Returns 'Cref' class for safe (cow) writing into String.
     * @sa Cref
     */
    inline Cref operator [](size_t pos) { return Cref(*this, pos); };

    /**
     * @brief Returns the character at the given index.
     * @return The character at the given index.
     * @param pos Index to return.
     * @pre The index must exist.
     * @sa operator[]()
     * @todo Perhaps this should throw an exception if out of range?
     */
    inline value_type at(size_t pos) const { return hasIndex(pos) ? (*this)[pos] : 0; };

    /**
     * @brief Return a new array from a subarray
     * @return a new ReferenceCountedArray
     * @param start The offset to begin the subarray from (indexed from 0)
     * @param len The length of the subarray to return
     * The returned slice is a reference to the original array until modified.
     */
    void slice(int start, int len = -1) {
      if (len == -1) len = int(length()) - start;
      // Start is after the end, set us to an empty array
      if (start >= static_cast<signed>(length())) {
        offset = length();
        setLength(0);
        return;
      }

      //Count backwards from the end
      if (start < 0)
        start = length() + start;
      // Start was before the beginning, just reset to the beginning
      if (start < 0)
        start = 0;

      offset += start;
      //If the length of the subarray exceeds the end of the array, truncate to the end of the array
      if (start + len >= static_cast<signed>(length()))
        len = length() - start;
      // If the length is negative, stop from counting backwards from the end
      else if (len < 0)
        len = length() - start + len;
      setLength(len);
    }

    /**
     * @brief Appends one item to end of buffer.
     * @param item The item to be appended.
     * @post The buffer is allocated.
     * @post The item is appended at the end of the buffer.
     * This is the same as inserting the item at the end of the buffer.
     */
    inline void append(const_reference item) { insert(length(), item); };
    /**
     * @brief Appends given rca to the end of buffer
     * @param rca The rca to be appended.
     * @param n How many characters to copy from the ReferenceCountedArray object.
     * @post The buffer is allocated.
     * This is the same as inserting the rca at the end of the buffer.
     */
    inline void append(const ReferenceCountedArray& rca, size_t n = npos) { insert(length(), rca, n); };


    /**
     * @brief Inserts a ReferenceCountedArray object into our buffer
     * @param pos The index to insert at.
     * @param rca The rca to insert.
     * @param n The length to insert.
     * @post The buffer contains n items from rca inserted at index pos.
     */
    void insert(size_t pos, const ReferenceCountedArray& rca, size_t n = npos) {
      if (n == 0) return;
      if (pos && !hasIndex(pos-1)) return;

      size_t slen = rca.length();

      /* New rca is longer than ours, and inserting at 0, just replace ours with a reference of theirs */
      if (pos == 0 && slen > length() && (n == npos || n == slen)) {
        *this = rca;
        return;
      }

      if (n == npos || n > slen)
        n = slen;
      slen -= slen - n;
      AboutToModify(length() + slen);
      memmove(Buf() + pos + slen, Buf() + pos, length() - pos);
      std::copy(rca.begin(), rca.begin() + slen, Buf() + pos);
      addLength(slen);
    }

    /**
     * @brief Insert an item at the given index.
     * @param pos The index to insert at.
     * @param item The item to be inserted.
     * @post A buffer is allocated.
     * @post If the old buffer was too small, it is enlarged.
     * @post The item is inserted at the given index.
     */
    void insert(size_t pos, const_reference item)
    {
      if (pos && !hasIndex(pos-1)) return;

      AboutToModify(length() + 1);
      memmove(Buf() + pos + 1, Buf() + pos, length() - pos);
      *(Buf(pos)) = item;
      addLength(1);
    }

    /**
     * @brief Replace the given index with the given item.
     * @param pos The index to replace.
     * @param item The item to replace with.
     * @post The given index has been replaced.
     * @post COW is done if needed.
     */
    void replace(size_t pos, const_reference item) {
      if (pos && !hasIndex(pos-1)) return;

      getOwnCopy();
      *(Buf(pos)) = item;
    }

    /**
     * @brief Replaces n elements in our buffer at index pos with the given ReferenceCountedArray object
     * @param pos The index to replace at.
     * @param rca The ReferenceCountedArray object to replace with.
     * @param n The number of characters to use for the replace.
     */
    void replace(size_t pos, const ReferenceCountedArray &rca, size_t n = npos) {
      if (n == 0) return;
      if (pos && !hasIndex(pos-1)) return;

      size_t slen = rca.length();

      /* Replace rca is longer than ours, and inserting at 0, just replace ours with a reference of theirs */
      if (pos == 0 && slen > length() && (n == npos || n == slen)) {
        *this = rca;
        return;
      }

      if (n == npos || n > slen)
        n = slen;
      slen -= slen - n;

      size_t newlen = pos + slen;

      if (newlen >= length()) {
        AboutToModify(newlen);
      } else {
        newlen = length();
        getOwnCopy();
      }
      std::copy(rca.begin(), rca.begin() + slen, Buf() + pos);
      setLength(newlen);
    }

};
BDLIB_NS_END

#endif
