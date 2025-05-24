REM Test file and directory operations

REM Tạo thư mục
mkdir testdir
dir

REM Tạo file
touch testfile.txt
dir

REM Ghi nội dung vào file
echo Hello, World! > testfile.txt
cat testfile.txt

REM Xóa file
rm testfile.txt
dir

REM Xóa thư mục
rmdir testdir
dir