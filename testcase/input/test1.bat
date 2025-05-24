REM Test script for MyShell

REM Hiển thị thư mục hiện tại
pwd

REM Tạo thư mục mới và chuyển vào đó
mkdir TestFolder
cd TestFolder
pwd

REM Tạo một file văn bản
echo This is a test file created by MyShell. > example.txt

REM Hiển thị nội dung file
type example.txt

REM Hiển thị danh sách file trong thư mục
dir

REM Thêm thư mục hiện tại vào PATH
path

REM Quay lại thư mục gốc và xóa thư mục
cd ..
rmdir /s /q TestFolder

REM Kết thúc
echo Test script completed successfully.