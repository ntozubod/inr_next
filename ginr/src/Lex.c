/*
 * Copyright (c) 1985, J Howard Johnson, University of Waterloo.
 *
 * This software was developed while I was a student and, later, professor
 * at the University of Waterloo.  It has only minimal enhancements and bug
 * fixes from later than August 1988.  It was released under the GPLv3
 * licence on July 26, 2010.
 *                 -- J Howard Johnson ( j.howard.johnson *at* gmail.com )
 *
 * This file is part of INR.
 *
 *   INR is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   INR is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with INR.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "O.h"
#include "y.tab.h"

A_OBJECT    A, Atemp;
Tn_OBJECT   TAlist;
A_OBJECT    * Alist;

T2_OBJECT   TT2;

char * pad20( char *s )
{
    static char tmp[ 41 ];
    if ( strlen( s ) >= 20 ) return( s );
    else {
        (void) strcpy( tmp, s );
        (void) strcat( tmp, " -------------------" );
        tmp[ 20 ] = 0;
        return( tmp );
    }
}

#define PROMT   if(isatty(fileno(fpin))&&isatty(fileno(fpout)))printf("--* ");

int ch = ' ';
char    token[512];
int in_string = 0;

char *copyof( char *str )
{
    return( strcpy( Salloc( strlen( str ) + 1 ), str ) );
}

int yylex()
{
    int li, d, lflag, in_comment;

    fflush( fpout );
    if ( in_string ) {
        ch = getc( fpin );
        if ( ch == '\'' ) {
            ch = getc( fpin );
            if ( ch != '\'' ) {
                in_string = 0;
                return( RPAREN );
            }
        }
        if ( ch == '\\' ) {
            ch = getc( fpin );
            switch( ch ) {
            case 'n':
                ch = '\n';
                break;
            case 't':
                ch = '\t';
                break;
            case '_':
                ch = ' ';
                break;
            case 'x':
                d = getc( fpin );
                if ( d >= '0' && d <= '9' ) d = d - '0';
                else if ( d >= 'a' && d <= 'f' ) d = d - 'a' + 10;
                else if ( d >= 'A' && d <= 'F' ) d = d - 'A' + 10;
                else Error( "Unexpected Hex digit" );
                ch = d << 4;
                d = getc( fpin );
                if ( d >= '0' && d <= '9' ) d = d - '0';
                else if ( d >= 'a' && d <= 'f' ) d = d - 'a' + 10;
                else if ( d >= 'A' && d <= 'F' ) d = d - 'A' + 10;
                else Error( "Unexpected Hex digit" );
                ch += d;
            }
        }
        if ( ch == EOF ) Error( "End of file in string" );
        yylval.up = P_create( T2_length( TT2, ch + 2 ),
                              T2_name( TT2, ch + 2 ) );
        return( PNAME );
    }
    in_comment = 0;
    while( ch == ' ' || ch == '\t' || ch == '\n' || ch == '#'
            || in_comment ) {
        if( in_comment && !isatty(fileno(fpout)) ) putc( ch, fpout );
        if ( ch == '#'  ) in_comment = 1;
        if ( ch == '\n' ) in_comment = 0;
        if ( ch == EOF  ) Error( "End of file in comment" );
        ch = getc( fpin );
    }
    if ( ch == EOF ) return( 0 );
    d = ch;
    ch = ' ';
    switch( d ) {
    case '!':
        return( EXCLAM );
    /*  case '"':   not used    */
    /*  case '#':   COMMENT     */
    case '$':
        return( DOLLAR );
    case '%':
        return( PERCENT );
    case '&':
        return( AMPERSAND );
    case '\'':
        in_string = 1;
        return( LPAREN );

    case '(':
        return( LPAREN );
    case ')':
        return( RPAREN );
    case '*':
        return( STAR );
    case '+':
        return( PLUS );
    case ',':
        return( COMMA );
    case '-':
        return( MINUS );
    /*  case '.':   ALPHANUMERIC    */
    case '/':
        return( SLASH );

    case ':':
        return( COLON );
    case ';':
        return( SEMI );
    /*  case '<':   not used    */
    case '=':
        return( EQUAL );
    /*  case '>':   not used    */
    case '?':
        return( QUESTION );

    case '@':
        return( AT );

    case '[':
        return( LBRACK );
    case '\\':
        return( BSLASH );
    case ']':
        return( RBRACK );
    case '^':
        return( CIRCUMFLEX );
    /*  case '_':   ALPHANUMERIC    */

    /*  case '`':   TOKEN QUOTE */

    case '{':
        return( LBRACE );
    case '|':
        return( VBAR );
    case '}':
        return( RBRACE );
        /*  case '~':   not used    */

    case '"':
    case '<':
    case '>':
    case '~':
        fprintf( fpout, "Reserved character: %c\n", d );
        return( d );
    }
    li = 0;
    ch = d;
    lflag = 1;
    if ( ch == '`' ) {
        ch = getc( fpin );
        while ( ch != EOF ) {
            if ( ch == '`' ) {
                ch = getc( fpin );
                if ( ch != '`' ) break;
            }
            if ( ch == '\\' ) {
                ch = getc( fpin );
                switch( ch ) {
                case 'n':
                    ch = '\n';
                    break;
                case 't':
                    ch = '\t';
                    break;
                case '_':
                    ch = ' ';
                    break;
                case 'x':
                    d = getc( fpin );
                    if ( d >= '0' && d <= '9' ) d = d - '0';
                    else if ( d >= 'a' && d <= 'f' ) d = d - 'a' + 10;
                    else if ( d >= 'A' && d <= 'F' ) d = d - 'A' + 10;
                    else Error( "Unexpected Hex digit" );
                    ch = d << 4;
                    d = getc( fpin );
                    if ( d >= '0' && d <= '9' ) d = d - '0';
                    else if ( d >= 'a' && d <= 'f' ) d = d - 'a' + 10;
                    else if ( d >= 'A' && d <= 'F' ) d = d - 'A' + 10;
                    else Error( "Unexpected Hex digit" );
                    ch += d;
                }
            }
            token[ li++ ] = ch;
            ch = getc( fpin );
        }
        if ( li == 0 ) return( CIRCUMFLEX );
        token[ li ] = 0;
        yylval.up = P_create( li, token );
        return( LNAME );
    } else {
        while ( lflag && ch != EOF ) {
            token[ li++ ] = ch;
            ch = getc( fpin );

            if ( li!=2 || token[1]!='.' || !isdigit(token[0]) ) {
                lflag = 0;
                switch( ch ) {
                case '.':
                case '_':

                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':

                case 'A':
                case 'B':
                case 'C':
                case 'D':
                case 'E':
                case 'F':
                case 'G':
                case 'H':
                case 'I':
                case 'J':
                case 'K':
                case 'L':
                case 'M':
                case 'N':
                case 'O':
                case 'P':
                case 'Q':
                case 'R':
                case 'S':
                case 'T':
                case 'U':
                case 'V':
                case 'W':
                case 'X':
                case 'Y':
                case 'Z':

                case 'a':
                case 'b':
                case 'c':
                case 'd':
                case 'e':
                case 'f':
                case 'g':
                case 'h':
                case 'i':
                case 'j':
                case 'k':
                case 'l':
                case 'm':
                case 'n':
                case 'o':
                case 'p':
                case 'q':
                case 'r':
                case 's':
                case 't':
                case 'u':
                case 'v':
                case 'w':
                case 'x':
                case 'y':
                case 'z':
                    lflag = 1;
                    break;
                }
            }

        }
    }
    token[ li ] = 0;
    yylval.up = P_create( li, token );
    return( PNAME );
}

char Notice[]
   = "Copyright (c) 1985, 1988, J Howard Johnson, University of Waterloo";

int main( int argc, char *argv[] )
{
    int ti, result;
    char tstr[3];
    char file_in[50], file_out[50], rpt_out[50];

    fpin  = stdin;
    fpout = stdout;

    if( argc > 3) {
        printf( "Usage: inr [ input_file ] [ output_file ]\n" );
        exit (1) ;
    }

    if( argc > 2 ) {
        strcpy( file_out, argv[2] ) ;
        fpout = fopen( file_out, FOPEN_WRITE );
    }

    if( argc > 1 ) {
        strcpy( file_in, argv[1] ) ;
        fpin = fopen( file_in, FOPEN_READ );
    }

    if (fpin == 0 || fpout == 0 ) {
        if( fpin )   strcpy( rpt_out, file_out );
        else if (fpout ) strcpy( rpt_out, file_in );
        else {
            strcpy( rpt_out, file_in );
            strcat( rpt_out, ", " );
            strcat( rpt_out, file_out );
        }
        printf ( "Problem with %s file(s) opens. -- aborting\n", rpt_out) ;
        exit (1) ;
    }

    if ( isatty(fileno(fpout)) ) {

fprintf( fpout, "\n" );
fprintf( fpout, "II  N     N  RRRRRR    I N R     " );
fprintf( fpout, "Version %s (Mar 25, 1988)\n", Version );
fprintf( fpout, "II  N N   N  R    RR" );
fprintf( fpout, "             Copyright (C) 1988 J Howard Johnson\n" );
fprintf( fpout, "II  N  N  N  RRRRRR    modified  %s\n", Date );
fprintf( fpout, "II  N   N N  R    R\n" );
fprintf( fpout, "II  N    NN  R     R" );
fprintf( fpout, "                              (For help type   `:help;')\n" );
fprintf( fpout, "This program comes with ABSOLUTELY NO WARRANTY; " );
fprintf( fpout, "for details type `:help w;'.\n" );
fprintf( fpout, "This is free software, and you are welcome " );
fprintf( fpout, "to redistribute it under certain\n" );
fprintf( fpout, "conditions; type `:help c;' for details.\n" );
fprintf( fpout, "\n" );

    } else {
        fprintf ( fpout, "I N R -- V %s, modified %s\n", Version, Date ) ;
        fprintf ( fpout, "Copyright (C) 1988 J Howard Johnson\n" ) ;
        fprintf ( fpout, "Distributed under GPLv3 (see COPYING)\n" ) ;
        if ( fpin != stdin )
            fprintf( fpout, "  (Source file: %s)", file_in ) ;
        fprintf( fpout, "\n\n\n" );
    }

    TT2 = T2_create();

    result = T2_insert( TT2, "^^", 2 );
    assert( result == 0 );
    result = T2_insert( TT2, "-|", 2 );
    assert( result == 1 );
    for( ti = 0; ti < 256; ti++ ) {
        tstr[ 0 ] = ti;
        tstr[ 1 ] = '\0';
        result = T2_insert( TT2, tstr, 1 );
        assert( result == ti + 2 );
    }

    Alist = (A_OBJECT *) Salloc( 100 * sizeof( A_OBJECT ) );
    TAlist = Tn_create();
    result = Tn_insert( TAlist, "_Last_", 6 );
    assert( result == 0 );
    Alist[ 0 ] = A_create();

    pr_time_diff();
    PROMT
    if ( yyparse() != 0 )
        Error( "yyparse returned unexpectedly" );
    T2_destroy( TT2 );
    Tn_destroy( TAlist );
    if ( A_report ) {
        fprintf( fpout, "\n" );
        Tn_stats();
        V_stats();
        R_stats();
        U_stats();
    }
    exit( 0 );
}

void yyerror( char *str )
{
    fprintf( fpout, "*** %s ***\n", str );
}

int tonum( char *p )
{
    int acum, c;
    acum = 0;
    while ( ( c = *p++ ) ) {
        if ( c < '0' || c > '9' ) return( -1 );
        acum = acum * 10 + c - '0';
    }
    return( acum );
}
