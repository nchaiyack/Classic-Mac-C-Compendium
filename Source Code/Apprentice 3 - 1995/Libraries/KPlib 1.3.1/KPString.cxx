// A module of KPlib v1.3.1.
// Written by Keith Pomakis during the summer of 1994.
// Released to the public domain on October 10, 1994.

#include <iostream.h>
#include <stdlib.h>
#include <iomanip.h>
#include <string.h>
#include <ctype.h>
#include "KPbasic.h"
#include "KPString.h"
#include "KPList.h"

const int KPString::max_token_size = 200;
const int KPString::max_line_size = 600;
KPString::Data KPString::null_data;

/****************************************************************************/

KPString::Data *
KPString::new_data(int length)
{
    if (length > 0) {
        Data *data = (Data *) malloc(sizeof(Data) + length);
        check_mem(data);
        data->ref_count = 0;
        data->length = length;
        data->chars[length] = '\0';
        return data;
    }
    else
        return &null_data;
}

void
KPString::replace_data(int length)
{
    if (length == my_data->length && my_data->ref_count <= 1)
        return;

    if (my_data != &null_data && --my_data->ref_count == 0)
        free(my_data);

    if (length > 0) {
        my_data = (Data *) malloc(sizeof(Data) + length);
        check_mem(my_data);
        my_data->ref_count = 1;
        my_data->length = length;
        my_data->chars[length] = '\0';
    }
    else
        my_data = &null_data;
}

void
KPString::replace_data(Data *data)
{
    if (my_data != &null_data && --my_data->ref_count == 0)
        free(my_data);

    if (data != &null_data)
        data->ref_count++;
    my_data = data;
}

void
KPString::make_unique()
{
    if (my_data->ref_count > 1) {
        Data *data = new_data(length());
        ::memcpy(data->chars, chars(), length());
        my_data->ref_count--;
        my_data = data;
        my_data->ref_count++;
    }
}

/****************************************************************************/

istream &
operator>>(istream &stream, KPString &s)
{
    char buffer[KPString::max_token_size+1];

    stream >> setw(KPString::max_token_size) >> buffer;
    if (stream) {
        const int new_length = ::strlen(buffer);
        s.replace_data(new_length);
        ::memcpy(s.chars(), buffer, new_length);
    }
    else
        s.replace_data(0);

    return stream;
}

/****************************************************************************/

KPString
operator+(const char *s1, const KPString &s2)
{
    const int s1_length = ::strlen(s1);

    if (s1_length == 0)
        return s2;
    else {
        KPString newstring;
        newstring.replace_data(s1_length + s2.length());
        ::memcpy(newstring.chars(), s1, s1_length);
        ::memcpy(&(newstring.chars())[s1_length], s2.chars(), s2.length());
        return newstring;
    }
}

/****************************************************************************/

KPString
KPString::operator+(const KPString &s) const
{
    if (length() == 0)
        return s;
    else if (s.length() == 0)
        return *this;
    else {
        KPString newstring;
        newstring.replace_data(length() + s.length());
        ::memcpy(newstring.chars(), chars(), length());
        ::memcpy(&(newstring.chars())[length()], s.chars(), s.length());
        return newstring;
    }
}

/****************************************************************************/

KPString
KPString::operator+(const char *s) const
{
    const int s_length = ::strlen(s);

    if (s_length == 0)
        return *this;
    else {
        KPString newstring;
        newstring.replace_data(length() + s_length);
        ::memcpy(newstring.chars(), chars(), length());
        ::memcpy(&(newstring.chars())[length()], s, s_length);
        return newstring;
    }
}

/****************************************************************************/

bool
KPString::is_whitespace() const
{
    if (my_data == &null_data)
		return false;

    for (register const char *p = chars(); *p; p++)
        if (!isspace(*p))
			return false;

	return true;
}

/****************************************************************************/

KPString
KPString::substr(int index, int len) const
{
    // A negative index specifies an index from the right of the string.
    if (index < 0)
        index += length();

    // A length of -1 specifies the rest of the string.
    if (len == -1)
        len = length() - index;

    if (index<0 || index>=length() || len<0 || len>length()-index)
        invalid_args_error("substr()");

    KPString newstring;
    newstring.replace_data(len);
    ::memcpy(newstring.chars(), &chars()[index], len);

    return newstring;
}

/****************************************************************************/

KPString &
KPString::cut(int index, int len)
{
    if (len == 0)
        return *this;

    // A negative index specifies an index from the right of the string.
    if (index < 0)
        index += length();

    // A length of -1 specifies the rest of the string.
    if (len == -1)
        len = length() - index;

    if (index<0 || index>=length() || len<0 || len>length()-index)
        invalid_args_error("cut()");

    Data *data = new_data(length() - len);
    if (index > 0)
        ::memcpy(data->chars, chars(), index);
    ::strcpy(&data->chars[index], &chars()[index+len]);
    replace_data(data);

    return *this;
}

/****************************************************************************/

KPString &
KPString::replace_substr(const KPString &s, int index, int len)
{
    // A negative index specifies an index from the right of the string.
    if (index < 0)
        index += length();

    // A length of -1 specifies the rest of the string.
    if (len == -1)
        len = length() - index;

    if (index<0 || index>=length() || len<0 || len>length()-index)
        invalid_args_error("replace_substr()");

    if (len == s.length() && my_data->ref_count == 1)
        ::memcpy(&chars()[index], s.chars(), len);
    else {
        Data *data = new_data(length() - len + s.length());
        if (index > 0)
            ::memcpy(data->chars, chars(), index);
        if (s.length() > 0)
            ::memcpy(&data->chars[index], s.chars(), s.length());
        ::strcpy(&data->chars[index+s.length()], &chars()[index+len]);
        replace_data(data);
    }

    return *this;
}

/****************************************************************************/

KPString &
KPString::replace_substr(const char *s, int index, int len)
{

    // A negative index specifies an index from the right of the string.
    if (index < 0)
        index += length();

    // A length of -1 specifies the rest of the string.
    if (len == -1)
        len = length() - index;

    if (index<0 || index>=length() || len<0 || len>length()-index)
        invalid_args_error("replace_substr()");

    const int s_length = ::strlen(s);

    if (len == s_length && my_data->ref_count == 1)
        ::memcpy(&chars()[index], s, len);
    else {
        Data *data = new_data(length() - len + s_length);
        if (index > 0)
            ::memcpy(data->chars, chars(), index);
        if (s_length > 0)
            ::memcpy(&data->chars[index], s, s_length);
        ::strcpy(&data->chars[index+s_length], &chars()[index+len]);
        replace_data(data);
    }

    return *this;
}

/****************************************************************************/

KPString &
KPString::insert(const KPString &s, int index)
{
    // A negative index specifies an index from the right of the string.
    if (index < 0)
        index += length();

    if (index < 0 || index >= length())
        invalid_index_error("insert()");

    if (s.length() > 0) {
        Data *data = new_data(length() + s.length());
        if (index > 0)
            ::memcpy(data->chars, chars(), index);
        ::memcpy(&data->chars[index], s.chars(), s.length());
        ::strcpy(&data->chars[index+s.length()], &chars()[index]);
        replace_data(data);
    }

    return *this;
}

/****************************************************************************/

KPString &
KPString::insert(const char *s, int index)
{
    // A negative index specifies an index from the right of the string.
    if (index < 0)
        index += length();

    if (index < 0 || index >= length())
        invalid_index_error("insert()");

    const int s_length = ::strlen(s);

    if (s_length > 0) {
        Data *data = new_data(length() + s_length);
        if (index > 0)
            ::memcpy(data->chars, chars(), index);
        ::memcpy(&data->chars[index], s, s_length);
        ::strcpy(&data->chars[index+s_length], &chars()[index]);
        replace_data(data);
    }

    return *this;
}

/****************************************************************************/

KPString &
KPString::to_upper()
{
    make_unique();

    for (register char *p = chars(); *p; p++)
        if (islower(*p))
            *p = toupper(*p);

    return *this;
}

/****************************************************************************/

KPString &
KPString::to_lower()
{
    make_unique();

    for (register char *p = chars(); *p; p++)
        if (isupper(*p))
            *p = tolower(*p);

    return *this;
}

/****************************************************************************/

int
KPString::index_of(const char *s, int start_index) const
{
    // A negative index specifies an index from the right of the string.
    if (start_index < 0)
        start_index += length();

    if (start_index < 0 || start_index >= length())
        invalid_index_error("index_of()");

    const char *index;
    if (!(index = strstr(&chars()[start_index], s)))
        return -1;
    else
        return index - chars();
}

/****************************************************************************/

int
KPString::index_of(char c, int start_index) const
{
    // A negative index specifies an index from the right of the string.
    if (start_index < 0)
        start_index += length();

    if (start_index < 0 || start_index >= length())
        invalid_index_error("index_of()");

    const char *index;

    if (c == '\0')
        return -1;

    else if (!(index = (char *) ::memchr(&chars()[start_index], c,
                                         length()-start_index)))
        return -1;
    else
        return index - chars();
}

/****************************************************************************/

KPList<KPString>
KPString::tokens(const char *separators) const
{
    KPList<KPString> list;
    int token_length, index = 0;

    do {
        index += ::strspn(&chars()[index], separators);
        token_length = ::strcspn(&chars()[index], separators);
        if (token_length > 0)
            list.add_to_tail(substr(index, token_length));
        index += token_length;
    } while (token_length > 0);

    return list;
}

/****************************************************************************/

KPList<KPString> KPString::tokens(char separator) const
{
    char separators[2];
    separators[0] = separator;
    separators[1] = '\0';
    return tokens(separators);
}

/****************************************************************************/

KPString &
KPString::read_line(istream &stream)
{
    char buffer[max_line_size+1];
    stream.getline(buffer, max_line_size);
    const int new_length = ::strlen(buffer);
    Data *data = new_data(new_length);
    ::memcpy(data->chars, buffer, new_length);
    replace_data(data);

    return *this;
}

/****************************************************************************/

KPString &
KPString::read_token(istream &stream, const char *separators)
{
    register int c;
    const int num_of_separators = ::strlen(separators);

    // Scan for the first character of the token.
    do {
        c = stream.get();
        if (c == EOF) {
            replace_data(0);
            return *this;
        }
    } while (::memchr(separators, c, num_of_separators));

    register int i = 0;
    char buffer[max_token_size];

    // Read in the token.
    do {
        buffer[i++] = c;
        c = stream.get();
    } while (c != EOF && !::memchr(separators, c, num_of_separators));

    stream.putback(c);
    replace_data(i);
    ::memcpy(chars(), buffer, i);

    return *this;
}

/****************************************************************************/

void
KPString::invalid_args_error(const char *fname)
{
    cerr << "KPString::" << fname << " - invalid arguments\n";
    exit(EXIT_FAILURE);
}

/****************************************************************************/

void
KPString::invalid_index_error(const char *fname)
{
    cerr << "KPString::" << fname << " - invalid index\n";
    exit(EXIT_FAILURE);
}

/****************************************************************************/
