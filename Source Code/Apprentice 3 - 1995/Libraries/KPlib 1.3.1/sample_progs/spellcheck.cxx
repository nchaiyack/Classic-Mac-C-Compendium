// Written by Keith Pomakis during the summer of 1994.
// Released to the public domain on October 10, 1994.

// This is just a sample program to illustrate how easy it is to write
// complex programs with my class library.  The following program accepts
// input through stdin, and prints out all inputted words that do not exist
// in the on-line dictionary (specified by WORD_FILE).

#include <iostream.h>
#include <fstream.h>
#include <ctype.h>
#include "../KPbasic.h"
#include "../KPList.h"
#include "../KPSet.h"
#include "../KPString.h"

#define WORD_FILE   "/usr/dict/words"
#define SEPARATORS  " \t\n,.<>:;\042!@#$%^&*()-=+[]{}\\|`~?"

// Explicit instantiations of required templates.  This is necessary for
// g++ 2.6.x in conjunction with the "-fno-implicit-templates" option, but
// may not be necessary in the future.

template class KPSet<KPString>;
template class KPList<KPString>;
template class KPComparableList<KPString>;
template class KPSortableList<KPString>;
template class KPReadOnlyIterator<KPString>;
template class KPIterator<KPString>;

bool is_valid(const KPString &s)
{ return isalpha(s[0]); }

int
main()
{
    KPSet<KPString> dictset, wordset;
    KPString word;

    // Since the dictionary is sorted, reading it into a set is very quick.
    cout << "Reading dictionary..." << flush;
    ifstream dictstream(WORD_FILE, ios::in);
    while (word.read_token(dictstream) != "")
        dictset += word.to_lower();
    dictstream.close();
    cout << " done!\n";

    while (word.read_token(cin, SEPARATORS) != "")
        wordset += word.to_lower();

    wordset = wordset.all_such_that(is_valid);
    cout << "Number of unique words in document: " << wordset.size() << '\n';

    cout << "Words not in dictionary:\n";
    KPList<KPString> misspelt = wordset - dictset;
    KPReadOnlyIterator<KPString> iter(misspelt);
    FOREACH(iter)
        cout << "    " << *iter << '\n';

	return true;
}
