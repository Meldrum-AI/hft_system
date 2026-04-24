#pragma once
#include <cstring>
#include <atomic>

namespace tdsys {
    template<size_t KeyLen, typename Val, size_t Cap>
    class FixedHashTable {
    private:
        struct Entry {
            char key[KeyLen + 1]{};
            Val val{};
            enum { EMPTY, USED, DEL } st = EMPTY;
        };
        Entry tab[Cap]{};
        std::atomic<size_t> cnt{ 0 };

        size_t hash(const char* k) const {
            size_t h = 0;
            for (size_t i = 0; i < KeyLen && k[i]; i++) h = h * 31 + k[i];
            return h;
        }

    public:
        bool insert(const char* k, const Val& v) {
            if (cnt >= Cap) return false;
            size_t i = hash(k) % Cap;
            for (size_t p = 0; p < Cap; p++) {
                if (tab[i].st != USED) {
                    strncpy(tab[i].key, k, KeyLen);
                    tab[i].val = v;
                    tab[i].st = USED;
                    cnt++;
                    return true;
                }
                i = (i + p * p) % Cap;
            }
            return false;
        }

        Val* find(const char* k) {
            size_t i = hash(k) % Cap;
            for (size_t p = 0; p < Cap; p++) {
                if (tab[i].st == USED && !strcmp(tab[i].key, k)) return &tab[i].val;
                i = (i + p * p) % Cap;
            }
            return nullptr;
        }
    };
}
