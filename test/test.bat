@echo off
setlocal

set REPOROOT=file:///C:/_test_repo
set REPOPATH=C:\_test_repo
set WC=C:\_test_wc

:: create empty repos & working copy

if exist %REPOPATH% rd /s /q %REPOPATH%
svnadmin create %REPOPATH%

if exist %WC% rd /s /q %WC%
svn co %REPOROOT% %WC%

:: r1: initial content

mkdir %WC%\trunk
mkdir %WC%\branches
mkdir %WC%\tags

echo "foo" > %WC%\trunk\foo.txt
echo "bar" > %WC%\trunk\bar.txt
echo "baz" > %WC%\trunk\baz.txt

svn add %WC%\trunk
svn add %WC%\branches
svn add %WC%\tags

svn ci %WC% -m "initial content"

:: r2: create branch

svn cp %WC%\trunk %WC%\branches\b
svn ci %WC%\branches -m "open branch"

:: r3: modify all files on trunk

echo "new foo" > %WC%\trunk\foo.txt
echo "new bar" > %WC%\trunk\bar.txt
echo "new baz" > %WC%\trunk\baz.txt

svn ci %WC% -m "modify all files on trunk"

:: r6: tagging

svn cp %WC%\branches\b %WC%\tags\t
svn ci %WC%\tags -m "tagging current b content"

echo Starting svnserve...
start "svnserve" svnserve.exe --daemon --foreground --root %REPOPATH% --listen-host localhost

echo Opening IE in svn://localhost:3690/...
start "iexplore" iexplore.exe svn://localhost:3690/

endlocal
pause
