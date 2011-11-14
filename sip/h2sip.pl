#! /usr/bin/env perl

while (<>) {
    /^\s*#/ and next;
    /^\s*using/ and next;
    /^\s*Q_OBJECT/ and next;
    /\s+~[a-zA-Z]+\s*\(/ and next;
    /^\s*(friend\s+)?class\s+.*;\s*$/ and next;
    /^\s*\/\// and next;

    s/\/\/.*//;

    if (/^\s*(class|struct)\s+([_a-zA-Z0-9]+)\s*;\s*$/) {
	print "$1 $2;\n";
	next;
    }

    if (/^\s*enum.*;\s*$/) {
	print "$1\n";
	next;
    }

    if (/^\s*class\s+([_a-zA-Z0-9]+)(\s*:\s*\S+\s*(.*))?/) {
        print "class $1";
        print ": $3" if $2;
        print "\n";
	print "{\n";
        print "%TypeHeaderCode\n";
        print "#include \"QWave2/$1.h\"\n";
	print "using namespace QWave2;\n";
	print "%End\n\n";
	<>;
        next;
    }

    if (/^\s*private:/) {
        while (<>) {
            /^\s*protected:|^\s*public:/ and last;
            /^\s*};/ and last;
        }
    }

    s/([^( =,)]+)\s+([^( =,)]+)\s*(,|\))/\1\3/g; 
    # ScrollBar(QWidget *parent=0, ...
    # --> ScrollBar(QWidget* parent=0, ...
    s/(\S+)\s+\*(\S)/\1* \2/g;
    # ScrollBar(QWidget* parent=0, ...
    # --> ScrollBar(QWidget*=0, ...
    s/\s+[^= ,)]+=/=/g;
    s/\)\s*{[^}]+}\s*$/\);\n/;

    s/char\s+const/const char/g;
    s/double\s+const/const double/g;

    if (/\)\s*$/) {
        chomp;
        print "$_;\n";
        $n = 0;
        $flag = 0;
        while (<>) {
            while (/{/) { s/{//; $n++; $flag=1; }
            while (/}/) { s/}//; $n--; }
            last if ($n==0 && $flag);
        }
        next;
    }

    if (/^\s*\/\*\s*$/) {
        while (<>) {
            last if /\*\//;
        }
        s/^.*\*\///;
    }

    /^\s*$/ and next;
    s/,\s*$/,/;
    s/^\s*([^,(]+((,)|(\)\s*;)))$/\1/;
    s/^\s*}\s*$/};/;

    print;
    print "\n\n" if /^\s*}\s*;\s*$/;
}
