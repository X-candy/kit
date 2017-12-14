default:
	cd src/ui;qmake-qt4;make;cp kit $(PWD) && echo "\n\t\033[32mBuild sucessful\n";date;echo

clean:
	cd src/ui;qmake;make clean && echo "\n\t\033[32mClean sucessful\n"

