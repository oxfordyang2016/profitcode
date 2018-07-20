#ifndef SRC_ZSHARED_STRING_MAP_H_
#define SRC_ZSHARED_STRING_MAP_H_

#include <stdint.h>
#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <string>

#include "zshared/util.h"

namespace zshared {

template<class ValueType>
class StringMap {
 public:
  StringMap() {
    root_ = new StringMapNode();
  }
  ~StringMap() {
    delete root_;
  }

  void Clear() {
    delete root_;
    root_ = new StringMapNode();
  }

  ValueType* Get(const char* key) const {
    return root_->Get(key, strlen(key));
  }
  ValueType* Get(const char* key, size_t size) const {
    return root_->Get(key, size);
  }
  ValueType* Get(const std::string & key) const {
    return root_->Get(key.c_str(), key.size());
  }

  ValueType* MatchPrefix(const char* key) const {
    return root_->MatchPrefix(key);
  }

  ValueType & Add(const char* key, const ValueType & value) {
    return root_->Add(key, strlen(key), value);
  }
  ValueType & Add(const char* key, size_t size, const ValueType & value) {
    return root_->Add(key, size, value);
  }
  ValueType & Add(const std::string & key, const ValueType & value) {
    return root_->Add(key.c_str(), key.size(), value);
  }

  bool Remove(const std::string & key) {
    return root_->Remove(key.c_str(), key.size());
  }

  ValueType & operator[](const char* key) {
    size_t len = strlen(key);
    ValueType* v = root_->Get(key, len);
    if (!v) {
      return root_->Add(key, len, ValueType());
    }
    return *v;
  }
  ValueType & operator[](const std::string & key) {
    ValueType* v = root_->Get(key.c_str(), key.size());
    if (!v) {
      return root_->Add(key.c_str(), key.size(), ValueType());
    }
    return *v;
  }

 private:
  class StringMapNode {
   public:
    StringMapNode()
      : has_value_(false),
        min_(0),
        count_(0),
        live_nodes_(0) {
    }

    ~StringMapNode() {
      if (count_ == 1) {
        delete next_.node;
        next_.node = 0;
      } else if (count_ > 1) {
        for (int i = 0; i < count_; ++i)
          delete next_.table[i];
        free(next_.table);
      }
    }

    ValueType & Add(const char* key, size_t size, const ValueType & value) {
      StringMapNode* current = this;
      while (size > 0) {
        unsigned char c = *key;
        current->ExpandTable(c);

        //  If next node does not exist, create one.
        if (current->count_ == 1) {
          if (!current->next_.node) {
            current->next_.node = new(std::nothrow) StringMapNode();
            ++current->live_nodes_;
          }
          current = current->next_.node;
        } else {
          if (!current->next_.table[c - current->min_]) {
            current->next_.table[c - current->min_] = new(std::nothrow) StringMapNode();
            ++current->live_nodes_;
          }
          current = current->next_.table[c - current->min_];
        }
        ++key;
        --size;
      }

      current->value_ = value;
      current->has_value_ = true;
      return current->value_;
    }

    bool Remove(const char* key, size_t size) {
      if (!size) {
        if (!has_value_)
          return false;
        has_value_ = false;
        return true;
      }

      unsigned char c = *key;
      if (!count_ || c < min_ || c >= min_ + count_)
        return false;

      StringMapNode *next_node = count_ == 1 ? next_.node : next_.table[c - min_];
      if (!next_node)
        return false;

      bool ret = next_node->Remove(key + 1, size - 1);

      if (next_node->IsRedundant()) {
        delete next_node;
        if (count_ == 1) {
          next_.node = 0;
          count_ = 0;
        } else {
          next_.table[c - min_] = 0;
        }
        --live_nodes_;
      }

      return ret;
    }

    ValueType* MatchPrefix(const char* key) const {
      const StringMapNode* current = this;
      unsigned char c = *key;
      while (c) {
        //  If there's no corresponding slot for the first character
        //  of the key, the message does not match.
        if (c < current->min_ || c >= current->min_ + current->count_) {
          return 0;
        }

        //  Move to the next character.
        if (current->count_ == 1) {
          current = current->next_.node;
        } else {
          current = current->next_.table[c - current->min_];
          if (!current) {
            return 0;
          }
        }

        if (current->has_value_) {
          return const_cast<ValueType*>(&current->value_);
        }

        ++key;
        c = *key;
      }

      return 0;
    }

    ValueType* Get(const char* key, size_t size) const {
      //  This function is on critical path. It deliberately doesn't use
      //  recursion to get a bit better performance.
      const StringMapNode* current = this;
      while (size > 0) {
        //  If there's no corresponding slot for the first character
        //  of the key, the message does not match.
        unsigned char c = *key;
        if (c < current->min_ || c >= current->min_ + current->count_)
          return 0;

        //  Move to the next character.
        if (current->count_ == 1) {
          current = current->next_.node;
        } else {
          current = current->next_.table[c - current->min_];
          if (!current)
            return 0;
        }
        ++key;
        --size;
      }

      return current->has_value_ ? const_cast<ValueType*>(&current->value_) : 0;
    }

   private:
    void ExpandTable(unsigned char c) {
      if (c < min_ || c >= min_ + count_) {
        //  The character is out of range of currently handled
        //  charcters. We have to extend the table.
        if (!count_) {
          min_ = c;
          count_ = 1;
          next_.node = 0;
        } else if (count_ == 1) {
          unsigned char oldc = min_;
          StringMapNode* oldp = next_.node;
          count_ = (min_ < c ? c - min_ : min_ - c) + 1;
          next_.table = static_cast<StringMapNode**>(malloc(sizeof(next_.node) * count_));
          assert(next_.table);
          for (unsigned short i = 0; i != count_; ++i)
            next_.table[i] = 0;
          min_ = std::min(min_, c);
          next_.table[oldc - min_] = oldp;
        } else if (min_ < c) {
          //  The new character is above the current character range.
          unsigned short old_count = count_;
          count_ = c - min_ + 1;
          next_.table = static_cast<StringMapNode**>(realloc(next_.table, sizeof(next_.node) * count_));
          assert(next_.table);
          for (unsigned short i = old_count; i != count_; i++)
            next_.table[i] = 0;
        } else {
          //  The new character is below the current character range.
          unsigned short old_count = count_;
          count_ = (min_ + old_count) - c;
          next_.table = static_cast<StringMapNode**>(realloc(next_.table, sizeof(next_.node) * count_));
          assert(next_.table);
          memmove(next_.table + min_ - c, next_.table, old_count * sizeof(next_.node));
          for (unsigned short i = 0; i != min_ - c; i++)
            next_.table[i] = 0;
          min_ = c;
        }
      }
    }

    bool IsRedundant() const {
      return !has_value_ && live_nodes_ == 0;
    }

    bool     has_value_;
    uint8_t  min_;
    uint16_t count_;
    uint16_t live_nodes_;
    union {
      class StringMapNode *node;
      class StringMapNode **table;
    } next_;
    ValueType value_;

    StringMapNode(const StringMapNode &);
    const StringMapNode &operator =(const StringMapNode &);
  };

 private:
  StringMapNode* root_;
  DISALLOW_COPY_AND_ASSIGN(StringMap);
};
}

#endif  // SRC_ZSHARED_STRING_MAP_H_
