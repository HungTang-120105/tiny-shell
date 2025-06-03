REM Testcase for addpath functionality

REM 1. Hiển thị PATH hiện tại
echo ==== 1. Current PATH ====
path

REM 2. Tạo một thư mục mới để chứa file thực thi
echo
echo ==== 2. Create a new directory "TestBin" ====
mkdir TestBin

REM 3. Tạo một file batch trong TestBin để kiểm tra
echo
echo ==== 3. Create a test batch file in TestBin ====
echo  "@echo TestBin command executed!" > TestBin\test_command.bat

REM 4. Thử thực thi lệnh từ TestBin trước khi thêm vào PATH
echo
echo ==== 4. Attempt to execute test_command.bat before adding TestBin to PATH ====
test_command.bat

REM 5. Thêm TestBin vào PATH
echo
echo ==== 5. Add "TestBin" to PATH ====
addpath TestBin

REM 6. Hiển thị PATH sau khi thêm TestBin
echo
echo ==== 6. PATH after adding TestBin ====
path

REM 7. Thực thi lệnh từ TestBin sau khi thêm vào PATH
echo
echo ==== 7. Execute test_command.bat from TestBin ====
test_command.bat

REM 8. Xóa TestBin khỏi PATH (nếu cần)
REM echo ==== 8. Remove TestBin from PATH ====
REM set PATH=%PATH:TestBin;=%

echo ==== END ====