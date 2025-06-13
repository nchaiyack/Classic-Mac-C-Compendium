/*
    This program accepts a list of tokens via standard in, and sends
    them to standard out in formatted columns.  The ordering of the
    tokens in the columns is `natural'.  That is, they are arranged
    top-to-bottom, and then left-to-right.  The screen width can be
    specified with the `-w<width>' option (default is 80).  the number
    of columns to display is chosen accordingly.

    Written by Keith Pomakis on October 14, 1994.  Public Domain.
*/

#include <iostream.h>
#include <strstream.h>
#include <ctype.h>
#include "../KPbasic.h"
#include "../KPList.h"
#include "../KPArray.h"
#include "../KPString.h"

// Explicit instantiations of required templates.  This is necessary for
// g++ 2.6.x in conjunction with the "-fno-implicit-templates" option, but
// may not be necessary in the future.

template class KPList<KPString>;
template class KPReadOnlyIterator<KPString>;
template class KPArray<KPString>;
template class KPArray< KPArray<KPString> >;
template int max(int, int);
template int min(int, int);

const char *const help_string =
    "This program accepts a list of tokens via standard in, and sends\n"
    "them to standard out in formatted columns.  The ordering of the\n"
    "tokens in the columns is `natural'.  That is, they are arranged\n"
    "top-to-bottom, and then left-to-right.  The screen width can be\n"
    "specified with the `-w<width>' option (default is 80).  the number\n"
    "of columns to display is chosen accordingly.\n\n"
    "Written by Keith Pomakis on October 14, 1994.  Public Domain.\n";

static inline KPString
base(const KPString &string)
{
    KPList<KPString> components;
    components = string.tokens('/');
    return components.tail();
}

int
main(int argc, char *argv[])
{
    int screen_width;
    KPString progname = base(argv[0]);
    KPString usage_string;

    usage_string = (KPString) "Usage: " + progname + " -w<width>\n"
                            + "       " + progname + " -help\n";

    if (argc > 2 || (argc == 2 && (argv[1][0] != '-'))) {
        cerr << usage_string;
        exit(EXIT_FAILURE);
    }

    if (argc == 2)
        switch(argv[1][1]) {
            case 'h':
                cout << help_string;
                exit(EXIT_SUCCESS);
                break;
            case 'w':
                if (!isdigit(argv[1][2])) {
                    cerr << usage_string;
                    exit(EXIT_FAILURE);
                }
                else {
                    istrstream strin(&argv[1][2]);
                    strin >> screen_width;
                }
                break;
            default:
                cerr << usage_string;
                exit(EXIT_FAILURE);
        }
    else
        screen_width = 80;

    KPList<KPString> tokens;
    KPString input_token;
    int max_length = 0;
    int i, j, k;
    const int min_separation = 2;

    // Read the tokens into a list.
    while (!cin.eof()) {
        input_token.read_token(cin);
        if (!cin.eof()) {
            if (input_token.length() > screen_width - min_separation)
                input_token.cut(screen_width - min_separation);
            max_length = max(max_length, input_token.length());
            tokens += input_token;
        }
    }

    // Figure out how to display them.
    const int potential_cols = screen_width / (max_length + min_separation);
    const int rows = (tokens.size() + potential_cols - 1 ) / potential_cols;
    const small_row_size = tokens.size() / rows;
    const int cols = (small_row_size*rows == tokens.size())?
                                    small_row_size : small_row_size + 1;
    const int full_rows = (tokens.size()%rows == 0)? rows : tokens.size()%rows;
    const int full_cols = (rows == full_rows)? cols : cols - 1;
    const int col_width = screen_width / cols;

    KPReadOnlyIterator<KPString> iter(tokens);

    // Using column-oriented indexing.  Read the tokens into the structure.
    KPArray< KPArray<KPString> > structure(cols);
    for (i = 0; i < full_cols; i++) {
        structure[i].resize(rows);
        for (j = 0; j < rows; j++) {
            structure[i][j] = *iter;
            iter++;
        }
    }
    if (cols != full_cols) {
        structure[cols-1].resize(full_rows);
        for (j = 0; j < full_rows; j++) {
            structure[cols-1][j] = *iter;
            iter++;
        }
    }

    // Print out the structure.
    for (j = 0; j < full_rows; j++) {
        for (i = 0; i < cols; i++) {
            cout << structure[i][j];
            if (i < cols - 1)
                for (k = structure[i][j].length(); k < col_width; k++)
                    cout << ' ';
        }
        cout << '\n';
    }
    for (j = full_rows; j < rows; j++) {
        for (i = 0; i < full_cols; i++) {
            cout << structure[i][j];
            if (i < full_cols - 1)
                for (k = structure[i][j].length(); k < col_width; k++)
                    cout << ' ';
        }
        cout << '\n';
    }

    return 0;
}
