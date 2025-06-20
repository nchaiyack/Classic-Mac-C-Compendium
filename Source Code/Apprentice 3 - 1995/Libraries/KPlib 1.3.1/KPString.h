// A module of KPlib v1.3.1.
// Written by Keith Pomakis during the summer of 1994.
// Released to the public domain on October 10, 1994.

#ifndef KP_STRING_DEFINED
#define KP_STRING_DEFINED

#include <iostream.h>
#include <strstream.h>
#include <string.h>
#include "KPbasic.h"

// This class uses reference counting and copy-on-write semantics to insure
// that it as efficient as possible.

// All indexes are zero-based.  For all functions that accept an index, a
// negative index specifies an index from the right of the string.  Also,
// for all functions that accept a length, a length of -1 specifies the rest
// of the string.

// "KPList.h" is included further on down to insure that it isn't defined
// before KPString, so that KPList<KPString> will be considered a complete
// type in the following definition of KPString.  This is probably a
// g++-specific problem.

template <class Element> class KPList;

class KPString {
        friend istream &operator>>(istream &stream, KPString &s);
        inline friend ostream &operator<<(ostream &stream, const KPString &s);
        friend KPString operator+(const char *s1, const KPString &s2);
        inline friend bool operator==(const char *s1, const KPString &s2);
        inline friend bool operator<(const char *s1, const KPString &s2);
        inline friend bool operator<=(const char *s1, const KPString &s2);
        inline friend bool operator>(const char *s1, const KPString &s2);
        inline friend bool operator>=(const char *s1, const KPString &s2);
        inline friend bool operator!=(const char *s1, const KPString &s2);
        inline friend void swap(KPString &s1, KPString &s2);
    public:
        KPString();
        KPString(const KPString &s);
        KPString(const char *s);
        KPString(char c, int n = 1);
        ~KPString();
        operator const char *() const;
        char &operator[](int index);
        char operator[](int index) const;
        KPString &operator=(const KPString &s);
        KPString &operator=(const char *s);
        KPString operator+(const KPString &s) const;
        KPString operator+(const char *s) const;
        bool operator==(const KPString &s) const;
        bool operator==(const char *s) const;
        bool operator<(const KPString &s) const;
        bool operator<(const char *s) const;
        bool operator<=(const KPString &s) const;
        bool operator<=(const char *s) const;
        bool operator>(const KPString &s) const;
        bool operator>(const char *s) const;
        bool operator>=(const KPString &s) const;
        bool operator>=(const char *s) const;
        bool operator!=(const KPString &s) const;
        bool operator!=(const char *s) const;
        KPString &operator+=(const KPString &s);
        KPString &operator+=(const char *s);
        int length() const;
        bool is_empty() const;
        bool is_whitespace() const;
        KPString substr(int index = 0, int len = -1) const;
        KPString copy() const;
        KPString &cut(int index = 0, int len = -1);
        KPString &replace_substr(const KPString &s, int index = 0, int len=-1);
        KPString &replace_substr(const char *s, int index = 0, int len = -1);
        KPString &insert(const KPString &s, int index = 0);
        KPString &insert(const char *s, int index = 0);
        KPString &to_upper();
        KPString &to_lower();
        KPString &clear();
        int index_of(const KPString &s, int start_index = 0) const;
        int index_of(const char *s, int start_index = 0) const;
        int index_of(char c, int start_index = 0) const;
        bool contains(const KPString &s) const;
        bool contains(const char *s) const;
        bool contains(char c) const;
        KPList<KPString> tokens(const char *separators=" \t\n\v\r\f") const;
        KPList<KPString> tokens(char separator) const;
        KPString &read_line(istream &stream);
        KPString &
			read_token(istream &stream, const char *separators=" \t\n\v\r\f");

        static const int max_token_size;
        static const int max_line_size;

    protected:
        static void invalid_args_error(const char *fname);
        static void invalid_index_error(const char *fname);
        class Data {
            public:
                Data(): ref_count(0), length(0) { chars[0] = '\0'; }
                unsigned int ref_count;
                int length;
                char chars[1];
        };
        static Data *new_data(int length);
        void replace_data(int length);
        void replace_data(Data *data);
        void make_unique();
        char *chars();
        const char *chars() const;
    protected:
        Data *my_data;
        static Data null_data;
};

/***************************************************************************/

#include "KPList.h"

inline char *KPString::chars()
{ return my_data->chars; }

inline const char *KPString::chars() const
{ return my_data->chars; }

inline ostream &operator<<(ostream &stream, const KPString &s)
{ stream << s.chars(); return stream; }

inline bool operator==(const char *s1, const KPString &s2)
{ return (strcmp(s1, s2.chars()) == 0); }

inline bool operator<(const char *s1, const KPString &s2)
{ return (strcmp(s1, s2.chars()) < 0); }

inline bool operator<=(const char *s1, const KPString &s2)
{ return (strcmp(s1, s2.chars()) <= 0); }

inline bool operator>(const char *s1, const KPString &s2)
{ return (strcmp(s1, s2.chars()) > 0); }

inline bool operator>=(const char *s1, const KPString &s2)
{ return (strcmp(s1, s2.chars()) >= 0); }

inline bool operator!=(const char *s1, const KPString &s2)
{ return (strcmp(s1, s2.chars()) != 0); }

inline void swap(KPString &s1, KPString &s2)
{ KPString::Data *tmp = s1.my_data; s1.my_data = s2.my_data; s2.my_data = tmp; }

inline KPString::KPString(): my_data(&null_data)
{ /* do nothing */ }

inline KPString::KPString(const KPString &s): my_data(&null_data)
{ replace_data(s.my_data); }

inline KPString::KPString(const char *s): my_data(&null_data)
{ const int length = ::strlen(s); replace_data(length);
  ::memcpy(chars(), s, length); }

inline KPString::KPString(char c, int n): my_data(&null_data)
{ replace_data(n); ::memset(chars(), c, n); }

inline KPString::~KPString()
{ if (my_data != &null_data && --my_data->ref_count == 0) free(my_data); }

inline KPString::operator const char *() const
{ return my_data->chars; }

inline char &KPString::operator[](int index)
{ if (index < 0) index += length();
  if (index < 0 || index >= length()) invalid_index_error("operator[]");
  make_unique(); return chars()[index]; }

inline char KPString::operator[](int index) const
{ if (index < 0) index += length();
  if (index < 0 || index >= length()) invalid_index_error("operator[]");
  return chars()[index]; }

inline KPString &KPString::operator=(const KPString &s)
{ replace_data(s.my_data); return *this; }

inline KPString &KPString::operator=(const char *s)
{ const int length = ::strlen(s); replace_data(length);
  ::memcpy(chars(), s, length); return *this; }


inline bool KPString::operator==(const KPString &s) const
{ return (length() == s.length()) &&
         (memcmp(chars(), s.chars(), length()) == 0); }

inline bool KPString::operator==(const char *s) const
{ return (strcmp(chars(), s) == 0); }

inline bool KPString::operator<(const KPString &s) const
{ return (strcmp(chars(), s.chars()) < 0); }

inline bool KPString::operator<(const char *s) const
{ return (strcmp(chars(), s) < 0); }

inline bool KPString::operator<=(const KPString &s) const
{ return (strcmp(chars(), s.chars()) <= 0); }

inline bool KPString::operator<=(const char *s) const
{ return (strcmp(chars(), s) <= 0); }

inline bool KPString::operator>(const KPString &s) const
{ return (strcmp(chars(), s.chars()) > 0); }

inline bool KPString::operator>(const char *s) const
{ return (strcmp(chars(), s) > 0); }

inline bool KPString::operator>=(const KPString &s) const
{ return (strcmp(chars(), s.chars()) >= 0); }

inline bool KPString::operator>=(const char *s) const
{ return (strcmp(chars(), s) >= 0); }

inline bool KPString::operator!=(const KPString &s) const
{ return (length() != s.length()) ||
         (memcmp(chars(), s.chars(), length()) != 0); }

inline bool KPString::operator!=(const char *s) const
{ return (strcmp(chars(), s) != 0); }

inline KPString &KPString::operator+=(const KPString &s)
{ *this = *this + s; return *this; }

inline KPString &KPString::operator+=(const char *s)
{ *this = *this + s; return *this; }

inline int KPString::length() const
{ return my_data->length; }

inline bool KPString::is_empty() const
{ return my_data == &null_data; }

inline KPString KPString::copy() const
{ KPString newstring(*this); return newstring; }

inline KPString &KPString::clear()
{ replace_data(0); return *this; }

inline int KPString::index_of(const KPString &s, int start_index) const
{ return index_of(s.chars(), start_index); }

inline bool KPString::contains(const KPString &s) const
{ return (index_of(s, 0) >= 0); }

inline bool KPString::contains(const char *s) const
{ return (index_of(s, 0) >= 0); }

inline bool KPString::contains(char c) const
{ return (index_of(c, 0) >= 0); }

#endif /* KP_STRING_DEFINED */
