REM ==============================================
REM test_commands.bat
REM Purpose: Test the following commands in Windows batch:
REM    cd, pwd (cd), dir, mkdir, rmdir, touch (type nul), rm (del), cat (type)
REM Also echo a large, symmetrical heart shape into a file and display (cat) it.
REM ==============================================

REM 1. Create a new directory named "MyTestDir"
echo ==== 1. Create directory "MyTestDir" ====
mkdir MyTestDir

REM 2. Navigate into the newly created directory
echo
echo ==== 2. Navigate into MyTestDir ====
cd MyTestDir
echo Switched to directory:
REM In Windows, the "cd" command displays the current path (equivalent to pwd)
cd

REM 3. List the contents of the directory (currently empty)
echo
echo ==== 3. List contents (dir) in MyTestDir ====
dir

REM 4. Create an empty file (equivalent to "touch newfile.txt")
echo
echo ==== 4. Create an empty file "sample.txt" (equivalent to touch) ====
touch sample.txt
echo Created empty file sample.txt.

REM 5. Create a file "heart.txt" and echo a large symmetrical heart shape into it
echo
echo ==== 5. Echo a large heart shape into heart.txt ====

echo "           *****           *****               " > heart.txt
echo "        ***********     ***********            " >> heart.txt
echo "      *************** ***************          " >> heart.txt
echo "     *********************************         " >> heart.txt
echo "    ***********************************        " >> heart.txt
echo "    ***********************************        " >> heart.txt
echo "     *********************************         " >> heart.txt
echo "      *******************************          " >> heart.txt
echo "        ***************************            " >> heart.txt
echo "          ***********************              " >> heart.txt
echo "            *******************                " >> heart.txt
echo "              ***************                  " >> heart.txt
echo "                ***********                    " >> heart.txt
echo "                  *******                      " >> heart.txt
echo "                    ***                        " >> heart.txt
echo "                     *                         " >> heart.txt
 
echo Wrote the large heart shape into heart.txt.

REM 6. Display (cat) the contents of heart.txt
echo
echo ==== 6. Display contents of heart.txt ====
cat heart.txt

REM 7. List the directory again to see both sample.txt and heart.txt
echo
echo ==== 7. List MyTestDir again ====
dir

REM 8. Delete the file sample.txt (equivalent to rm sample.txt)
echo
echo ==== 8. Delete the file sample.txt ====
rm sample.txt

REM 9. Delete the file heart.txt
echo
echo ==== 9. Delete the file heart.txt ====
rm heart.txt

REM 10. Navigate to the parent directory and delete MyTestDir
echo
echo ==== 10. Navigate to the parent directory and delete MyTestDir ====
cd ..
rmdir MyTestDir

echo
echo ==== END ====